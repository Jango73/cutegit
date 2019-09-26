import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0
import "../components"

Item {
    id: root

    property variant repository: null

    signal requestTextFilter(var text)

    StandardTextFilter {
        id: filter
        objectName: "filter"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: Const.filterText
        enabled: false

        onFilterTextChanged: {
            root.requestTextFilter(text)
        }
    }

    StandardLabel {
        anchors.fill: list
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: Const.listEmptyText
        visible: root.repository === null | list.count === 0
    }

    StandardListView {
        id: list
        objectName: "list"
        anchors.top: filter.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        itemsSelectable: false
        model: root.repository !== null ? root.repository.fileBlameModel : undefined

        delegate: StandardListViewItem {
            objectName: "list.delegate." + index
            width: parent.width
            listView: parent
            primaryText: model.text
            selectionShown: false
            focusShown: false
        }
    }
}
