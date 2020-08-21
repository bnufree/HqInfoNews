#ifndef DATATHREADMGR_H
#define DATATHREADMGR_H

#include <QObject>

class DataThreadMgr : public QObject
{
    Q_OBJECT
private:
    explicit DataThreadMgr(QObject *parent = nullptr);
public:
    static DataThreadMgr* instance();
    void   appendThread(int id);
    void   removeThread(int id);

signals:

public slots:

private:
    static DataThreadMgr* m_pInstance;
};

#endif // DATATHREADMGR_H
