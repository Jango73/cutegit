import QtQuick 2.12
import "../components"

BaseTab {
    id: root

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
