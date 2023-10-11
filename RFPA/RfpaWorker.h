/** Copyright (C) Fuqing Medical and USTC BMEC RFLab - All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited.
 ** Proprietary and confidential.
 ** Created on 202308014, by yue.wang.
 ** brief:  This class is used for tcp socket communication with Sar monitoring box
 **/
#ifndef RFPAWORKER_H
#define RFPAWORKER_H

#include <QObject>
#include <QJsonObject>
#include "ClientWorker.h"

class RfpaWorker : public ClientWorker
{
    Q_OBJECT
    /// Ten second instantaneous SAR.
    LAB_PROP_RS(sarInTenSecs, float, SARInTenSecs)

    /// Six minute SAR
    LAB_PROP_RS(sarInSixMins, float, SARInSixMins)

    /// Control whether to enter controlled mode
    LAB_PROP_RS(restrictedMode, int, RestrictedMode)

    /// Limits for primary controlled mode
    LAB_PROP_RS(firstLevel, int, FirstLevel)

    /// Limits for secondary controlled mode
    LAB_PROP_RS(secondLevel, int, SecondLevel)

    /// Current sanning patient's bodypart.
    /// It is useful when considering partial SAR.
    LAB_PROP_RS(bodyPart, QString, BodyPart)

    /// Effective energy duty cycle of the sequence currently being scanned
    /// Change this when the sequence being scanned changes.
    /// \note Or, you can change ir by listening trough files
    LAB_PROP_RS(effDC, double, EffDC) // in microseconds

public:
    enum SarType{
        Undefined = 0,
        SarIntenSecs,
        SarInsixMins
    };
    enum Mode{
        Normal = 0,
        Level1Restricted,
        Level2Restricted
    };
    enum VoltPeakType{
        Native = 0,
        Incident,
        Reflection
    };

    explicit RfpaWorker(QObject *parent = nullptr);

    /// Get all status data pack in json, this method should be able to call from another thread.
    QJsonObject getStatus();

Q_SIGNALS:
    // TODO ?
    /// append sar json in worker thread.
    /// \note currently use this method to add new data in worker thread.
    void nextPendingSAR(QJsonObject sarJ);

    LAB_PROP_SIGNAL(enabled, bool)
    LAB_PROP_SIGNAL(connected, bool)
    LAB_PROP_SIGNAL(lastError, QString)
    LAB_PROP_SIGNAL(sarInTenSecs, float)
    LAB_PROP_SIGNAL(sarInSixMins, float)
    LAB_PROP_SIGNAL(restrictedMode, int)
    LAB_PROP_SIGNAL(firstLevel, int)
    LAB_PROP_SIGNAL(secondLevel, int)
    LAB_PROP_SIGNAL(bodyPart, QString);
    LAB_PROP_SIGNAL(effDC, double);

protected:
    /// Send instruction data
    void sendAction(const QByteArray &data);

    /// Parse m_readData
    void parseBuffer() override;

    /// Calc sarInTenSecs and sarInSixMins
    ///  \warning only thread internal calls, a process of continuous change
    void recalculateSAR();

    double calcPowerWithInterval(SarType type);

    double calcPowerWithInterval(quint16 interval, bool forward = true,
                                 bool remove = false);

    /// Calc equivalent power
    double calcEquivPower(double vpp, VoltPeakType voltType);

    /// Judge whether the SAR is in the normal range
    void focusOnMode();

protected:
    /// last sarJson added time.
    QDateTime m_lastAppendTime;

    /// all sar info in this thread.
    QJsonObject m_sarJson;

    /// current power list
    QMap<QString, double> m_powerList;

    /// current patient's weight
    float m_currentWeight;

protected Q_SLOTS:
    /// when socket is ready to read
    void slotReadyRead() override;

public Q_SLOTS:
    void init() override;

    /// read config file and connect to host
    void open(const bool closeAndOpen = true) override;

    /// send any string ,just for test
    void sendString(const QString& data);

    /// reset RF Module
    void resetRFMod();

    /// reset gradient module
    void resetGradMod();

    /// get current all sar json
    QJsonObject getAllSarJson();

    /// remove unnecessary sar info
    void removeSarJson(const QStringList keys);

    /// clear all sar json
    void clearAllSarJson();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastCmdTime;
};

#endif // RFPAWORKER_H
