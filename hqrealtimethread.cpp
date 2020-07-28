﻿#include "hqrealtimethread.h"
#include "qhttpget.h"

HqRealtimeThread::HqRealtimeThread(QObject *parent) : QThread(parent)
{
    qRegisterMetaType<HqRtDataList>("const HqRtDataList&");
}

void HqRealtimeThread::run()
{
    while (1) {
        if(mCodesList.size() == 0) {sleep(1); continue;}
        QString url = "http://hq.sinajs.cn/list=";
        foreach (QString code, mCodesList) {
            if(code.left(1) == "s" || code.left(1) == "r")
            {
                url.append(QString("%1,").arg(code));
                continue;
            }
            if(code.length() == 6)
            {
                if(code.toInt() < 500000){
                    url.append(QString("s_sz%1,").arg(code));
                } else
                {
                    url.append(QString("s_sh%1,").arg(code));
                }
            } else
            {
                url.append(QString("rt_hk%1,").arg(code));
            }
        }
        QString result = QString::fromLocal8Bit(QHttpGet::getContentOfURL(url));
        QStringList resultList = result.split("\n", QString::SkipEmptyParts);
        HqRtDataList hq_list;
        foreach (QString line, resultList) {
            line.remove("var hq_str_");
            QStringList line_list = line.split(QRegExp("[=\",;]"), QString::SkipEmptyParts);
            if(line_list.size() == 0) continue;
            QString code = line_list[0];
            if(code.left(2) == "s_")
            {
                HqRtData data;
                data.mCode = code.right(6);
                if(line_list.size() > 1) data.mName = line_list[1];
                if(line_list.size() > 2) data.mCur = line_list[2].toDouble();
                if(line_list.size() > 4) data.mChgPercnt = line_list[4].toDouble();
                if(line_list.size() > 6) data.mTotal = line_list[6].toDouble() / 10000.0;
                hq_list.append(data);

            } else if(code.left(2) == "rt")
            {
                HqRtData data;
                data.mCode = code.mid(5);
                if(line_list.size() > 2) data.mName = line_list[2];
                if(line_list.size() > 7) data.mCur = line_list[7].toDouble();
                if(line_list.size() > 9) data.mChgPercnt = line_list[9].toDouble();
                if(line_list.size() > 12)
                {
                    data.mTotal = line_list[12].toDouble() / 100000000.0;
                    if(code == "rt_hkHSI")
                    {
                        data.mTotal *= 1000;
                    }
                }
                hq_list.append(data);

            } else if(code.left(2) == "sh" || code.left(2) == "sz")
            {
                HqRtData data;
                data.mCode = code.right(6);
                if(line_list.size() > 1) data.mName = line_list[1];
                double last_close = 0.0;
                if(line_list.size() > 3) last_close = line_list[3].toDouble();
                if(line_list.size() > 4) data.mCur = line_list[4].toDouble();
                if(last_close > 0.0) data.mChgPercnt = (data.mCur - last_close) / last_close * 100;
                if(line_list.size() > 10) data.mTotal = line_list[10].toDouble() / 100000000.0;
                hq_list.append(data);
            }
        }

        emit signalSendHqRtDataList(hq_list);

        sleep(60);
    }
}

void HqRealtimeThread::appendCodes(const QStringList &list)
{
    foreach (QString code, list) {
        if(!mCodesList.contains(code)) mCodesList.append(code);
    }
}

void HqRealtimeThread::removeCodes(const QStringList &list)
{
    foreach (QString code, list) {
        mCodesList.removeOne(code);
    }
}