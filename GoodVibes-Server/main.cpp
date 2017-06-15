#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString port(argv[1]);
    if (!port.isEmpty()) {
        Server server(port.toInt());
        server.start();
    }
    else {
        qDebug() << "You didn't enter the port number.";
        exit(1);
    }

    return a.exec();
}
