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
            text: Const.copyrightText.arg(root.controller ? root.controller.version : "")
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
                boundsBehavior: Flickable.StopAtBounds
                flickDeceleration: Const.flickDecelerationWithoutInertia
                maximumFlickVelocity: Const.maximumFlickVelocityWithoutInertia

                StandardText {
                    id: generalText
                    padding: Const.mainPadding * 2
                    text: Const.generalHelpText
                }
            }

            Item {
                id: shortcutView

                StandardText {
                    id: shortcutText
                    anchors.top: parent.top
                    padding: Const.mainPadding * 2
                    text: Const.shortcutsText
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
                            text: modelData.name
                        }

                        StandardText {
                            anchors.left: shortcutName.right
                            width: parent.width * 0.4
                            text: modelData.sequence
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
}
