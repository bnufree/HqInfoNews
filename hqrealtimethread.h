#ifndef HQREALTIMETHREAD_H
#define HQREALTIMETHREAD_H

#include <QThread>
#include <QList>

struct HqRtData{
    QString     mCode;
    QString     mName;
    double      mCur;
    double      mChgPercnt;
    double      mTotal;
};

typedef QList<HqRtData>     HqRtDataList;

class HqRealtimeThread : public QThread
{
    Q_OBJECT
public:
    explicit HqRealtimeThread(QObject *parent = nullptr);
    void run();
    void    appendCodes(const QStringList& list);
    void    removeCodes(const QStringList& list);
    static HqRtDataList getHqRtDataList(const QStringList& codelist);

signals:
    void    signalSendHqRtDataList(const HqRtDataList& list);
public slots:
private:
    QStringList     mCodesList;
};

#endif // HQREALTIMETHREAD_H
