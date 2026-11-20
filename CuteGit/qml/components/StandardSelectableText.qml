import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

TextEdit {
    readOnly: true
    selectByMouse: true

    font.family: Const.mainFontFamily
    font.pixelSize: Const.mainFontSize
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignTop
    wrapMode: TextEdit.WordWrap
    color: Material.foreground
}
