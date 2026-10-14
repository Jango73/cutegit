import QtQuick 2.12
import "../components"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

    StandardListView {
        id: tagList
        anchors.fill: parent
        visible: count > 0

        model: root.repository ? root.repository.tagModel : undefined

        delegate: StandardListViewItem {
            width: ListView.view.width
            listView: tagList
            selectionShown: false
            primaryText: model.name
        }
    }

    function focusTab() {
        tagList.forceActiveFocus()
    }
}
