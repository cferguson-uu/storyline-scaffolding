#include "analyticssocket.h"

AnalyticsSocket::AnalyticsSocket(QObject *parent) : QObject(parent)
{

}

void AnalyticsSocket::SetUpSocket(QString address, int port)
{
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten()), this, SLOT(bytesWritten()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    qDebug() << "Connecting...";

    socket->connectToHost(address, port);

    if(!socket->waitForConnected(5000))
    {
        qDebug() << "Error: " << socket->errorString();
    }
}

void AnalyticsSocket::connected()
{
    qDebug() << "Connected.";

    socket->write("Send me some stuff");
}

void AnalyticsSocket::disconnected()
{
    qDebug() << "Disconnected.";
}
void AnalyticsSocket::bytesWritten(qint64 bytes)
{
    qDebug() << "Wrote: " << bytes;
}

void AnalyticsSocket::readyRead()
{
    qDebug() << "Reading...";
    qDebug() << socket->readAll();
}
