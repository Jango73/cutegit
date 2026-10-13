import QtQuick 2.12
import QtQuick.Controls 2.5
import "."

StandardListView {
    delegate: ElideText {
        width: ListView.view.width
        height: Const.listViewItemHeight
        text: display
    }
}
