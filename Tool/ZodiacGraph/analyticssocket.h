#ifndef ANALYTICSSOCKET_H
#define ANALYTICSSOCKET_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QAbstractSocket>

class AnalyticsSocket : public QObject
{
    Q_OBJECT
public:
    explicit AnalyticsSocket(QObject *parent = 0);

    void SetUpSocket(QString address, int port);

signals:

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    QTcpSocket *socket;
};

#endif // ANALYTICSSOCKET_H
