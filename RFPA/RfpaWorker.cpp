/** Copyright (C) Fuqing Medical and USTC BMEC RFLab - All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited.
 ** Proprietary and confidential.
 ** Created on 202308014, by yue.wang.
 **/
#include "RfpaWorker.h"
#include <QDebug>
#include <QDataStream>
#include <QThread>
#include "LabTool.h"
#ifndef  STRING_CMP
#define STRING_CMP(opt, obj) \
opt.compare( obj , Qt::CaseInsensitive) == 0
#endif

const int MAX_SOCKET_READ = 1024 * 3; // maximum number of read bytes at a time
const int DATA_LENGTH = 30; // TODO
const int SEND_INTERVAL = 200; //TODO
std::chrono::time_point lastCmdTime = std::chrono::high_resolution_clock::now();

RfpaWorker::RfpaWorker(QObject *parent) :
    ClientWorker{parent}, m_sarInTenSecs(0), m_sarInSixMins(0), m_currentWeight(50.),
    m_restrictedMode(0), m_firstLevel(0), m_secondLevel(0), m_effDC(592.)
{
    m_lastCmdTime = std::chrono::high_resolution_clock::now();
}

QJsonObject RfpaWorker::getStatus()
{
    QJsonObject data;
    data.insert("connected", getConnected());
    data.insert("enabled", getEnabled());
    data.insert("lastErr", getLastError());
    data.insert("sarInTenSecs", getSARInTenSecs());
    data.insert("sarInSixMins", getSARInSixMins());
    data.insert("restrictedMode", getRestrictedMode());
    data.insert("firstLevel", getFirstLevel());
    data.insert("secondLevel", getSecondLevel());
    return data;
}

void RfpaWorker::sendAction(const QByteArray &data)
{
    if(!getEnabled() || !getConnected()){
        qDebug() << "RFPA is either disabled or disconnected:" << getEnabled() << getConnected();
        return;
    }
    while(SEND_INTERVAL >= (std::chrono::high_resolution_clock::now() - lastCmdTime).count() / 1e6){
    }
    sendData(data);
    m_tcpSocket->waitForBytesWritten();
    lastCmdTime = std::chrono::high_resolution_clock::now();
}

void RfpaWorker::parseBuffer()
{
    qDebug() << "RfpaWorker::parseBuffer():" << m_readData;
    // TODO
    QString bufferData = m_readData.constData();
    //    QRegExp regex("5AA540(.{2})(.{8})");
    QRegExp regex{"\"actionType\":\"PULSE_POWER\",\"incident\":424.792,\"reflection\":414.394}{\"actionType\":\"PULSE_POWER\",\"incident\":429.475,\"reflection\":419.491}"};
    int pos = 0;
    while((pos = regex.indexIn(bufferData, pos)) != -1){
        QString matchedValue = regex.cap(0);
        qDebug() << "matchedValue:" << matchedValue;
        QString actionType = regex.cap(1);
        QString incident = regex.cap(2);
        m_readData.remove(0, pos + regex.matchedLength());  // 从缓存中删除匹配到的数据
        pos += regex.matchedLength();  // 继续从下一个位置开始匹配
    }
}

void RfpaWorker::recalculateSAR()
{
    if(!getEnabled() || m_powerList.isEmpty()) return;
    double tenSecsPower = calcPowerWithInterval(SarIntenSecs);
    double sixMinsPower = calcPowerWithInterval(SarInsixMins);

    double patientWeight = 50.0;
    patientWeight = m_currentWeight;
    if(patientWeight > 0){
        double sarInTenSecs = tenSecsPower / patientWeight;
        double sarInSixMins = sixMinsPower / patientWeight;
        sarInTenSecs = qMax(sarInTenSecs, 0.);
        sarInSixMins = qMax(sarInSixMins, 0.);
        setSARInTenSecs(sarInTenSecs);
        setSARInSixMins(sarInSixMins);
        /// judge whether the SAR is in the normal range
        focusOnMode();
    }
}

double RfpaWorker::calcPowerWithInterval(SarType type)
{
    if(type == SarType::SarIntenSecs){
        return calcPowerWithInterval(10, true, false);
    }else if(type == SarType::SarInsixMins){
        return calcPowerWithInterval(360, false, true);
    }else{
        return 0.;
    }
}

double RfpaWorker::calcPowerWithInterval(quint16 interval, bool forward, bool remove)
{
/*
    // TODO
    if(interval < 1) return 0.;
    QString format = "yyyy-MM-dd hh:mm:ss:zzz";
    QDateTime currentTime = QDateTime::currentDateTime();
    double duration = -interval;
    QDateTime intervalAgoTime = currentTime.addSecs(duration);

    QStringList timeList = m_powerList.keys();
    if(timeList.isEmpty()) return 0.;
    int startIdx = 0;
    int toIdx = timeList.size() - 1;
    QString firstTimeStr = timeList.first();
    QDateTime firstTime = QDateTime::fromString(firstTimeStr, format);
    QString lastTimeStr = timeList.last();
    QDateTime lastTime = QDateTime::fromString(lastTimeStr, format);

    if(intervalAgoTime.msecsTo(lastTime) >= 0 && currentTime.msecsTo(lastTime) <= 0){
        if(intervalAgoTime.msecsTo(firstTime) >=0){
            startIdx = 0;
        }else{
            if(forward){
                for(int i = timeList.size() - 1; i >= 0; i--){
                    if( i <= 0 ) {
                        startIdx = i;
                        break;
                    }
                    QString dateTimeStr1 = timeList.at(i);
                    QString dateTimeStr2 = timeList.at(i-1);
                    QDateTime dateTime1 = QDateTime::fromString(dateTimeStr1, format);
                    QDateTime dateTime2 = QDateTime::fromString(dateTimeStr2, format);
                    if((intervalAgoTime.msecsTo(dateTime1) >= 0 &&
                         intervalAgoTime.msecsTo(dateTime2) < 0)){
                        startIdx = i;
                        break;
                    }else{
                        if( remove ) m_powerList.remove(dateTimeStr1);
                    }
                }
            }else{
                for(int i = 0; i < timeList.size(); i++){
                    if( i == timeList.size() - 1 ){
                        startIdx = i;
                        break;
                    }
                    QString dateTimeStr1 = timeList.at(i).split("_").first();
                    QString dateTimeStr2 = timeList.at(i+1).split("_").first();
                    QDateTime dateTime1 = QDateTime::fromString(dateTimeStr1, format);
                    QDateTime dateTime2 = QDateTime::fromString(dateTimeStr2, format);
                    if(intervalAgoTime.msecsTo(dateTime1) <= 0 &&
                        intervalAgoTime.msecsTo(dateTime2) >0){
                        startIdx = i;
                        break;
                    }else{
                        if( remove ) m_powerList.remove(dateTimeStr1);
                    }
                }
            }
        }
    }else if(intervalAgoTime.msecsTo(lastTime) < 0){
        if( remove ) m_powerList.clear();
        return 0;
    }else{
        /// error message
        qCritical() << "Index error, currentTime is earlier than recording time.";
        return 0;
    }

    double powerU = 0;
    for(int i = startIdx; i < toIdx + 1; ++i){
        if(i >= timeList.size()) break;
        QString key = timeList.at(i);
        if(!m_powerList.contains(key)) continue;
        double power = m_powerList.value(key);
        powerU = powerU + power;
    }
    return powerU / interval;
*/
    return 0.;
}

double RfpaWorker::calcEquivPower(double vpp, VoltPeakType voltType)
{
//    /// \warning Calculating the power at the Detector Output.
//    double Z0 = m_priorJ.value("Z0").toDouble(50);
//    double factor = m_priorJ.value("Factor").toDouble(56);
//    double power = std::pow(vpp / 1e3, 2) / (8 * Z0);
//    if(voltType == INCIDENT){
//        double incidentAtt = m_priorJ.value("IncidentAtt").toDouble(15);
//        factor = factor + incidentAtt;
//    }else if(voltType == REFLECTION){
//        double reflectionAtt = m_priorJ.value("ReflectAtt").toDouble(18);
//        factor = factor + reflectionAtt;
//    }
//    double powerInDBm = 10 * std::log10(power*1e3) + factor;
//    double powerInW = std::pow(10, powerInDBm / 10) / 1e3;
//    return powerInW;
    return 0.;
}

void RfpaWorker::focusOnMode()
{
    float sarInTenSecs = getSARInTenSecs();
    float sarInSixMins = getSARInSixMins();
    double firstLevel = getFirstLevel();
    double secondLevel = getSecondLevel();

    ///  \note currently volumetric SAR limits are used.
    int secsCtl = 0, minsCtl = 0;
    if(sarInTenSecs <= 2 * firstLevel){
        secsCtl = 0;
    }else if((2 * firstLevel < sarInTenSecs) && (sarInTenSecs < 2 * secondLevel)){
        secsCtl = 1;
    }else if(sarInTenSecs >= 2 * secondLevel){
        secsCtl = 2;
    }

    if(sarInSixMins <= firstLevel){
        minsCtl = 0;
    }else if((firstLevel < sarInSixMins) && (sarInSixMins <= secondLevel)){
        minsCtl = 1;
    }else if(sarInSixMins >= secondLevel){
        minsCtl = 2;
    }
    int restrictedMode = std::max(secsCtl, minsCtl);
    setRestrictedMode(restrictedMode);
}

void RfpaWorker::slotReadyRead()
{
    QByteArray data = m_tcpSocket->readAll();
    qDebug() << "new data:" << data;
    qDebug() << "new hex :" << data.toHex();
    m_readData.append(data);
    if(DATA_LENGTH > m_readData.length()) return;
    parseBuffer();
}

void RfpaWorker::init()
{
    ClientWorker::init();
    m_tcpSocket->setReadBufferSize(MAX_SOCKET_READ);
}

void RfpaWorker::open(const bool closeAndOpen)
{
    qDebug() << "RfpaWorker::open:" << closeAndOpen;
    if(m_tcpSocket && m_tcpSocket->isOpen()){
        if(closeAndOpen){
            m_tcpSocket->close();
        }else{
            qWarning()<<"Sar tcp socket is already open, set closeAndOpen to true to close and reopen.";
            return;
        }
    }

    const auto configJ = LabTool::ReadJsonFile("config/exthw.json").value("rfserver").toObject();
    setEnabled(configJ.value("enabled").toBool());
    if(!getEnabled()){
        qDebug() << "RFPA module is disabled or file config/exthw.json is invalid.";
        setConnected(false);
        return;
    }
    const auto host = configJ.value("server").toString("127.0.0.1");
    const auto port = configJ.value("port").toDouble(8899);
    bool hostValid = checkHost(host);
    bool portValid = checkPort(port);
    if(hostValid && portValid){
        m_tcpSocket->connectToHost(host, port);
    }else{
        QString msg = QObject::tr("Invalid server or port, please check it.");
        qWarning() << msg;
        setLastError(msg);
    }
}

void RfpaWorker::sendString(const QString &data)
{
    qDebug() << "RfpaWorker::sendString:" << data;
    QByteArray byteArray;
    int len = data.length();
    for(int i = 0; i < len; i += 2){
        QString hex = data.mid(i, 2);
        bool ok;
        quint8 value = hex.toUInt(&ok, 16);
        if(ok){
            byteArray.append(value);
        }
    }
    sendAction(byteArray);
}

void RfpaWorker::resetRFMod()
{
    // use modbus protocol
    QByteArray data;
    data.append('\x01'); data.append('\x06');
    data.append('\x01'); data.append('\x00');
    data.append('\x00'); data.append('\x01');
    data.append('\x49'); data.append('\xf6');
    sendAction(data);
}

void RfpaWorker::resetGradMod()
{
    /// \todo add this command stream protocol.
    /// reset gradient module, when gradient protection.
}

QJsonObject RfpaWorker::getAllSarJson()
{
    return m_sarJson;
}

void RfpaWorker::removeSarJson(const QStringList keys)
{
    Q_FOREACH(auto k, keys){
        if(m_sarJson.contains(k)){
            m_sarJson.remove(k);
        }
    }
}

void RfpaWorker::clearAllSarJson()
{
    m_sarJson = QJsonObject();
}
