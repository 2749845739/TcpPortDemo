#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTime>
#include <QFile>
#include "ClientWorker.h"

void write2File(QString msg) {
    QFile file("RFPower_record.log");
    file.open(QIODevice::Append);
    QTextStream stream(&file);
    stream << msg;
}

void logOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    QDateTime dateTime = QDateTime::currentDateTime();
    QString now = dateTime.toString("yyyy-MM-dd HH:mm:ss");
    QString msgStr;
    switch (type) {
    case QtDebugMsg:
        msgStr = now + "," + "Debug:: " + localMsg + "(" + file + "::line " + QString::number(context.line) + ")" + "\n";
        break;
    case QtCriticalMsg:
        msgStr = now + "," + "Error:: " + localMsg + "(" + file + "::line " + QString::number(context.line) + ")" + "\n";
        break;
    case QtInfoMsg:
        msgStr = now + "," + "info:: " + localMsg + "(" + file + "::line " + QString::number(context.line) + ")" + "\n";
        break;
    default:
        break;
    }
    write2File(msgStr);
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
//    qInstallMessageHandler(logOutput);

    QQmlApplicationEngine engine;
    qmlRegisterType<ClientWorker>("MyLab", 1, 0, "LabTcp");

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
