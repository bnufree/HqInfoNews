#include "settingscfg.h"
#include "ui_settingscfg.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QCloseEvent>
#include "profiles.h"
#include <QTimer>
#include "qhttpget.h"
#include <QListWidget>
#include <QScrollBar>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include "hqrealtimethread.h"

#define     CODE_PROPERTY       "code"

struct SuggestData{
    QString mCode;
    QString mName;
};

enum FUNC_TYPE{
    RT_INFO = 0,
    RT_HQ_ZXG,
    RT_HQ_INDEX,
    RT_NORTH_ZJLX,
    RT_HQ_NORTH_TOP10,
};

SettingsCfg::SettingsCfg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsCfg)
{
    ui->setupUi(this);
    mSuggestWidget = 0;
    mRow = 0;
    mCol = 0;
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedSize(600, 400);
    QRect src = this->geometry();
    QRect dest = QApplication::desktop()->availableGeometry();
    src.moveCenter(dest.center());
    this->setGeometry(src);

    //
    ui->sh_sz_Index->setProperty(CODE_PROPERTY, "sh000001");
    ui->sh_hs300_Index->setProperty(CODE_PROPERTY, "sh000300");
    ui->sh_kc50_Index->setProperty(CODE_PROPERTY, "sh000688");
    ui->sh_sz50_Index->setProperty(CODE_PROPERTY, "sh000016");

    ui->sz_sz_Index->setProperty(CODE_PROPERTY, "sz399001");
    ui->sz_cyb_index->setProperty(CODE_PROPERTY, "sz399006");
    ui->sz_cyb50_index->setProperty(CODE_PROPERTY, "sz399673");
    ui->sz_zxb_Index->setProperty(CODE_PROPERTY, "sz399005");

    ui->us_gold->setProperty(CODE_PROPERTY, "hf_GC");
    ui->us_silver->setProperty(CODE_PROPERTY, "hf_SI");
    ui->ft_A50->setProperty(CODE_PROPERTY, "hf_CHA50CFD");
    ui->hk_hs_Index->setProperty(CODE_PROPERTY, "rt_hkHSI");

    mIndexCheckList<<ui->sh_hs300_Index<<ui->sh_kc50_Index<<ui->sh_sz50_Index<<ui->sh_sz_Index<<ui->sz_cyb50_index<<ui->sz_cyb_index<<ui->sz_sz_Index<<ui->sz_zxb_Index;
    mIndexCheckList<<ui->us_gold<<ui->us_silver<<ui->ft_A50<<ui->hk_hs_Index;
    connect(ui->sh_sz_Index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->sh_hs300_Index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->sh_kc50_Index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->sh_sz50_Index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));

    connect(ui->sz_sz_Index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->sz_cyb_index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->sz_cyb50_index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->sz_zxb_Index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));

    connect(ui->us_gold, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->us_silver, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->ft_A50, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));
    connect(ui->hk_hs_Index, SIGNAL(clicked(bool)), this, SLOT(slotIndexCodesEnabled(bool)));

//    PROFILES_INSTANCE->setDefault("Hq", "Index", QString("s_sh000001,s_sh000688,s_sz399001,s_sz399006,rt_hkHSI,hf_GC,hf_CHA50CFD").split(","));
//    PROFILES_INSTANCE->setDefault("Hq", "Zxg", QString("300059,002351,000069,000021").split(","));

    mIndexList = PROFILES_INSTANCE->value("Hq", "Index").toStringList();
    foreach (QString code, mIndexList) {
        foreach (QCheckBox* chk, mIndexCheckList) {
            if(chk->property(CODE_PROPERTY).toString() == code){
                chk->setChecked(true);
                break;
            }
        }
    }

    QTimer::singleShot(1000, this, [=](){emit signalAddIndex(mIndexList);});

    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, [=](QTableWidgetItem* item)
    {
        //将当前从自选股删除
        int row = item->row();
        int col = item->column();
        item = ui->tableWidget->takeItem(row, col);
        QString wkCode = PROFILES_INSTANCE->formatCode(item->data(Qt::UserRole).toString());
        if(mZxgList.contains(wkCode))
        {
            mZxgList.removeOne(wkCode);
            emit signalRemoveShareCode(QStringList()<<wkCode);
            PROFILES_INSTANCE->setValue("Hq", "Zxg", mZxgList);
        }
        //将后面的单元格移动
        delete item;
        QList<SuggestData> moveList;
        mCol = col;
        mRow = row;
        while (row <= ui->tableWidget->rowCount() - 1) {
            col = (col + 1 ) % 5;
            if(col == 0) row ++;
            QTableWidgetItem *nextItem = 0;
            if(row <= ui->tableWidget->rowCount() - 1)
            {
                nextItem = ui->tableWidget->takeItem(row, col);
            }
            if(nextItem)
            {
                SuggestData data;
                data.mName = nextItem->text();
                data.mCode = nextItem->data(Qt::UserRole).toString();
                moveList.append(data);
                delete nextItem;
            } else
            {
                break;
            }
        }
        for(int i=0; i<ui->tableWidget->rowCount();)
        {
            bool empty = true;
            for(int k=0; k<ui->tableWidget->columnCount(); k++)
            {
                QTableWidgetItem *item = ui->tableWidget->item(i, k);
                if(item)
                {
                    empty = false;
                    break;
                }
            }
            if(empty)
            {
                ui->tableWidget->removeRow(i);
            } else
            {
                i++;
            }
        }

        foreach (SuggestData data, moveList) {
            slotInsertCode(data.mName, data.mCode);
        }

    });

    mZxgList = PROFILES_INSTANCE->value("Hq", "Zxg").toStringList();
    qDebug()<<mZxgList;
    HqRtDataList rtList = HqRealtimeThread::getHqRtDataList(mZxgList);
    foreach (HqRtData data, rtList) {
        slotInsertCode(data.mName, data.mCode);
    }
    //更新功能项目
    ui->info_rt->setProperty("TYPE", RT_INFO);
    ui->hq_zxg->setProperty("TYPE", RT_HQ_ZXG);
    ui->hq_index->setProperty("TYPE", RT_HQ_INDEX);
    ui->northbound_rt->setProperty("TYPE", RT_NORTH_ZJLX);
    ui->borthbound_top->setProperty("TYPE", RT_HQ_NORTH_TOP10);
    connect(ui->info_rt, SIGNAL(toggled(bool)), this, SLOT(slotFuncCheckBoxChanged(bool)));
    connect(ui->hq_zxg, SIGNAL(toggled(bool)), this, SLOT(slotFuncCheckBoxChanged(bool)));
    connect(ui->hq_index, SIGNAL(toggled(bool)), this, SLOT(slotFuncCheckBoxChanged(bool)));
    connect(ui->northbound_rt, SIGNAL(toggled(bool)), this, SLOT(slotFuncCheckBoxChanged(bool)));
    connect(ui->borthbound_top, SIGNAL(toggled(bool)), this, SLOT(slotFuncCheckBoxChanged(bool)));


    bool isInfo = PROFILES_INSTANCE->value("Hq", "Info_Enabled", true).toBool();
    bool isZxg = PROFILES_INSTANCE->value("Hq", "ZXG_Enabled", true).toBool();
    bool isIndex = PROFILES_INSTANCE->value("Hq", "Index_Enabled", true).toBool();
    bool isRtNorth = PROFILES_INSTANCE->value("Hq", "RtNorth_Enabled", true).toBool();
    bool isRtNorthTop10 = PROFILES_INSTANCE->value("Hq", "RtNorthTop10_Enabled", true).toBool();

    ui->info_rt->setChecked(isInfo);
    ui->hq_zxg->setChecked(isZxg);
    ui->hq_index->setChecked(isIndex);
    ui->northbound_rt->setChecked(isRtNorth);
    ui->borthbound_top->setChecked(isRtNorthTop10);

    QTimer::singleShot(1000, this, [=](){
        emit signalAddIndex(mIndexList);
        emit signalAddShareCode(mZxgList);
    });




}

void SettingsCfg::closeEvent(QCloseEvent * e)
{
    e->ignore();
    hide();
}

SettingsCfg::~SettingsCfg()
{
    delete ui;
}

void SettingsCfg::slotIndexCodesEnabled(bool sts)
{
    QCheckBox *chk = qobject_cast<QCheckBox*>(sender());
    if(!chk) return;
    QString code = chk->property(CODE_PROPERTY).toString();
    if(sts)
    {
        emit signalAddIndex(QStringList()<<code);
        mIndexList.append(code);
    } else
    {
        emit signalRemoveIndex(QStringList()<<code);
        mIndexList.removeOne(code);
    }

    PROFILES_INSTANCE->setValue("Hq", "Index", mIndexList);
}

void SettingsCfg::on_lineEdit_textChanged(const QString &arg1)
{
    if(arg1.isEmpty() && mSuggestWidget)
    {
        mSuggestWidget->hide();
        return;
    }
    QString url = QString("http://suggest3.sinajs.cn/suggest/type=11,31,22,81&key=%1&name=").arg(arg1);
    QString result = QString::fromUtf8(QHttpGet::getContentOfURL(url));
    result = result.mid(result.indexOf("\""));
    if(result.size() == 0) return;
    result.replace("\"", "");
    QList<SuggestData> resultList;
    QStringList lines = result.split(";", QString::SkipEmptyParts);
    foreach (QString line, lines) {
        QStringList data_list = line.split(",", QString::SkipEmptyParts);
        if(data_list.size() <= 5) continue;
//        qDebug()<<line;
        SuggestData data;
        data.mCode = data_list[3];
        data.mCode = PROFILES_INSTANCE->formatCode(data.mCode);
        data.mName = data_list[4];
        resultList.append(data);
//        qDebug()<<data.mCode<<data.mName;
    }
    if(resultList.size() == 0) return;
    if(mSuggestWidget == 0)
    {
        mSuggestWidget = new QListWidget(ui->groupBox_2);
        mSuggestWidget->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::FramelessWindowHint);

        mSuggestWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
//        mSuggestWidget->setSizePolicy(QSizePolicy::Preferred);
        mSuggestWidget->setFixedSize(200, 400);
        mSuggestWidget->horizontalScrollBar()->setVisible(false);
        mSuggestWidget->verticalScrollBar()->setVisible(false);
        connect(mSuggestWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));

    }
    mSuggestWidget->clear();
    int num = 0;
    foreach (SuggestData data, resultList) {
        mSuggestWidget->addItem(QString("%1  %2").arg(data.mName).arg(data.mCode));
        mSuggestWidget->item(mSuggestWidget->count()-1)->setData(Qt::UserRole, data.mCode);
        mSuggestWidget->item(mSuggestWidget->count()-1)->setData(Qt::UserRole+1, data.mName);
        num ++;
        if(num >= 10) break;
    }
    QPoint pnt = ui->tableWidget->geometry().topLeft();
    pnt.setY(pnt.y() + 1);
    mSuggestWidget->move(pnt);

    mSuggestWidget->show();
    mSuggestWidget->raise();
}

void SettingsCfg::slotItemDoubleClicked(QListWidgetItem *item)
{
    if(!item) return;
    QString code = PROFILES_INSTANCE->formatCode(item->data(Qt::UserRole).toString());
    emit signalAddShareCode(QStringList()<<code);
    slotInsertCode(item->data(Qt::UserRole+1).toString(), code);
    mSuggestWidget->hide();
}

void SettingsCfg::slotInsertCode(const QString &name, const QString &code)
{
    QString wkCode = code;
//    int index = wkCode.indexOf(QRegExp("[0-9]"));
//    if(index != 0 ) wkCode = wkCode.mid(index);
    if(mCol == 0 || mCol == 5)
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        mCol = 0;
        mRow = ui->tableWidget->rowCount() -1;
    }
    ui->tableWidget->setItem(mRow, mCol, new QTableWidgetItem(name));
    ui->tableWidget->item(mRow, mCol)->setData(Qt::UserRole, wkCode);
    ui->tableWidget->item(mRow, mCol)->setTextAlignment(Qt::AlignCenter);
    mCol++;
    if(!mZxgList.contains(wkCode))
    {
        mZxgList.append(wkCode);        
    }
    PROFILES_INSTANCE->setValue("Hq", "Zxg", mZxgList);
    qDebug()<<mZxgList;
}



void SettingsCfg::slotFuncCheckBoxChanged(bool sts)
{
    QCheckBox* box = qobject_cast<QCheckBox*>(sender());
    if(!box) return;
    switch (box->property("TYPE").toInt()) {
    case RT_INFO:
        PROFILES_INSTANCE->value("Hq", "Info_Enabled", sts);
        emit signalEnableRtInfo(sts);
        break;
    case RT_HQ_INDEX:
        PROFILES_INSTANCE->value("Hq", "Index_Enabled", sts);
        emit signalEnableRtIndex(sts);
        break;
    case RT_HQ_ZXG:
        PROFILES_INSTANCE->value("Hq", "ZXG_Enabled", sts);
        emit signalEnableRtZxg(sts);
        break;
    case RT_HQ_NORTH_TOP10:
        PROFILES_INSTANCE->value("Hq", "RtNorthTop10_Enabled", sts);
        emit signalEnableRtnorthTop10(sts);
        break;
    case RT_NORTH_ZJLX:
        PROFILES_INSTANCE->value("Hq", "RtNorth_Enabled", sts);
        emit signalEnableRtnorth(sts);
        break;
    default:
        break;
    }
}


