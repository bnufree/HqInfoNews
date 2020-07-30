#include "hqinfonews.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
    HqInfoNews w;
    w.hide();

    return a.exec();
}
