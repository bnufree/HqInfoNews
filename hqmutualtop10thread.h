#ifndef HQMUTUALTOP10THREAD_H
#define HQMUTUALTOP10THREAD_H

#include <QThread>

struct ExchangeData{
    QString     mCode;
    QString     mName;
    double      mBuy;
    double      mSell;
    double      mTotal;
    double      mNet;
    double      mCur;
    double      mChgPercnt;

    bool operator <(const ExchangeData& other) const
    {
        return this->mNet < other.mNet;
    }

    bool operator >(const ExchangeData& other) const
    {
        return this->mNet > other.mNet;
    }

};

struct MutualTop10Data{
    QString     mDate;
    QList<ExchangeData> mExchangeDataList;
};

class HqMutualTop10Thread : public QThread
{
    Q_OBJECT
public:
    explicit HqMutualTop10Thread(QObject *parent = nullptr);

    void run();
private:
    void getRtNorthMoneyInfo();

signals:
    void  signalSendTop10DataList(const QDate& date, const QList<ExchangeData>& north, const QList<ExchangeData>& south);
    void  signalSendNorthMoney(double total, double sh, double sz);
public slots:
};

#endif // HQMUTUALTOP10THREAD_H
