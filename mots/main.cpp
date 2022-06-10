#include "mots.h"
#include <QtWidgets/QApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //single instance check
    QSharedMemory sm("Monkey_of_the_Sea");
    mots w;
    if (!sm.create(16, QSharedMemory::ReadWrite)) {
        QMessageBox::information(0, "Error - mots", "There is already an instance of mots running.", QMessageBox::Ok);
        return 0;
    }

    w.show();
    return a.exec();
}
