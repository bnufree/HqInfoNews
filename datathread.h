#ifndef DATATHREAD_H
#define DATATHREAD_H

#include <QThread>

class Datathread : public QThread
{
    Q_OBJECT
public:
    explicit Datathread(QObject *parent = nullptr);
public slots:
    void    slotSendMsg(bool sts) { mSendMsg = sts;}

signals:

public slots:

protected:
    bool    mSendMsg;
};

#endif // DATATHREAD_H
