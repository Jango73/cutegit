import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

//-------------------------------------------------------------------------------------------------

// A generic scrollable strip. It is completely agnostic of its children: it
// only shows scroll buttons when the content does not fit, and scrolls the
// content by a fixed step. It supports both horizontal and vertical layouts
// through the vertical/orientation property.
Item {
    id: root

    // Orientation of the strip. Qt.Horizontal (default) lays children out left
    // to right; Qt.Vertical lays them top to bottom.
    property int orientation: Qt.Horizontal

    // Convenience flag kept in sync with orientation.
    property bool vertical: false

    onOrientationChanged: root.vertical = (root.orientation === Qt.Vertical)
    onVerticalChanged: {
        root.orientation = root.vertical ? Qt.Vertical : Qt.Horizontal
        outerLayout.forceLayout()
        contentGrid.forceLayout()
    }

    // Current index, kept for callers that use the strip as a tab bar.
    property int currentIndex: 0

    // Amount scrolled by each button press.
    property int scrollStep: 120

    // Children are placed inside the scrolling content grid.
    default property alias contentChildren: contentGrid.children

    implicitHeight: root.vertical
        ? contentGrid.implicitHeight
        : Math.max(contentGrid.implicitHeight, firstButton.implicitHeight, secondButton.implicitHeight)
    implicitWidth: root.vertical
        ? Math.max(contentGrid.implicitWidth, firstButton.implicitWidth, secondButton.implicitWidth)
        : contentGrid.implicitWidth

    //-------------------------------------------------------------------------------------------------

    readonly property bool overflow: root.vertical
        ? (contentGrid.implicitHeight > contentFlick.height + 1)
        : (contentGrid.implicitWidth > contentFlick.width + 1)

    //-------------------------------------------------------------------------------------------------

    GridLayout {
        id: outerLayout
        anchors.fill: parent
        rows: root.vertical ? 3 : 1
        columns: root.vertical ? 1 : 3
        flow: root.vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
        rowSpacing: 0
        columnSpacing: 0

        ToolButton {
            id: firstButton
            visible: root.overflow
            Layout.preferredWidth: visible ? implicitWidth : 0
            Layout.preferredHeight: visible ? implicitHeight : 0
            text: root.vertical ? "^" : "<"

            onClicked: {
                if (root.vertical)
                    contentFlick.contentY = Math.max(0, contentFlick.contentY - root.scrollStep)
                else
                    contentFlick.contentX = Math.max(0, contentFlick.contentX - root.scrollStep)
            }
        }

        Flickable {
            id: contentFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: root.vertical ? width : contentGrid.implicitWidth
            contentHeight: root.vertical ? contentGrid.implicitHeight : height
            clip: true
            interactive: false
            boundsBehavior: Flickable.StopAtBounds

            GridLayout {
                id: contentGrid
                rows: root.vertical ? -1 : 1
                columns: root.vertical ? 1 : -1
                flow: root.vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
                rowSpacing: 0
                columnSpacing: 0
            }
        }

        ToolButton {
            id: secondButton
            visible: root.overflow
            Layout.preferredWidth: visible ? implicitWidth : 0
            Layout.preferredHeight: visible ? implicitHeight : 0
            text: root.vertical ? "v" : ">"

            onClicked: {
                if (root.vertical) {
                    var maximumY = Math.max(0, contentFlick.contentHeight - contentFlick.height)
                    contentFlick.contentY = Math.min(maximumY, contentFlick.contentY + root.scrollStep)
                } else {
                    var maximumX = Math.max(0, contentFlick.contentWidth - contentFlick.width)
                    contentFlick.contentX = Math.min(maximumX, contentFlick.contentX + root.scrollStep)
                }
            }
        }
    }
}