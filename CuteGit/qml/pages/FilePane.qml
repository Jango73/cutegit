import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"
import "../views"
import "../popups"

ExtendablePane {
    id: root
    showTitle: false

    property variant repository: null
    property variant flatSelection: null

    signal requestMenu(var fileNames)
    signal requestDeleteFiles(var fileNames)
    signal requestFileFilter(var text)
    signal requestFileSortField(var field)
    signal requestFileSortDirection(var direction)

    content: [
        StandardSortFilter {
            id: filterContainer
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: Const.microButtonWidth
            filterTipText: Const.tipFileFilter

            fieldModel: ListModel {
                ListElement {
                    text: qsTr("Absolute name")
                    field: CEnums.SortFullName
                }
                ListElement {
                    text: qsTr("File name")
                    field: CEnums.SortFileName
                }
            }

            onRequestFilter: function(text) { root.requestFileFilter(text) }
            onRequestSortField: function(field) { root.requestFileSortField(field) }
            onRequestSortDirection: function(direction) { root.requestFileSortDirection(direction) }
        },

        StandardLabel {
            anchors.fill: flatFileView
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: Const.allFilesCleanText
            visible: root.repository === null | flatFileView.count === 0
        },

        FlatFileView {
            id: flatFileView
            anchors.top: filterContainer.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            repository: root.repository
            selection: root.flatSelection

            onRequestMenu: root.requestMenu(fileNames)
            onRequestDeleteFiles: root.requestDeleteFiles(fileNames)
        }
    ]

    function getSelectedFiles() {
        return flatFileView.getSelectedFiles()
    }

    function activateFlatFileView() {
        flatFileView.forceActiveFocus()
    }
}
