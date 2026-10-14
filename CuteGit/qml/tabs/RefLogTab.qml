import QtQuick 2.12
import "../views"

BaseTab {
    id: root

    LogView {
        id: refLogView
        anchors.fill: parent

        model: root.repository !== null ? root.repository.refLogModel : undefined
    }

    function focusTab() {
        refLogView.forceActiveFocus()
    }
}
