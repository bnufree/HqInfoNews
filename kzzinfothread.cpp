#include "kzzinfothread.h"
#include "qhttpget.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

KZZInfoThread::KZZInfoThread(QObject *parent) : QThread(parent)
{
    qRegisterMetaType<QList<KZZ>>("const QList<KZZ>&");
}

void KZZInfoThread::run()
{
    while (1) {
        QThread::sleep(300);
        if(QTime::currentTime().hour() >= 15) continue;
        QString url = "http://dcfm.eastmoney.com/em_mutisvcexpandinterface/api/js/get?type=KZZ_LB&token=70f12f2f4f091e459a279469fe49eca5&st=STARTDATE&sr=-1&p=1&ps=10&js=";
        QString result = QString::fromUtf8(QHttpGet::getContentOfURL(url));

        int start_index = result.indexOf("[", 0);
        if(start_index < 0) return;
        result = result.mid(start_index);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8(), &error);
        if(error.error != QJsonParseError::NoError) return;
        if(!doc.isArray()) return;
        QJsonArray array = doc.array();
        QStringList HqCodeList;
        QMap<QString, KZZ>  datalist;
        for(int i=0; i<array.size(); i++)
        {
            QJsonObject obj = array.at(i).toObject();
            KZZ data;
            data.mName = obj.value("SNAME").toString();
            data.mZGMC = obj.value("SECURITYSHORTNAME").toString();
            data.mSGRI = obj.value("STARTDATE").toString().mid(0, 10);
            data.mGQDJ = obj.value("GDYX_STARTDATE").toString().mid(0, 10);
            data.mZGDM = obj.value("SWAPSCODE").toString();
            data.mPSBL = obj.value("FSTPLACVALPERSTK").toDouble();
            data.mZGJ  = obj.value("ZGJZGJ").toDouble();
            if(QDate::fromString(data.mSGRI, "yyyy-MM-dd") >= QDate::currentDate())
            {
                datalist[data.mZGDM] = data;                

                HqCodeList.append(data.mZGDM);
            } else
            {
                break;
            }

        }
        //获取现价
        if(HqCodeList.size() > 0)
        {
            url = "http://hq.sinajs.cn/list=";
            for (int i=0; i<HqCodeList.size(); i++) {
                QString &wkcode = HqCodeList[i];
                if(wkcode.left(1).toInt() < 5)
                {
                    wkcode.prepend("s_sz");
                } else
                {
                    wkcode.prepend("s_sh");
                }
            }
            url.append(HqCodeList.join(","));

            QString result = QString::fromUtf8(QHttpGet::getContentOfURL(url));
            QStringList line_list = result.split("\n", QString::SkipEmptyParts);
            foreach (QString line, line_list) {
                QStringList hqlist = line.split(QRegExp("[=\"\\,;]"), QString::SkipEmptyParts);
                if(hqlist.size() >= 7)
                {
                    QString code = hqlist[0].right(6);
                    double  cur =  hqlist[2].toDouble();
                    KZZ &data = datalist[code];
                    data.mMoney = 1000 / data.mPSBL * cur;
                    data.mZGJZ = 100 *(cur/data.mZGJ);
//                    qDebug()<<data.mZGMC<<data.mPSBL<<data.mZGJZ<<cur;
                }
            }
        }

        if(datalist.size() > 0)
        {
            emit signalSendKZZDataList(datalist.values());
        }

    }

}
