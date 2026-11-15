import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

Item {
    id: root
    clip: true
    property string tipText: ""
    property alias text: edit.text
    property alias placeHolderText: placeholder.text
    property alias horizontalAlignment: edit.horizontalAlignment
    property alias verticalAlignment: edit.verticalAlignment

    Rectangle {
        id: background
        anchors.fill: parent
        color: Const.transparent
        border.width: 1
        border.color: Material.accent
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: edit.paintedHeight + Const.mainPadding * 2
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: Const.flickDecelerationWithoutInertia
        maximumFlickVelocity: Const.maximumFlickVelocityWithoutInertia
        ScrollBar.vertical: ScrollBar {
        }

        TextEdit {
            id: edit
            x: Const.mainPadding
            y: Const.mainPadding
            width: flick.width - Const.mainPadding * 2
            height: Math.max(flick.height - Const.mainPadding * 2, paintedHeight)
            font.family: Const.mainFontFamily
            font.pixelSize: Const.mainFontSize
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: TextEdit.WordWrap
            color: Material.foreground
            selectByMouse: true
            focus: true
            onCursorRectangleChanged: root.ensureCursorVisible()
        }
    }

    StandardText {
        id: placeholder
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        visible: edit.text === ""
        color: Material.backgroundDimColor
    }

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Keeps the text cursor inside the visible area of the flickable content
    function ensureCursorVisible() {
        var cursorTop = edit.y + edit.cursorRectangle.y
        var cursorBottom = cursorTop + edit.cursorRectangle.height

        if (cursorTop < flick.contentY)
            flick.contentY = cursorTop
        else if (cursorBottom > flick.contentY + flick.height)
            flick.contentY = cursorBottom - flick.height
    }
}
