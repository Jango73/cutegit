import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import "../components"

StandardPopup {
    id: root
    width: parent.width * Const.popupWidthNorm
    title: Const.helpTitleText

    property variant controller: null

    controls: [
        StandardText {
            id: copyrightText
            width: parent.width
            height: parent.height * 0.15
        },

        TabBar {
            id: tabBar
            width: parent.width
            height: implicitHeight

            StandardTabButton {
                width: implicitWidth
                text: Const.generalText
            }

            StandardTabButton {
                width: implicitWidth
                text: Const.shortcutsText
            }
        },

        StandardTabView {
            id: swipeView
            width: parent.width
            height: Const.elementHeight * 20
            clip: true
            currentIndex: tabBar.currentIndex

            Flickable {
                id: helpView
                contentHeight: generalText.height

                StandardText {
                    id: generalText
                    padding: Const.mainPadding * 2
                }
            }

            Item {
                id: shortcutView

                StandardText {
                    id: shortcutText
                    anchors.top: parent.top
                    padding: Const.mainPadding * 2
                }

                StandardListView {
                    id: shortcutListView
                    anchors.top: shortcutText.bottom
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right

                    model: Const.shortcutModel

                    delegate: Item {
                        width: parent.width
                        height: shortcutName.height

                        StandardText {
                            id: shortcutName
                            width: parent.width * 0.6
                            text: name
                        }

                        StandardText {
                            anchors.left: shortcutName.right
                            width: parent.width * 0.4
                            text: sequence
                        }
                    }
                }
            }
        }
    ]

    buttons: [
        StandardToolButton {
            action: Action {
                id: closeButton
                text: Const.closeText
                onTriggered: {
                    root.close()
                }
            }
        }
    ]

    Component.onCompleted: {
        copyrightText.text = Const.copyrightText.arg(root.controller.version)
        generalText.text = Const.generalHelpText
        shortcutText.text = Const.shortcutsText
    }
}
