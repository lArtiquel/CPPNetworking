import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.3
import com.arti_tsv.classes.background 1.0
import "clientFunc.js" as ClientFunctional

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("Mail Client")

    Background {                // my c++ background object
        id:background
    }


    ListModel {
        id: listModel
    }

    Component {                 // list delegate describes how each element looks
        id: listDelegate

        Item {
            width: parent.width
            height: 40

            Column {
                Text { text: msg; font.pixelSize: 24 }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: listView.currentIndex = index
            }
        }
    }

    ListView {
        id: listView
        width: parent.width/5*4
        height: parent.height
        clip: true
        model: listModel
        delegate: listDelegate
        header: bannercomponent
        footer: Rectangle {
            width: parent.width; height: 30;
            gradient: clubcolors
        }
        highlight: Rectangle {
            width: parent.width
            color: "lightgray"
        }
    }

    Component {     //instantiated when header is processed
        id: bannercomponent
        Rectangle {
            id: banner
            width: parent.width; height: 50
            gradient: clubcolors
            border {color: "#9EDDF2"; width: 2}
            Text {
                anchors.centerIn: parent
                text: "Messages"
                font.pixelSize: 32
            }
        }
    }

    Gradient {
        id: clubcolors
        GradientStop { position: 0.0; color: "#8EE2FE"}
        GradientStop { position: 0.66; color: "#7ED2EE"}
    }

    ColumnLayout {          // buttons layout
        id: buttonsLayout
        anchors.top: parent.top
        anchors.left: listView.right
        anchors.margins: 10
        width: parent.width/5
        height: parent.height

        Button {
            id: signIn
            text: "Sign In"
            enabled: true
            property string login: ""
            property string password: ""

            onClicked: {
                // ask login & password
                lpDialog.open()
            }
        }

        Button {
            id: incomingMsg
            text: "Check messages"
            enabled: false;

            onClicked: {
                ClientFunctional.checkForMessages()
            }
        }

        Button {
            id: deleteMsg
            text: "Delete messages"
            enabled: false;

            onClicked: {
                if(listView.currentIndex !== -1)
                {
                    if(background.deleteIncMessage(listView.currentIndex))
                    {
                        console.log("deleted msg num:"+listView.currentIndex)
                        //now pull from server new list of messages
                        ClientFunctional.checkForMessages()
                    } else
                    {
                        console.log("we are disconnected from server!")
                        listModel.clear()
                        signIn.enabled = true
                        incomingMsg.enabled = false
                        deleteMsg.enabled = false
                        signOut.enabled = false
                    }
                } else
                    console.log("message not choosen!")
            }
        }

        Button {
            id: signOut
            text: "Sign out"
            enabled: false;
            onClicked: {
                if(background.signOut())
                {
                    console.log("successfully signouted!")
                    listModel.clear()
                    signIn.enabled = true
                    incomingMsg.enabled = false
                    deleteMsg.enabled = false
                    signOut.enabled = false
                } else
                {
                    console.log("we are disconnected with error from server!")
                    listModel.clear()
                    signIn.enabled = true
                    incomingMsg.enabled = false
                    deleteMsg.enabled = false
                    signOut.enabled = false
                }
            }
        }
    }


    Dialog {                            //login & password dialog
        id: lpDialog
        visible: false
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
            signIn.login = loginEdit.getText(0, loginEdit.length)
            signIn.password = passwordEdit.getText(0, passwordEdit.length)

            if(background.connectToServer(signIn.login, signIn.password))    // use c++ background to connect to server
            {
                // unlock/lock buttons
                signIn.enabled = false;
                incomingMsg.enabled = true;
                deleteMsg.enabled = true;
                signOut.enabled = true;
                // get incoming messages
                ClientFunctional.checkForMessages()
            }
            this.close()
        }

        onRejected: {
            this.close()
        }
    }
}








