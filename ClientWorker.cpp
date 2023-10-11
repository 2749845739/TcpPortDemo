#include "ClientWorker.h"

ClientWorker::ClientWorker(QObject *parent)
    : QObject{parent}, m_socket(nullptr), m_bytesWritten(0)
{
    qDebug() << "ClientWorker";
    m_socket = new QTcpSocket();
    connect(m_socket, &QTcpSocket::connected, this, &ClientWorker::slotConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientWorker::slotDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, &ClientWorker::slotError);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientWorker::slotReadyRead);
    connect(m_socket, &QTcpSocket::bytesWritten, this, &ClientWorker::slotReadyRead);
}

bool ClientWorker::connectToServer(const QString &host, quint16 port)
{
    qDebug() << "connectToServer:" << host << port;
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected();
}

bool ClientWorker::sendData(const QByteArray &data)
{
    qDebug() << "sendData:" << data.toHex();
    m_writeData = data;
    const qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        qWarning() << "Failed to write the data:" << m_socket->errorString();
        return false;
    } else if (bytesWritten != m_writeData.size()) {
        qWarning() << "Failed to write all the data" << m_socket->errorString();
        return false;
    }
    return true;
}

void ClientWorker::parseBuffer()
{
    qDebug() << "parseBuffer:" << m_readData;
    // TODO
    QString bufferData = m_readData.constData();
//    QRegExp regex("5AA540(.{2})(.{8})");
    QRegExp regex{"\"actionType\":\"PULSE_POWER\",\"incident\":424.792,\"reflection\":414.394}{\"actionType\":\"PULSE_POWER\",\"incident\":429.475,\"reflection\":419.491}"};
    int pos = 0;
    while ((pos = regex.indexIn(bufferData, pos)) != -1) {
        QString matchedValue = regex.cap(0);
        qDebug() << "matchedValue:" << matchedValue;
        QString actionType = regex.cap(1);
        QString incident = regex.cap(2);

        m_readData.remove(0, pos + regex.matchedLength());  // 从缓存中删除匹配到的数据
        pos += regex.matchedLength();  // 继续从下一个位置开始匹配
    }
}

void ClientWorker::sendString(const QString &data)
{
    qDebug() << "sendString:" << data;
    QByteArray byteArray;
    for (int i = 0; i < data.length(); i += 2) {
        QString hex = data.mid(i, 2);
        bool ok;
        quint8 value = hex.toUInt(&ok, 16);
        if (ok) {
            byteArray.append(value);
        }
    }
    sendData(byteArray);
}

void ClientWorker::resetRFMod()
{
    qDebug() << "resetRFMod";
    // use modbus protocol
    QByteArray data;
    data.append('\x01'); data.append('\x06');
    data.append('\x01'); data.append('\x00');
    data.append('\x00'); data.append('\x01');
    data.append('\x49'); data.append('\xf6');
    sendData(data);
}

void ClientWorker::getPeek()
{
    qDebug() << "getPeek";
    QByteArray data = QByteArrayLiteral("\x00\x00\x00\x01");
    sendData(data);
}

void ClientWorker::clearCache()
{
    qDebug() << "clearCache";
    m_socket->readAll();
}

void ClientWorker::disconnectFromServer()
{
    qDebug() << "disconnectFromServer";
    m_socket->disconnectFromHost();
    m_socket->waitForDisconnected();
}

void ClientWorker::slotConnected()
{
    qDebug() << "slotConnected";
}

void ClientWorker::slotDisconnected()
{
    qDebug() << "slotDisconnected";

}

void ClientWorker::slotError(QAbstractSocket::SocketError error)
{
    qDebug() << "slotError";
    qWarning() << "Socket error:" << m_socket->errorString();
}

void ClientWorker::slotReadyRead()
{
    QByteArray data = m_socket->readAll();
    qDebug() << "new data:" << data;
    qDebug() << "new hex :" << data.toHex();
    m_readData.append(data);
    parseBuffer();
}

void ClientWorker::slotBytesWritten(qint64 bytes)
{
    m_bytesWritten += bytes;
    if (m_bytesWritten == m_writeData.size()) {
        m_bytesWritten = 0;
        qDebug() << "Data successfully sent";
        return;
    }
}


