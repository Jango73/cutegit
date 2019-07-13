import QtQuick 2.12
import QtQuick.Controls 1.5
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

Pane {
    id: root
    padding: 2

    Material.elevation: Const.paneElevation

    property variant controller: null
    property variant selection: null

    TreeView {
        id: view
        anchors.fill: parent

        model: root.controller.fileModel
        rootIndex: root.controller.fileModel !== null ? root.controller.fileModel.rootPathIndex : undefined
        selection: root.selection
        selectionMode: 2
        // "None", "Single", "Extended", "Multi", "Contig."

        TableViewColumn {
            title: "Name"
            role: "fileName"
            width: view.width * 0.7
        }

        TableViewColumn {
            title: "Status"
            role: "status"
            width: view.width * 0.2
        }

        TableViewColumn {
            title: "Staged"
            role: "staged"
            width: view.width * 0.1
        }

        style: TreeViewStyle {
            headerDelegate: Item {
                height: Const.mainFontSize * 1.3

                Rectangle {
                    anchors.fill: parent
                    color: Material.background
                    height: Const.mainFontSize * 1.3
                }

                StandardText {
                    text: styleData.value
                }
            }

            rowDelegate: Rectangle {
                color: Material.background
                height: Const.mainFontSize * 1.3
            }

            itemDelegate: Item {
                Selection {
                    id: selection
                    targetWidth: text.width
                    targetHeight: text.height
                    anchors.centerIn: text
                    visible: styleData.selected && styleData.value !== ""
                }

                StandardText {
                    id: text
                    text: styleData.value
                    color: selection.visible ? Material.background : Material.foreground
                }
            }
        }
    }
}
