﻿#ifndef SETTINGSCFG_H
#define SETTINGSCFG_H

#include <QDialog>

class QCheckBox;
class QListWidget;
class QListWidgetItem;

namespace Ui {
class SettingsCfg;
}

class SettingsCfg : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsCfg(QWidget *parent = 0);
    ~SettingsCfg();
private slots:
    void slotIndexCodesEnabled(bool sts);
    void on_lineEdit_textChanged(const QString &arg1);
    void slotItemDoubleClicked(QListWidgetItem*);
    void slotInsertCode(const QString& name, const QString& code);
    void slotFuncCheckBoxChanged(bool sts);

protected:
    void  closeEvent(QCloseEvent *);
signals:
    void  signalAddIndex(const QStringList& list);
    void  signalRemoveIndex(const QStringList& list);
    void  signalAddShareCode(const QStringList& list);
    void  signalRemoveShareCode(const QStringList& list);
    void  signalEnableRtInfo(bool sts);
    void  signalEnableRtZxg(bool sts);
    void  signalEnableRtIndex(bool sts);
    void  signalEnableRtnorth(bool sts);
    void  signalEnableRtnorthTop10(bool sts);

private:
    Ui::SettingsCfg *ui;
    QList<QCheckBox*>         mIndexCheckList;
    QStringList               mIndexList;
    QListWidget*              mSuggestWidget;
    int                       mRow;
    int                       mCol;
    QStringList               mZxgList;
};

#endif // SETTINGSCFG_H
