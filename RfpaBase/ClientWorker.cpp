/** Copyright (C) Fuqing Medical and USTC BMEC RFLab - All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited.
 ** Proprietary and confidential.
 ** Created on 20230807, by yue.wang.
 ** brief:  This class is the base class for tcp communication
 **/

#include "clientworker.h"
#include <QDebug>
#include <QDoubleValidator>

ClientWorker::ClientWorker(QObject *parent)
    : QObject{parent}, m_tcpSocket(nullptr), m_bytesWritten(0), m_port(0)
{
}

ClientWorker::~ClientWorker()
{
    close();
}

bool ClientWorker::checkHost(const QString &ip)
{
    QStringList ipAddr = ip.split(".", Qt::SkipEmptyParts);
    QDoubleValidator validator(0, 255, 0);
    int pos = 0;
    Q_FOREACH(auto s, ipAddr){
        if(validator.validate(s, pos) != QValidator::Acceptable){
            return false;
        }
    }
    return true;
}

bool ClientWorker::checkPort(const int port)
{
    int pos = 0;
    QDoubleValidator validator(0, 65535, 0);
    auto portStr = QString::number(port);
    auto state = validator.validate(portStr, pos);
    return state == QValidator::Acceptable;
}

bool ClientWorker::sendData(const QByteArray &data)
{
    qDebug() << "sendData:" << data;
    qDebug() << "sendData:" << data.toHex();
    m_writeData = data;
    const qint64 bytesWritten = m_tcpSocket->write(data);
    if(bytesWritten == -1){
        qWarning() << "Failed to write the data:" << m_tcpSocket->errorString();
        return false;
    }else if(bytesWritten != m_writeData.size()){
        qWarning() << "Failed to write all the data" << m_tcpSocket->errorString();
        return false;
    }
    return true;
}

void ClientWorker::slotConnected()
{
    setConnected(true);
    qDebug() << "Tcp socket has been successfully established:" << m_host << m_port;
}

void ClientWorker::slotDisconnected()
{
    setConnected(false);
    qDebug() << "Tcp socket has been disconnected:" << m_host << m_port;
}

void ClientWorker::slotError(QAbstractSocket::SocketError error)
{
    QString err = m_tcpSocket->errorString();
    setLastError(err);
    qWarning() << "Tcp socket(" << m_host << m_port << ") error:" << err;
}

void ClientWorker::slotBytesWritten(qint64 bytes)
{
    m_bytesWritten += bytes;
    if(m_bytesWritten == m_writeData.size()){
        m_bytesWritten = 0;
        qDebug() << "Tcp data successfully sent" << m_writeData.size() << "bytes";
        return;
    }
}

void ClientWorker::init()
{
    qDebug() << "ClientWorker::init()";
    m_tcpSocket = QSharedPointer<QTcpSocket>(new QTcpSocket, &QObject::deleteLater);
    m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    // TODO: not need ?
    m_tcpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 64*1024*1024);
    connect(m_tcpSocket.get(), &QTcpSocket::connected, this, &ClientWorker::slotConnected);
    connect(m_tcpSocket.get(), &QTcpSocket::disconnected, this, &ClientWorker::slotDisconnected);
    connect(m_tcpSocket.get(), &QTcpSocket::errorOccurred, this, &ClientWorker::slotError);
    connect(m_tcpSocket.get(), &QTcpSocket::readyRead, this, &ClientWorker::slotReadyRead);
    connect(m_tcpSocket.get(), &QTcpSocket::bytesWritten, this, &ClientWorker::slotBytesWritten);
    open();
}

void ClientWorker::close()
{
    if(m_tcpSocket && m_tcpSocket->isOpen()){
        m_tcpSocket->close();
    }
    setConnected(false);
}
