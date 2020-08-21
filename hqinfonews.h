#ifndef HQINFONEWS_H
#define HQINFONEWS_H

#include <QTextBrowser>
#include "hqkuaixun.h"
#include "kzzinfothread.h"
#include "hqmutualtop10thread.h"
#include <QSystemTrayIcon>

class SettingsCfg;
class InfoRollingWidget;

class HqInfoNews : public QTextBrowser
{
    Q_OBJECT

public:
    HqInfoNews(InfoRollingWidget* roll, QWidget *parent = 0);
    ~HqInfoNews();
    void  setInfoRollingWidget(InfoRollingWidget* w) {mRollWidget = w;}


private:
    QString getRichTextString(double val);
    void    appendText(const QStringList& list, int time_out = 10);
    void    adjustPostion();
    QString getFormatString(const QStringList& list);
    void    initCfg();
    QAction*  createAction(const QString& title, const QObject *receiver, const char *member);
    QString   getDisplayText() const;

private slots:
    void slotRecvKuaiXunList(const KuaiXunList& list);
    void slotRecvKZZDataList(const QList<KZZ>& list);
    void slotRecvMutualTop10DataList(const QDate& date, const QList<ExchangeData>& north, const QList<ExchangeData>& south);
    void slotTimeOut();
    void slotRecvNorthMoney(double total, double sh, double sz);    
    void slotSystemTrayOperation(QSystemTrayIcon::ActivationReason);
    void slotSetDisplayStatus();
private:
    QTimer*         mDisplaytimer;
    SettingsCfg*        mCfg;
    InfoRollingWidget   *mRollWidget;
    QList<QAction*>         mPopMenuActionList;
    bool                    mForceDisplay;
};

#endif // HQINFONEWS_H
