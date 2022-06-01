#include "mots.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mots w;
    w.show();
    return a.exec();
}
