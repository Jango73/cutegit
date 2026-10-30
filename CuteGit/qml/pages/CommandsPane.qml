import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "../components"

ExtendablePane {
    id: root

    property variant repository: null

    title: Const.commandsText

    content: Item {
        anchors.fill: parent

        StandardToolBar {
            id: buttons
            width: parent.width
            anchors.top: parent.top

            Row {
                spacing: Const.mainPadding

                StandardToolButton {
                    action: Action {
                        id: clearButton
                        text: Const.clearText
                        onTriggered: {
                            root.repository.clearOutput()
                        }
                    }
                    icon.source: Const.deleteIcon
                }
            }
        }

        StandardStringListView {
            id: listView
            anchors.top: buttons.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            autoScrollToEnd: true
            model: root.repository ? root.repository.commandOutputModel : undefined
        }
    }

    onRepositoryChanged: {
        if (repository) {
            repository.commandOutputModelChanged.connect(listView.modelChanged)
        }
    }
}
