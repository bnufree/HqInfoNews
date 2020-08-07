#include "hqinfonews.h"
#include "inforollingwidget.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
    HqInfoNews w(0);
//    w.setInfoRollingWidget(roll);
    w.hide();


    return a.exec();
}
