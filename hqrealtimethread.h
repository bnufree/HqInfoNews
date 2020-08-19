#ifndef HQREALTIMETHREAD_H
#define HQREALTIMETHREAD_H

#include "datathread.h"
#include <QList>

struct HqRtData{
    QString     mCode;
    QString     mName;
    double      mCur;
    double      mChgPercnt;
    double      mTotal;
};

typedef QList<HqRtData>     HqRtDataList;

class HqRealtimeThread : public Datathread
{
    Q_OBJECT
public:
    explicit HqRealtimeThread(QObject *parent = NULL);
    void run();
    QStringList getCodes() const {return mCodesList;}
    static HqRtDataList getHqRtDataList(const QStringList& codelist);

signals:
    void    signalSendHqRtDataList(const HqRtDataList& list);
public slots:    
    void    appendCodes(const QStringList& list);
    void    removeCodes(const QStringList& list);
private:
    QStringList     mCodesList;
};

#endif // HQREALTIMETHREAD_H
