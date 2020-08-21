#include "datathread.h"

Datathread::Datathread(QObject *parent) : QThread(parent)
{
    mSendMsg = true;
}
