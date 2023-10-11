/** Copyright (C) Fuqing Medical and USTC BMEC RFLab - All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited.
 ** Proprietary and confidential.
 ** Created on 20230807, by yue.wang.
 ** brief:  This class is the base class for tcp communication
 **/
#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QSharedPointer>
#include "propSetGet.h"

class ClientWorker : public QObject
{
    Q_OBJECT
    LAB_PROP_RS(enabled, bool, Enabled);
    LAB_PROP_RS(connected, bool, Connected);
    LAB_PROP_RS(lastError, QString, LastError);

public:
    explicit ClientWorker(QObject* parent = nullptr);
    virtual ~ClientWorker();

    /// check current server ip
    bool checkHost(const QString &ip);

    /// check current port
    bool checkPort(const int port);

protected:
    bool sendData(const QByteArray& data);

    /// parse m_readData
    virtual void parseBuffer() = 0;

Q_SIGNALS:
    LAB_PROP_SIGNAL(enabled, bool)
    LAB_PROP_SIGNAL(connected, bool)
    LAB_PROP_SIGNAL(lastError, QString);

protected Q_SLOTS:
    /// when the connection has been successfully established
    void slotConnected();

    /// when the socket has been disconnected
    void slotDisconnected();

    /// when an error occurred
    void slotError(QAbstractSocket::SocketError error);

    /// when socket is ready to read
    virtual void slotReadyRead() = 0;

    /**
     * @brief slotBytesWritten: When a payload of data has been written to the device's current write channel.
     * @param bytes             The number of bytes that were written in this payload
     */
    void slotBytesWritten(qint64 bytes);

public Q_SLOTS:
    /// establish connects for tcp socket
    virtual void init();

    /// read config file and open tcp socket.
    virtual void open(const bool closeAndOpen = true) = 0;

    /// close TCP socket and update related status members.
    void close();

protected:
    QSharedPointer<QTcpSocket> m_tcpSocket;
    QByteArray m_writeData;
    QByteArray m_readData;
    qint64 m_bytesWritten;
    QString m_host;
    qint16 m_port;
};

#endif // CLIENTWORKER_H
