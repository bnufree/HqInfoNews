#ifndef KZZINFOTHREAD_H
#define KZZINFOTHREAD_H

#include <QThread>
struct KZZ{
    QString   mName;
    QString     mSGRI;        //申购日期
    QString     mGQDJ;        //股东配售登记日
    double    mPSBL;        //配售比例
    double    mMoney;       //稳获10股需要的钱数
    double    mZGJ;
    double    mZGJZ;        //转股价值  小于100就有可能破发
    QString   mZGMC;        //正股名称
    QString   mZGDM;       //正股代码
};

class KZZInfoThread : public QThread
{
    Q_OBJECT
public:
    explicit KZZInfoThread(QObject *parent = NULL);

    void run();

signals:
    void  signalSendKZZDataList(const QList<KZZ>& list);
public slots:
};

#endif // KZZINFOTHREAD_H
