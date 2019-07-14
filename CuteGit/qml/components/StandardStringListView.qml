import QtQuick 2.12
import QtQuick.Controls 2.5
import "."

StandardListView {
    id: root

    delegate: StandardText {
        width: parent.width
        height: Const.elementHeight
        text: display
        elide: Text.ElideRight
    }
}
