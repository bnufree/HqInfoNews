#include "hqinfonews.h"
#include <QScrollBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QDebug>
#include <QDate>
#include <QMenu>
#include "hqrealtimethread.h"
#include "settingscfg.h"
#include "inforollingwidget.h"
#include <QThreadPool>

#pragma execution_character_set("utf-8")

HqInfoNews::HqInfoNews(InfoRollingWidget* roll, QWidget *parent)
    : QTextBrowser(parent)
{
//    this->setAttribute(Qt::WA_TranslucentBackground, true);
    mForceDisplay = true;
    mRollWidget = roll;
    if(mRollWidget == 0)
    {
        mRollWidget = new InfoRollingWidget;
        mRollWidget->moveToBottom();
        mRollWidget->show();
    }
    initCfg();
    connect(mCfg, SIGNAL(signalEnableRtZxg(bool)), mRollWidget, SLOT(slotEnableShare(bool)));
    connect(mCfg, SIGNAL(signalEnableRtIndex(bool)), mRollWidget, SLOT(slotEnableIndex(bool)));

    mDisplaytimer = new QTimer(this);
    connect(mDisplaytimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    mDisplaytimer->setInterval(10 * 1000);

    this->setWindowFlags(Qt::Tool| Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setWordWrapMode(QTextOption::WordWrap);
    this->verticalScrollBar()->setVisible(false);
    this->horizontalScrollBar()->setVisible(false);
    this->setStyleSheet("background:black;color:white;font-family:Microsoft Yahei;font-size: 12pt; font-weight:bold;");

    HqKuaixun *info_thread = new HqKuaixun(this);
    connect(mCfg, SIGNAL(signalEnableRtInfo(bool)), info_thread, SLOT(slotSendMsg(bool)));
    connect(info_thread, SIGNAL(signalSendKuaiXun(KuaiXunList)), this, SLOT(slotRecvKuaiXunList(KuaiXunList)));
    info_thread->start();

    KZZInfoThread *kzz = new KZZInfoThread(this);
    connect(kzz, SIGNAL(signalSendKZZDataList(QList<KZZ>)), this, SLOT(slotRecvKZZDataList(QList<KZZ>)));
    kzz->start();

    HqMutualTop10Thread *top10 = new HqMutualTop10Thread(this);
    connect(mCfg, SIGNAL(signalEnableRtnorthTop10(bool)), top10, SLOT(slotSendMsg(bool)));
    connect(top10, SIGNAL(signalSendTop10DataList(QDate,QList<ExchangeData>,QList<ExchangeData>)),
            this, SLOT(slotRecvMutualTop10DataList(QDate,QList<ExchangeData>,QList<ExchangeData>)));
    connect(top10, SIGNAL(signalSendNorthMoney(double,double,double)),
            this, SLOT(slotRecvNorthMoney(double,double,double)));
    top10->start();    

    QIcon appIcon = QIcon("://img/12.jpg");
    this->setWindowIcon(appIcon);
    QSystemTrayIcon *mSysTrayIcon = new QSystemTrayIcon(this);
    mSysTrayIcon->setIcon(appIcon);
    mSysTrayIcon->setVisible(true);
    connect(mSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slotSystemTrayOperation(QSystemTrayIcon::ActivationReason)));
    QMenu *popMenu = new QMenu(this);
    if(mPopMenuActionList.size() == 0)
    {
        mPopMenuActionList.append(createAction(getDisplayText(), this, SLOT(slotSetDisplayStatus())));
        mPopMenuActionList.append(createAction(QString::fromUtf8("退出"), QApplication::instance(), SLOT(quit())));
    }

    popMenu->addActions(mPopMenuActionList);
    mSysTrayIcon->setContextMenu(popMenu);
}

HqInfoNews::~HqInfoNews()
{

}

QString HqInfoNews::getRichTextString(double val)
{
    QString res = "";
    if(val < 0)
    {
        res = QString("<font style='font-weight:bold;color:green;font-style:oblique;'>  %1  </font>").arg(-val, 0, 'f', 2);
    } else
    {
        res = QString("<font style='font-weight:bold;color:red;font-style:oblique;'>  %1  </font>").arg(val, 0, 'f', 2);
    }

    return res;
}

QString HqInfoNews::getHtmlTableTextString(const QDate &date, const QList<ExchangeData> &north, const QList<ExchangeData> &south)
{
    int total_width = width();
    QString result;
    result.append("<html></head><body>");
    result.append(QString("<p>%1</p>").arg(date.toString("yyyy-MM-dd") + " " + QString::fromUtf8("沪深港通交易数据更新")));
    result.append(QString("<style>.td{width:%1;}</style>").arg(total_width/4));
    result.append(QString("<table border=\"0\", width = %1>").arg(total_width));
    QList<ExchangeData> total = north;
    total.append(south);
    result.append(QString("<tr><td>     </td><td >%1</td><td >%2</td><td >%3</td><td >%4</td></tr>")
                  .arg(QString::fromUtf8("名称"))
                  .arg(QString::fromUtf8("现价"))
                  .arg(QString::fromUtf8("涨跌"))
                  .arg(QString::fromUtf8("净买")));
    int i = 0;
    foreach (ExchangeData data, total) {
        double net = data.mNet/100000000.0;
        i++;
        result.append("<tr>");
        result.append(QString("<td  style=\"color:#ffffff\">%1</td>").arg(i));
        result.append(QString("<td  style=\"color:#ffffff\">%1</td>").arg(data.mName));
        QString colrStr = data.mChgPercnt > 0.001 ? "#ff0000" : data.mChgPercnt < -0.001 ? "#00ff00" : "#ffffff";
        result.append(QString("<td  style=\"color:%1\">%2</td>").arg(colrStr).arg(data.mCur, 0, 'f', 2));
        result.append(QString("<td  style=\"color:%1\">%2</td>").arg(colrStr).arg(data.mChgPercnt, 0, 'f', 2));
        colrStr = data.mNet > 0.001 ? "#ff0000" : data.mNet < -0.001 ? "#00ff00" : "#ffffff";
        result.append(QString("<td  style=\"color:%1\">%2</td>").arg(colrStr).arg(net, 0, 'f', 2));
        result.append("</tr>");
    }

    result.append("</table>");
    result.append("</body></html>");
    return result;
}

void HqInfoNews::slotRecvMutualTop10DataList(const QDate& date, const QList<ExchangeData>& north, const QList<ExchangeData>& south)
{
    clear();
    QStringList totalContent;
#if 0
    totalContent.append(date.toString("yyyy-MM-dd") + " " + QString::fromUtf8("沪深港通交易数据更新"));
    totalContent.append(QString::fromUtf8("北向方面："));

    QStringList lines;
    foreach (ExchangeData data, north) {
        double net = data.mNet/100000000.0;
        lines.append(QString("%1(%2,%3,%4)").arg(data.mName).arg(data.mCur, 0, 'f', 2).arg(getRichTextString(data.mChgPercnt)).arg(getRichTextString(net)));
//        if(mRtThread) mRtThread->appendCodes(QStringList()<<data.mCode);
    }
    totalContent.append(lines.join(" ,"));
    lines.clear();
    totalContent.append(QString::fromUtf8("南向方面："));
    foreach (ExchangeData data, south) {
        double net = data.mNet/100000000.0;
        lines.append(QString("%1(%2,%3,%4)").arg(data.mName).arg(data.mCur, 0, 'f', 2).arg(getRichTextString(data.mChgPercnt)).arg(getRichTextString(net)));
    }
    totalContent.append(lines.join(" ,"));
#else
    totalContent.append(getHtmlTableTextString(date, north, south));
#endif
    appendText(totalContent, 20);
}


void HqInfoNews::slotRecvNorthMoney(double total, double sh, double sz)
{
    QStringList totalContent;
    totalContent.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString::fromUtf8("沪深港通实时资金流更新"));
    totalContent.append(QString::fromUtf8("北向方面：%1亿， 其中上海%2亿，深圳%3亿").arg(getRichTextString(total / 10000.0)).arg(getRichTextString(sh/10000.0)).arg(getRichTextString(sz/10000.0)));
#if 1
    qDebug()<<totalContent;
    appendText(totalContent);
#else
    if(mRollWidget) mRollWidget->slotAppendText(totalContent.last());
#endif
}


void HqInfoNews::slotRecvKuaiXunList(const KuaiXunList& list)
{
    if(list.size() == 0) return;
    QStringList totalContent;
    KuaixunData data = list.first();

    totalContent.append(data.src_time + "  " + data.sourceString());
    totalContent.append(data.digest);
#if 1
    appendText(totalContent);
#else
    if(mRollWidget) mRollWidget->slotAppendText(data.title);
#endif
}


void HqInfoNews::slotRecvKZZDataList(const QList<KZZ>& list)
{
    if(list.size() == 0) return;
    QStringList totalContent;
    QStringList titles;
    titles.append(QString::fromUtf8("名称"));
    titles.append(QString::fromUtf8("申购日期"));
    titles.append(QString::fromUtf8("股权登记"));
    titles.append(QString::fromUtf8("确保配售"));
    titles.append(QString::fromUtf8("转股价值"));
    totalContent.append(titles.join("  "));
    for(int i=0; i<list.size(); i++)
    {
        KZZ data = list[i];
        titles.clear();
        titles.append(data.mZGMC);
        titles.append(data.mSGRI);
        titles.append(data.mGQDJ);
        titles.append(QString::number(data.mMoney, 'f', 0));
        titles.append(QString::number(data.mZGJZ, 'f', 0));
        totalContent.append(titles.join("  "));
    }
    appendText(totalContent);
}

void HqInfoNews::appendText(const QStringList &list, int time_out)
{
    clear();
    foreach (QString text, list) {
        this->append(text);
    }
    adjustPostion();
    this->show();
    if(mDisplaytimer)
    {
        int cur_timeout = time_out * 1000;
        if(mDisplaytimer->isActive())
        {
            if(cur_timeout < mDisplaytimer->interval()) cur_timeout = mDisplaytimer->interval();
        }
        mDisplaytimer->start(cur_timeout);
    }
}

void HqInfoNews::adjustPostion()
{
    QRect rect = QApplication::desktop()->availableGeometry();
    this->setFixedWidth(rect.width() * 0.2);
//    this->document()->setTextWidth(this->width());
#if 0
    int newheight = this->document()->size().rheight() + 20;
    if (newheight != height())
    {
        this->setFixedHeight(newheight);
    }
#else
    int height = this->verticalScrollBar()->maximum() - this->verticalScrollBar()->minimum() + this->verticalScrollBar()->pageStep();
    this->setFixedHeight(height * 1.05);
#endif



    this->move(rect.width() - this->width() - 20, rect.height() - this->height() - 60);
}

void HqInfoNews::slotTimeOut()
{
    this->clear();
    this->setFixedHeight(1.0);
    this->hide();
}

QString HqInfoNews::getFormatString(const QStringList &list)
{
    QString result;
    QRect rect = QApplication::desktop()->availableGeometry();
    int   col_width = rect.width() * 0.15 / list.size();
    foreach (QString src, list) {
        int cal_width = this->fontMetrics().width(src);
        while (cal_width < col_width) {
            src.prepend(" ");
            src.append(" ");
            cal_width = this->fontMetrics().width(src);
        }
        result.append(src);
    }

    return result;

}

void HqInfoNews::initCfg()
{
    mCfg = new SettingsCfg;
    if(mRollWidget)
    {
        connect(mCfg, SIGNAL(signalAddIndex(QStringList)), mRollWidget, SLOT(slotAppendIndex(QStringList)));
        connect(mCfg, SIGNAL(signalRemoveIndex(QStringList)), mRollWidget, SLOT(slotRemoveIndex(QStringList)));
        connect(mCfg, SIGNAL(signalAddShareCode(QStringList)), mRollWidget, SLOT(slotAppendShare(QStringList)));
        connect(mCfg, SIGNAL(signalRemoveShareCode(QStringList)), mRollWidget, SLOT(slotRemoveShare(QStringList)));
    }
}

QAction* HqInfoNews::createAction(const QString &title, const QObject *receiver, const char *member)
{
    QAction* act = new QAction(title, this);
    connect(act, SIGNAL(triggered(bool)), receiver, member);
    return act;
}

void HqInfoNews::slotSetDisplayStatus()
{
    mForceDisplay = !mForceDisplay;
    QAction* act = qobject_cast<QAction*>(sender());
    if(act) act->setText(getDisplayText());
    if(mRollWidget) mRollWidget->setForceDisplay(mForceDisplay);
    if(!mForceDisplay)
    {
//        this->setVisible(false);
    }
}

QString HqInfoNews::getDisplayText() const
{
    if(mForceDisplay)
    {
        return tr("隐藏");
    }

    return tr("显示");
}

void HqInfoNews::slotSystemTrayOperation(QSystemTrayIcon::ActivationReason val)
{
    qDebug()<<"system icon :"<<val;
    switch (val) {
    case QSystemTrayIcon::DoubleClick:
    {
        if(!mCfg)
        {
            initCfg();
        }
        mCfg->show();
        mCfg->raise();
    }
        break;
    case QSystemTrayIcon::Trigger:
    {
        if(mRollWidget && mForceDisplay) mRollWidget->raise();
    }
        break;
    case QSystemTrayIcon::Context:
    {


    }
        break;
    default:
        break;
    }

}
