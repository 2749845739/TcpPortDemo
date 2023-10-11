/** Copyright (C) Fuqing Medical and USTC BMEC RFLab - All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited.
 ** Proprietary and confidential.
 ** Created on 20230807, by yue.wang.
 ** brief:  This class is used for tcp communication with the RF Power Amplifier
 **/

#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

#include <QObject>
#include <QTcpSocket>

class ClientWorker : public QObject
{
    Q_OBJECT
public:
    explicit ClientWorker(QObject* parent = nullptr);

    Q_INVOKABLE bool connectToServer(const QString& host, quint16 port);

    /// send any string ,just for test
    Q_INVOKABLE void sendString(const QString& data);

    /// reset RF Module
    Q_INVOKABLE void resetRFMod();

    /// get the peak values of travelling wave and reflecting wave
    Q_INVOKABLE void getPeek();

    Q_INVOKABLE void clearCache();

    Q_INVOKABLE void disconnectFromServer();

private:
    bool sendData(const QByteArray& data);
    void parseBuffer();


signals:

public slots:

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError error);
    void slotReadyRead();
    void slotBytesWritten(qint64 bytes);

private:
    QTcpSocket *m_socket;
    QByteArray m_writeData;
    QByteArray m_readData;
    qint64 m_bytesWritten;
};

#endif // CLIENTWORKER_H
