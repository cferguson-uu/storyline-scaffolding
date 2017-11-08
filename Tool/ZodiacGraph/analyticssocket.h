#ifndef ANALYTICSSOCKET_H
#define ANALYTICSSOCKET_H

#include <QDialog>
#include <QDebug>

#include <QTcpSocket>
#include <QAbstractSocket>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>

#include "analyticslogwindow.h"

class AnalyticsSocket : public QDialog
{
    Q_OBJECT
public:
    explicit AnalyticsSocket(/*AnalyticsLogWindow* analyticsLog, */QWidget *parent = 0);

signals:
    void connectedCallback();
    void disconnectedCallback();
    void readMessage(QString message);

public slots:
    void SetUpSocket();

    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    void connectToServer(QString address, int port);

    QTcpSocket *m_socket;
    AnalyticsLogWindow *m_analyticsLog;

    QString m_address;
    int m_port;
};

#endif // ANALYTICSSOCKET_H
