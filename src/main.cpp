#include "sc_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SC_Window w;
    w.show();

    return a.exec();
}
