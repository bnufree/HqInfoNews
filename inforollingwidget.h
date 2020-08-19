#ifndef INFOROLLINGWIDGET_H
#define INFOROLLINGWIDGET_H

#include <QLabel>
#include "hqrealtimethread.h"

class InfoRollingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InfoRollingWidget(QWidget *parent = nullptr);
    void  moveToBottom();
    QString getHtmlRichText(const HqRtDataList& list);
private:


protected:
    void  resizeEvent(QResizeEvent* e);

signals:

public slots:
    void        slotTimeOut();
    void        slotAppendText(const QString& text);
    void        slotRecvHqRtDataList(const HqRtDataList& list);
    void        slotAppendIndex(const QStringList& codes);
    void        slotAppendShare(const QStringList &codes);
    void        slotRemoveIndex(const QStringList& codes);
    void        slotRemoveShare(const QStringList &codes);
    void        slotEnableIndex(bool sts) {if(mIndexThread) mIndexThread->slotSendMsg(sts);}
    void        slotEnableShare(bool sts) {if(mShareThread) mShareThread->slotSendMsg(sts);}

private:
    HqRealtimeThread    *mShareThread;
    HqRealtimeThread    *mIndexThread;
    QMap<QString, HqRtData>  mRollDataMap;
    QList<QLabel*>          mDisplayLabelList;
    int                     mLastIndex;
};

#endif // INFOROLLINGWIDGET_H
