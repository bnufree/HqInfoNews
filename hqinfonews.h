#ifndef HQINFONEWS_H
#define HQINFONEWS_H

#include <QTextBrowser>
#include "hqkuaixun.h"
#include "kzzinfothread.h"
#include "hqmutualtop10thread.h"
#include "hqrealtimethread.h"
#include <QSystemTrayIcon>

class SettingsCfg;
class HqInfoNews : public QTextBrowser
{
    Q_OBJECT

public:
    HqInfoNews(QWidget *parent = 0);
    ~HqInfoNews();

private:
    QString getRichTextString(double val);
    void    appendText(const QStringList& list, int time_out = 10);
    void    adjustPostion();
    QString getFormatString(const QStringList& list);
    void    initCfg();

private slots:
    void slotRecvKuaiXunList(const KuaiXunList& list);
    void slotRecvKZZDataList(const QList<KZZ>& list);
    void slotRecvMutualTop10DataList(const QDate& date, const QList<ExchangeData>& north, const QList<ExchangeData>& south);
    void slotTimeOut();
    void slotRecvNorthMoney(double total, double sh, double sz);
    void slotRecvHqRtDataList(const HqRtDataList& list);
    void slotSystemTrayOperation(QSystemTrayIcon::ActivationReason);

private:
    QTimer*         mDisplaytimer;
    HqRealtimeThread    *mRtThread;
    HqRealtimeThread    *mIndexThread;
    SettingsCfg*        mCfg;
};

#endif // HQINFONEWS_H
