#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString port(argv[1]);
    qDebug() << port;
    if (!port.isEmpty()) {
        int nPort = port.toInt();
        Server server(nPort);
        server.start();
    }
    else {
        qDebug() << "You didn't enter the port number.";
        exit(1);
    }

    return a.exec();
}
