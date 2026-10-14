import QtQuick 2.12
import "../views"

Item {
    id: root

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null

    LogView {
        id: refLogView
        anchors.fill: parent

        model: root.repository !== null ? root.repository.refLogModel : undefined
    }

    function focusTab() {
        refLogView.forceActiveFocus()
    }
}
