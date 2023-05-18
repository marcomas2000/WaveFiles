#include "waveinspector.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WaveInspector w;
    w.show();

    return a.exec();
}
