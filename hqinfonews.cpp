#include "hqinfonews.h"
#include <QScrollBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QDebug>
#include <QDate>
#include "hqrealtimethread.h"


HqInfoNews::HqInfoNews(QWidget *parent)
    : QTextBrowser(parent)
{
    mRtThread = 0;
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
    mRtThread->appendCodes(QStringList()<<"300059"<<"00700"<<"002475"<<"s_sh000001"<<"rt_hkHSI"<<"sz399006");
    connect(mRtThread, SIGNAL(signalSendHqRtDataList(HqRtDataList)), this, SLOT(slotRecvHqRtDataList(HqRtDataList)));
    mRtThread->start();
}

HqInfoNews::~HqInfoNews()
{

}

QString HqInfoNews::getRichTextString(double val)
{
    QString res = "";
    if(val < 0)
    {
        res = QString("<font style='font-size:20px; font-weight:bold; color:green;'>%1</font>").arg(-val, 0, 'f', 2);
    } else
    {
        res = QString("<font style='font-size:20px; font-weight:bold; color:red;'>%1</font>").arg(val, 0, 'f', 2);
    }

    return res;
}

void HqInfoNews::slotRecvMutualTop10DataList(const QDate& date, const QList<ExchangeData>& north, const QList<ExchangeData>& south)
{
    QStringList totalContent;
    totalContent.append(date.toString("yyyy-MM-dd") + " " + QString::fromLocal8Bit("沪深港通交易数据更新"));
    totalContent.append(QString::fromLocal8Bit("北向方面："));

    QStringList lines;
    foreach (ExchangeData data, north) {
        double net = data.mNet/100000000.0;
        lines.append(QString("%1(%2)").arg(data.mName).arg(getRichTextString(net)));
        if(mRtThread) mRtThread->appendCodes(QStringList()<<data.mCode);
    }
    totalContent.append(lines.join(" ,"));
#if 0
    lines.clear();
    totalContent.append(QString::fromLocal8Bit("南向方面："));
    foreach (ExchangeData data, south) {
        double net = data.mNet/100000000.0;
        lines.append(QString("%1(%2)").arg(data.mName).arg(getRichTextString(net)));
    }
    totalContent.append(lines.join(" ,"));
#endif
    appendText(totalContent);
}


void HqInfoNews::slotRecvNorthMoney(double total, double sh, double sz)
{
    QStringList totalContent;
    totalContent.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString::fromLocal8Bit("沪深港通实时资金流更新"));
    totalContent.append(QString::fromLocal8Bit("北向方面：%1亿， 其中上海%2亿，深圳%3亿").arg(getRichTextString(total / 10000.0)).arg(getRichTextString(sh/10000.0)).arg(getRichTextString(sz/10000.0)));
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
    titles.append(QString::fromLocal8Bit("名称"));
    titles.append(QString::fromLocal8Bit("申购日期"));
    titles.append(QString::fromLocal8Bit("股权登记"));
    titles.append(QString::fromLocal8Bit("确保配售"));
    titles.append(QString::fromLocal8Bit("转股价值"));
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

void HqInfoNews::appendText(const QStringList &list)
{
    foreach (QString text, list) {
        this->append(text);
    }
    adjustPostion();
    this->show();
    if(mDisplaytimer) mDisplaytimer->start();
}

void HqInfoNews::adjustPostion()
{
    QRect rect = QApplication::desktop()->availableGeometry();
    this->setFixedWidth(rect.width() * 0.2);
    this->document()->setTextWidth(this->width());
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

void HqInfoNews::slotRecvHqRtDataList(const HqRtDataList &list)
{
    if(list.size() == 0) return;
    QStringList totalContent;
    QStringList titles;
    titles.append(QString::fromLocal8Bit("名称"));
    titles.append(QString::fromLocal8Bit("涨跌幅(%)"));
    titles.append(QString::fromLocal8Bit("成交金额(亿)"));
    totalContent.append(titles.join("    "));
    for(int i=0; i<list.size(); i++)
    {
        HqRtData data = list[i];
        titles.clear();
        titles.append(data.mName);
        titles.append(getRichTextString(data.mChgPercnt));
        titles.append(QString::number(data.mTotal, 'f', 0));
        totalContent.append(titles.join("    "));
    }
    appendText(totalContent);
}
