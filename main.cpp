#include "TorClientWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TorClientWindow w;
    w.show();

    return a.exec();
}
