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
#pragma execution_character_set("utf-8")

HqInfoNews::HqInfoNews(QWidget *parent)
    : QTextBrowser(parent)
{
    mRtThread = 0;
    mIndexThread = 0;
    mCfg = 0;

    mDisplaytimer = new QTimer(this);
    connect(mDisplaytimer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    mDisplaytimer->setInterval(10 * 1000);

    this->setWindowFlags(Qt::Tool| Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setWordWrapMode(QTextOption::WordWrap);
    this->verticalScrollBar()->setVisible(false);
    this->horizontalScrollBar()->setVisible(false);
    this->setStyleSheet("background:black;color:white;font-family:Microsoft Yahei;font-size: 12pt; font-weight:bold;");

    HqKuaixun *info_thread = new HqKuaixun(this);
    connect(info_thread, SIGNAL(signalSendKuaiXun(KuaiXunList)), this, SLOT(slotRecvKuaiXunList(KuaiXunList)));
    info_thread->start();

    KZZInfoThread *kzz = new KZZInfoThread(this);
    connect(kzz, SIGNAL(signalSendKZZDataList(QList<KZZ>)), this, SLOT(slotRecvKZZDataList(QList<KZZ>)));
    kzz->start();

    HqMutualTop10Thread *top10 = new HqMutualTop10Thread(this);
    connect(top10, SIGNAL(signalSendTop10DataList(QDate,QList<ExchangeData>,QList<ExchangeData>)),
            this, SLOT(slotRecvMutualTop10DataList(QDate,QList<ExchangeData>,QList<ExchangeData>)));
    connect(top10, SIGNAL(signalSendNorthMoney(double,double,double)),
            this, SLOT(slotRecvNorthMoney(double,double,double)));
    top10->start();

    mRtThread = new HqRealtimeThread(this);
//    mRtThread->appendCodes(QStringList()<<"300059"<<"159949"<<"002475"<<"159995"<<"002351");
    connect(mRtThread, SIGNAL(signalSendHqRtDataList(HqRtDataList)), this, SLOT(slotRecvHqRtDataList(HqRtDataList)));
    mRtThread->start();

    mIndexThread = new HqRealtimeThread(this);
//    mIndexThread->appendCodes(QStringList()<<"s_sh000001"<<"rt_hkHSI"<<"sz399006"<<"hf_GC"<<"hf_SI"<<"hf_CL"<<"hf_OIL"<<"hf_CHA50CFD");
    connect(mIndexThread, SIGNAL(signalSendHqRtDataList(HqRtDataList)), this, SLOT(slotRecvHqRtDataList(HqRtDataList)));
    mIndexThread->start();

    QIcon appIcon = QIcon("://img/12.jpg");
    this->setWindowIcon(appIcon);
    QSystemTrayIcon *mSysTrayIcon = new QSystemTrayIcon(this);
    mSysTrayIcon->setIcon(appIcon);
    mSysTrayIcon->setVisible(true);
    connect(mSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slotSystemTrayOperation(QSystemTrayIcon::ActivationReason)));

    initCfg();
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

void HqInfoNews::slotRecvMutualTop10DataList(const QDate& date, const QList<ExchangeData>& north, const QList<ExchangeData>& south)
{
    clear();
    QStringList totalContent;
    totalContent.append(date.toString("yyyy-MM-dd") + " " + QString::fromUtf8("沪深港通交易数据更新"));
    totalContent.append(QString::fromUtf8("北向方面："));

    QStringList lines;
    foreach (ExchangeData data, north) {
        double net = data.mNet/100000000.0;
        lines.append(QString("%1(%2,%3,%4)").arg(data.mName).arg(data.mCur, 0, 'f', 2).arg(getRichTextString(data.mChgPercnt)).arg(getRichTextString(net)));
//        if(mRtThread) mRtThread->appendCodes(QStringList()<<data.mCode);
    }
    totalContent.append(lines.join(" ,"));
#if 1
    lines.clear();
    totalContent.append(QString::fromUtf8("南向方面："));
    foreach (ExchangeData data, south) {
        double net = data.mNet/100000000.0;
        lines.append(QString("%1(%2,%3,%4)").arg(data.mName).arg(data.mCur, 0, 'f', 2).arg(getRichTextString(data.mChgPercnt)).arg(getRichTextString(net)));
    }
    totalContent.append(lines.join(" ,"));
#endif
    appendText(totalContent, 20);
}


void HqInfoNews::slotRecvNorthMoney(double total, double sh, double sz)
{
    QStringList totalContent;
    totalContent.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString::fromUtf8("沪深港通实时资金流更新"));
    totalContent.append(QString::fromUtf8("北向方面：%1亿， 其中上海%2亿，深圳%3亿").arg(getRichTextString(total / 10000.0)).arg(getRichTextString(sh/10000.0)).arg(getRichTextString(sz/10000.0)));
    appendText(totalContent);
}


void HqInfoNews::slotRecvKuaiXunList(const KuaiXunList& list)
{
    if(list.size() == 0) return;
    QStringList totalContent;
    KuaixunData data = list.first();
//    this->clear();

    totalContent.append(data.src_time + "  " + data.sourceString());
    totalContent.append(data.digest);
    appendText(totalContent);
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

void HqInfoNews::slotRecvHqRtDataList(const HqRtDataList &list)
{
    if(list.size() == 0) return;
    QStringList totalContent;
    QStringList titles;
    titles.append(QString::fromUtf8("名称"));
    titles.append(QString::fromUtf8("涨跌幅(%)"));
    titles.append(QString::fromUtf8("成交金额(亿)"));
    totalContent.append(getFormatString(titles));
    for(int i=0; i<list.size(); i++)
    {
        HqRtData data = list[i];
        titles.clear();
        titles.append(data.mName);
        titles.append(getRichTextString(data.mChgPercnt));
        titles.append(QString::number(data.mTotal, 'f', 2));
        totalContent.append(titles.join("               "));
    }
    appendText(totalContent);
}

void HqInfoNews::initCfg()
{
    mCfg = new SettingsCfg;
    connect(mCfg, SIGNAL(signalAddIndex(QStringList)), mIndexThread, SLOT(appendCodes(QStringList)));
    connect(mCfg, SIGNAL(signalRemoveIndex(QStringList)), mIndexThread, SLOT(removeCodes(QStringList)));

    connect(mCfg, SIGNAL(signalAddShareCode(QStringList)), mRtThread, SLOT(appendCodes(QStringList)));
    connect(mCfg, SIGNAL(signalRemoveShareCode(QStringList)), mRtThread, SLOT(removeCodes(QStringList)));
}

void HqInfoNews::slotSystemTrayOperation(QSystemTrayIcon::ActivationReason val)
{
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
    case QSystemTrayIcon::Context:
    {
//        QMenu *popMenu = new QMenu(this);
//        QList<QAction*> actlist;
//        QStringList poplist;
//        poplist<<QStringLiteral("显示")<<QStringLiteral("退出");
//        int index = -1;
//        foreach (QString name, poplist) {
//            index++;
//            QAction *act = new QAction(this);
//            act->setText(name);
//            act->setData(index);
////            connect(act, &QAction::triggered, this, &zchxMainWindow::slotSystemTrayMenuClicked);
//            actlist.append(act);
//        }

//        popMenu->addActions(actlist);
//        popMenu->popup(QCursor::pos());
    }
        break;
    default:
        break;
    }

}
