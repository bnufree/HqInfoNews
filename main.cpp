#include "hqinfonews.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HqInfoNews w;
    w.hide();

    return a.exec();
}
