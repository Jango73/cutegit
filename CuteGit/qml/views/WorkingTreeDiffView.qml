import QtQuick 2.12
import QtQuick.Layouts 1.3
import "../components"

DiffView {
    id: root

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredHeight: stretchWeight

    /*! Stretch weight inside the working tree column */
    property int stretchWeight: Const.stagedDiffStretch

    /*! Forwards right clicks with the staged side and current selection */
    signal requestMenu(var operation, var text, var hunkId, var selectedRows)

    onItemRightClicked: function(operation, text, hunkId) {
        root.requestMenu(operation, text, hunkId, root.selectedRows)
    }
}
