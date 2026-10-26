import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

TitlePane {
    id: root

    property variant controller: null

    title: Const.repositoriesText

    content: Item {
        anchors.fill: parent

        StandardLabel {
            id: listPlaceHolder
            anchors.fill: parent
            visible: !list.visible
            text: Const.openRepositoryPlaceholderText
        }

        StandardListView {
            id: list
            anchors.fill: parent
            visible: count > 0

            model: root.controller.repositoryModel

            delegate: Item {
                width: parent.width
                height: Const.listViewItemHeight

                Item {
                    id: item
                    anchors.fill: parent
                    anchors.margins: Const.smallPadding

                    MouseArea {
                        anchors.fill: selection
                        onDoubleClicked: {
                            root.controller.openRepository(display)
                        }
                    }

                    Selection {
                        id: selection
                        anchors.fill: text
                        show: display === root.controller.currentRepository.repositoryPath
                    }

                    TextOverSelection {
                        id: text
                        width: parent.width - Const.smallPadding
                        text: item.getDisplayName()

                        selection: selection
                    }

                    function getDisplayName() {
                        return display.split("/").slice(-1)[0]
                    }
                }
            }
        }
    }
}
