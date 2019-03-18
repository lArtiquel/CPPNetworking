import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.3

Dialog {                            //login & password dialog
    id: lpDialog
    width: parent.width
    height: parent.height
    visible: true
    modality: Qt.ApplicationModal

    title: "Enter login & password"
    standardButtons: StandardButton.Ok | StandardButton.Cancel

    RowLayout{
        anchors.centerIn: parent
        ColumnLayout{
            Label {
                id: loginLabel
                width: lpDialog.width/5
                font.family: "Helvetica"
                font.pointSize: 20
                text: "Login"
            }

            Label {
                id: passwordLabel
                width: lpDialog.width/5
                font.family: "Helvetica"
                font.pointSize: 20
                text: "Password"
            }
        }

        ColumnLayout{
            TextEdit {
                id: loginEdit
                width: lpDialog.width/5*4
                text: "Login"
                font.family: "Helvetica"
                font.pointSize: 20
                color: "grey"
                focus: true
            }

            TextEdit {
                id: passwordEdit
                width: lpDialog.width/5*4
                text: "Password"
                font.family: "Helvetica"
                font.pointSize: 20
                color: "grey"
                focus: false
            }
        }
    }

    onAccepted: {
        signIn.login = loginEdit.getText()
        signIn.password = password.getText()
        this.close()
    }

    onRejected: {
        this.close()
    }
}
