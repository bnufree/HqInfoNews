#include "inforollingwidget.h"
#include <QResizeEvent>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QHBoxLayout>
#include "profiles.h"

#pragma execution_character_set("utf-8")
#define         LABEL_SIZE          250

InfoRollingWidget::InfoRollingWidget(QWidget *parent) : QWidget(parent)
{
    mLastIndex = 0;
    mShareThread = 0;
    mIndexThread = 0;
    this->setWindowFlags(windowFlags() |Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    timer->start();
    this->setStyleSheet("background-color:transparent; font-weight:bold; font-size:12pt; font-family: Microsoft YaHei; color:white;");

    mShareThread = new HqRealtimeThread(this);
//    mRtThread->appendCodes(QStringList()<<"300059"<<"159949"<<"002475"<<"159995"<<"002351");
    connect(mShareThread, SIGNAL(signalSendHqRtDataList(HqRtDataList)), this, SLOT(slotRecvHqRtDataList(HqRtDataList)));
    mShareThread->start();

    mIndexThread = new HqRealtimeThread(this);
//    mIndexThread->appendCodes(QStringList()<<"s_sh000001"<<"rt_hkHSI"<<"sz399006"<<"hf_GC"<<"hf_SI"<<"hf_CL"<<"hf_OIL"<<"hf_CHA50CFD");
    connect(mIndexThread, SIGNAL(signalSendHqRtDataList(HqRtDataList)), this, SLOT(slotRecvHqRtDataList(HqRtDataList)));
    mIndexThread->start();
    this->setLayout(new QHBoxLayout);
    this->layout()->setMargin(0);

}


void InfoRollingWidget::slotRecvHqRtDataList(const HqRtDataList &list)
{
    if(list.size() == 0) return;
    foreach (HqRtData data, list) {
        mRollDataMap[data.mCode] = data;
    }
}



QString InfoRollingWidget::getHtmlRichText(const HqRtDataList& list)
{
    QStringList result_list;
    result_list.append("<html><head/><body>");
    foreach (HqRtData data, list) {
        result_list.append(data.mName);
        QString colorName;
        double change_percent = data.mChgPercnt;
        if(change_percent >= 0.00)
        {
            colorName = QColor(Qt::red).name();
        } else
        {
            colorName = QColor(Qt::green).name();
        }
        result_list.append(QString("<span style=\" font-weight:600; color:%1;\">%2   %3%</span>")
                           .arg(colorName)
                           .arg(data.mCur, 0, 'f', 3)
                           .arg(data.mChgPercnt, 0, 'f', 2));
    }
    result_list.append("</body></html>");
    return result_list.join(" ");
}

void InfoRollingWidget::moveToBottom()
{
    QRect total_rect = QApplication::desktop()->screenGeometry();
    QRect rect = QApplication::desktop()->availableGeometry();
    int test_height = total_rect.height() - rect.height();
    this->setFixedHeight(test_height);
    this->setFixedWidth(800);
    this->move(800, rect.height());
}

void InfoRollingWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    int totalWidth = e->size().width();
    int count = totalWidth / LABEL_SIZE;
    foreach (QLabel* l, mDisplayLabelList) {
        this->layout()->removeWidget(l);
        delete l;
    }



    for(int i=0; i<count; i++)
    {
        QLabel * l = new QLabel(this);
        l->setAlignment(Qt::AlignCenter);
        layout()->addWidget(l);
        mDisplayLabelList.append(l);
    }
}

void InfoRollingWidget::slotAppendText(const QString &text)
{
//    mRollText.append(text);
}

void InfoRollingWidget::slotTimeOut()
{
    if(mRollDataMap.size() == 0) return;
    HqRtDataList list = mRollDataMap.values();
    int size = list.size();
    foreach (QLabel * l, mDisplayLabelList) {
        l->setText(getHtmlRichText(HqRtDataList()<<list[mLastIndex % size]));
        mLastIndex = (++mLastIndex) % list.size();
    }
    raise();
}

void InfoRollingWidget::slotAppendIndex(const QStringList &codes)
{
    if(mIndexThread) mIndexThread->appendCodes(codes);
}

void InfoRollingWidget::slotAppendShare(const QStringList &codes)
{
    if(mShareThread) mShareThread->appendCodes(codes);
}

void InfoRollingWidget::slotRemoveIndex(const QStringList &codes)
{
    if(mIndexThread) mIndexThread->removeCodes(codes);
    foreach (QString code, codes) {
        mRollDataMap.remove(PROFILES_INSTANCE->formatCode(code));
    }
}

void InfoRollingWidget::slotRemoveShare(const QStringList &codes)
{
    if(mShareThread) mShareThread->removeCodes(codes);
    foreach (QString code, codes) {
        mRollDataMap.remove(PROFILES_INSTANCE->formatCode(code));
    }
}


