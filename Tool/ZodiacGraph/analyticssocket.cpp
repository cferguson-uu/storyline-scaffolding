#include "analyticssocket.h"

AnalyticsSocket::AnalyticsSocket(/*AnalyticsLogWindow* analyticsLog,*/ QWidget *parent)
    : QDialog(parent)
 //   , m_analyticsLog(analyticsLog)
{

}

void AnalyticsSocket::SetUpSocket()
{
    QLabel *ipLabel = new QLabel("IP: ");
    QLineEdit *ipField = new QLineEdit();

    QLabel *portLabel = new QLabel("Port: ");
    QLineEdit *portField = new QLineEdit();

    QPushButton *connectBtn = new QPushButton("Connect");
    QPushButton *cancelBtn = new QPushButton("Cancel");

    connect(connectBtn, &QPushButton::released, [=]{connectToServer(ipField->text(), portField->text().toInt());});

    connect(cancelBtn, &QPushButton::released, [=]{this->hide();});

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(ipLabel, 0, 0);
    mainLayout->addWidget(ipField, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portField, 1, 1);
    mainLayout->addWidget(connectBtn);
    mainLayout->addWidget(cancelBtn);
    setLayout(mainLayout);

    setWindowTitle(tr("Connect to Server"));

    this->show();
}

void AnalyticsSocket::connectToServer(QString address, int port)
{
    if(address == "")
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Address cannot be empty.");
        messageBox.setFixedSize(500,200);
    }
    else
        if(port == 0)
        {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","Port is invalid.");
            messageBox.setFixedSize(500,200);
        }
    else
        {
            m_address = address;
            m_port = port;

            m_socket = new QTcpSocket(this);

            connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
            connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
            connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
            connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

            qDebug() << "Connecting...";

            m_socket->connectToHost(address, port);

            this->hide();

            if(!m_socket->waitForConnected(5000))
            {
                QMessageBox messageBox;
                messageBox.critical(0,"Error",m_socket->errorString());
                messageBox.setFixedSize(500,200);
                this->show();
            }
        }
}

void AnalyticsSocket::connected()
{
    qDebug() << "Connected.";
    QMessageBox messageBox;
    messageBox.information(0, "Connected", "Connected to: " + m_address + ":" + QString::number(m_port));
    messageBox.setFixedSize(500,200);

    //m_socket->write("Send me some stuff");

    //m_analyticsLog->initialiseLogFile();

    connectedCallback();
}

void AnalyticsSocket::disconnected()
{
    qDebug() << "Disconnected.";

    QMessageBox messageBox;
    messageBox.information(0, "Disconnected", "Disconnected from: " + m_address + ":" + QString::number(m_port));
    messageBox.setFixedSize(500,200);

    m_socket->deleteLater();
    m_socket->close();

    disconnectedCallback();
}

void AnalyticsSocket::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void AnalyticsSocket::bytesWritten(qint64 bytes)
{
    qDebug() << "Wrote: " << bytes;
}

void AnalyticsSocket::readyRead()
{
    //qDebug() << "Reading...";
    QString data = m_socket->readAll();
    //qDebug() << data;
    //qDebug() << m_socket->readAll();
    //m_analyticsLog->appendMessage(data);
    //m_socket->write(data.toStdString().c_str());

    readMessage(data);
}
