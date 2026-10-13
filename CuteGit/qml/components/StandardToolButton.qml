import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

ToolButton {
    display: AbstractButton.TextBesideIcon
    implicitHeight: Const.toolButtonHeight
    topPadding: 0
    bottomPadding: 0
    topInset: 0
    bottomInset: 0
    icon.color: action.enabled ? Material.foreground : Material.backgroundDimColor
    icon.width: Const.mainFontSize * 1.5
    icon.height: Const.mainFontSize * 1.5
    font.family: Const.mainFontFamily
    font.pixelSize: Const.mainFontSize
}
