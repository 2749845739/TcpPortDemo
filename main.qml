import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import  MyLab 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    ColumnLayout{
        id: layout
        spacing: 5
        anchors.fill: parent

        RowLayout{
            spacing: 5
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            Rectangle{
                width: 50
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Host:")
                    font.pixelSize: 20
                }
            }
            TextField {
                id: hostText
                placeholderText: qsTr("Host value")
                Layout.preferredWidth: 100
                font.pixelSize: 14
            }

            Item {
                width: 30
            }
            Rectangle{
                width: 50
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Port:")
                    font.pixelSize: 20
                }
            }
            TextField {
                id: portText
                placeholderText: qsTr("Port value")
                Layout.preferredWidth: 100
                font.pixelSize: 14
            }

            Item {
                width: 30
            }
            Button{
                text: "Connect"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        tcpWorker.connectToServer(hostText.text, portText.text)
                    }
                }
            }
        }

        RowLayout{
            spacing: 20
            Layout.alignment: Qt.AlignHCenter
            Button{
                text: "resetRFMod"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        tcpWorker.resetRFMod()
                    }
                }
            }

            Button{
                text: "getPeek"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        tcpWorker.getPeek()
                    }
                }
            }
        }

        RowLayout{
            spacing: 20
            Layout.alignment: Qt.AlignHCenter
            Rectangle{
                width: 50
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Content:")
                    font.pixelSize: 20
                }
            }
            TextField {
                id: contentText
                placeholderText: qsTr("Content value")
                Layout.preferredWidth: 200
                font.pixelSize: 14
            }

            Item {
                width: 30
            }
            Button{
                text: "sendAny"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        tcpWorker.sendString(contentText.text)
                    }
                }
            }
        }

    }
    LabTcp{
        id: tcpWorker
    }
}
