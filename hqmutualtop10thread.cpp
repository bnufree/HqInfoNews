#include "hqmutualtop10thread.h"
#include "qhttpget.h"
#include <QDebug>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include "hqrealtimethread.h"

HqMutualTop10Thread::HqMutualTop10Thread(QObject *parent) : Datathread(parent)
{
    qRegisterMetaType<QList<ExchangeData> >("const QList<ExchangeData>&");
}

void HqMutualTop10Thread::getRtNorthMoneyInfo()
{
    QByteArray recv = QHttpGet::getContentOfURL(QString("http://push2.eastmoney.com/api/qt/kamt/get?fields1=f1,f2,f3,f4&fields2=f51,f52,f53,f54,f63&ut=b2884a393a59ad64002292a3e90d46a5&cb=&_%1")
                                                .arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    QJsonDocument doc = QJsonDocument::fromJson(recv);
    if(!doc.isObject()) return;
    QJsonObject obj = doc.object().value("data").toObject();
    double hk2sh = obj.value("hk2sh").toObject().value("netBuyAmt").toDouble();
    double hk2sz = obj.value("hk2sz").toObject().value("netBuyAmt").toDouble();
    double total = hk2sh + hk2sz;

    qDebug()<<"now north money:"<<total<<hk2sh<<hk2sz;
    emit signalSendNorthMoney(total, hk2sh, hk2sz);
}

void HqMutualTop10Thread::run()
{
    QMap<QString, ExchangeData> dataMap;
    QDate workDate;
    int top10_update = 0;
    QStringList codelist;
    while (1) {
        QDateTime now = QDateTime::currentDateTime();
        if(now.date().dayOfWeek() == 7 || now.date().dayOfWeek() == 6) continue;
        int hour =  now.time().hour();
        if(hour >= 15 && hour < 17) continue;

        getRtNorthMoneyInfo();

        if(hour < 17)
        {
            if(now.date().dayOfWeek() == 1)
            {
                now.setDate(now.date().addDays(-3));
            } else
            {
                now.setDate(now.date().addDays(-1));
            }
        }
        top10_update++;

        if(top10_update % 3 == 0)
        {
            top10_update = 0;
            dataMap.clear();
            codelist.clear();
            while(1)
            {
                QByteArray recv = QHttpGet::getContentOfURL(QString("http://sc.hkex.com.hk/TuniS/www.hkex.com.hk/chi/csm/DailyStat/data_tab_daily_%1c.js?_=%2").arg(now.toString("yyyyMMdd"))
                                                            .arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
                int index = recv.indexOf("[");
                if(index >= 0) recv = recv.mid(index);
                QString result = QString::fromUtf8(recv).remove(QRegExp("[\\r\\n\\t]"));
                QRegularExpression start_reg("\\[\\[\"[0-9]{1,2}\"");
                int start_index = 0;
                while ((start_index = result.indexOf(start_reg, start_index)) >= 0) {
                    int end_index = result.indexOf("]]", start_index);
                    if(end_index == -1) break;
                    QString line = result.mid(start_index, end_index - start_index+2);
                    QStringList lineList = line.split("\", \"", QString::SkipEmptyParts);
                    start_index = end_index;
                    if(lineList.size() == 6)
                    {
                        ExchangeData data;
                        data.mCode = lineList[1];
                        if(data.mCode.size() != 5)
                        {
                            int code = data.mCode.toInt();
                            data.mCode = QString("").sprintf("%06d", code);
                        }
                        data.mName = lineList[2].trimmed();
                        data.mBuy = lineList[3].remove(",").toDouble();
                        data.mSell = lineList[4].remove(",").toDouble();
                        data.mTotal = data.mBuy + data.mSell;
                        data.mNet = data.mBuy - data.mSell;
                        if(dataMap.contains(data.mCode))
                        {
                            ExchangeData& exist = dataMap[data.mCode];
                            exist.mBuy += data.mBuy;
                            exist.mSell += data.mSell;
                            exist.mTotal += data.mTotal;
                            exist.mNet += data.mNet;

                        } else
                        {
                            dataMap.insert(data.mCode, data);
                        }
                        codelist.append(data.mCode);
                    }
                }
                if(dataMap.size()  > 0) break;
                now = now.addDays(-1);
            }

//            qDebug()<<workDate<<now.date()<<dataMap.size()<<codelist.size();

            if(dataMap.size() > 0 && mSendMsg)
            {
                HqRtDataList hqlist = HqRealtimeThread::getHqRtDataList(codelist);
                foreach (HqRtData data, hqlist) {
                    ExchangeData &chg = dataMap[data.mCode.mid(data.mCode.indexOf(QRegExp("\\d")))];
                    chg.mCur = data.mCur;
                    chg.mChgPercnt = data.mChgPercnt;
                }
                QList<ExchangeData> north, south;
                foreach (ExchangeData data, dataMap) {
                    if(data.mCode.size() == 6)
                    {
                        north.append(data);
                    } else
                    {
                        south.append(data);
                    }
                }
                std::sort(north.begin(), north.end(), std::greater<ExchangeData>());
                std::sort(south.begin(), south.end());
                emit signalSendTop10DataList(now.date(), north, south);
            }
        }

        sleep(60);

    }
}
