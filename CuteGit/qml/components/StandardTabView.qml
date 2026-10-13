import QtQuick 2.12
import QtQuick.Layouts 1.3

StackLayout {
    property Item currentItem: null
    onCurrentIndexChanged: currentItem = itemAt(currentIndex)
    onCountChanged: currentItem = itemAt(currentIndex)
}
