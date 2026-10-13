import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import ".."

Item {
    id: root
    height: Const.listViewItemHeight * 2

    state: expanded
           ? "expanded"
           : root.secondaryText === ""
             ? "retractedPrimaryOnly"
             : "retracted"

    /*! The list view owning this item */
    property Item listView: null

    /*! If \c true, the item is in expanded state */
    property bool expanded: false

    /*! If \c true, the selection indicator fills the item, otherwise it fills the primary text */
    property bool selectionFillsItem: true
    property bool dataLoading: false
    property variant textElide: Text.ElideRight

    property alias background: background.children
    property alias symbolText: symbolText.text
    property alias primaryText: primaryText.text
    property alias secondaryText: secondaryText.text
    property alias primaryColor: primaryText.color
    property alias primaryFont: primaryText.font
    property alias primaryZone: primaryZoneContent.children
    property alias secondaryZone: secondaryZoneContent.children
    property alias selectionShown: selection.show
    property alias focusShown: focusIndicator.visible
    property alias mouseEnabled: mouse.enabled

    /*! If true, a drawn graph gutter is shown on the left of the item */
    property bool graphVisible: false
    property int graphCommitLane: -1
    property var graphVerticals: []
    property var graphConnections: []

    signal clicked(var mouse, var previousIndex)
    signal doubleClicked(var mouse, var previousIndex)
    signal pressed(var mouse)
    signal entered(var mouse)
    signal exited(var mouse)

    Item {
        anchors.fill: parent
        anchors.leftMargin: Const.verySmallPadding
        anchors.rightMargin: Const.verySmallPadding

        MouseArea {
            id: mouse
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            hoverEnabled: true

            onClicked: function(mouse) {
                if (root.listView) {
                    var previousIndex = root.listView.currentIndex
                    root.listView.changeCurrentIndex(index)
                    root.listView.forceActiveFocus()
                    root.clicked(mouse, previousIndex)
                }
            }

            onDoubleClicked: function(mouse) {
                if (root.listView) {
                    var previousIndex = root.listView.currentIndex
                    root.listView.changeCurrentIndex(index)
                    root.listView.forceActiveFocus()
                    root.doubleClicked(mouse, previousIndex)
                }
            }

            onPressed: function(mouse) { root.pressed(mouse) }
            onEntered: function() { root.entered(mouse) }
            onExited: function() { root.exited(mouse) }
        }

        Item {
            id: background
            anchors.fill: parent
        }

        Selection {
            id: selection
            anchors.fill: selectionFillsItem ? parent : primaryZone
        }

        FocusIndicator {
            id: focusIndicator
            anchors.fill: selection
            visible: root.listView && root.listView.activeFocus && index === root.listView.currentIndex
        }

        Item {
            id: graphZone
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: root.graphVisible ? graphGutter.implicitWidth : 0

            GraphGutter {
                id: graphGutter
                anchors.fill: parent
                commitLane: root.graphCommitLane
                verticals: root.graphVerticals
                connections: root.graphConnections
            }
        }

        Item {
            id: symbolZone
            anchors.left: graphZone.right
            anchors.top: parent.top
            width: (symbolText.text === "" || symbolText.text === Const.statusClean ) ? 0 : symbolText.width + Const.mainPadding
            height: primaryZone.height

            TextOverSelection {
                id: symbolText
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: root.textElide
                selection: root.selectionFillsItem ? selection : null
            }
        }

        /*! The primary zone contains the upper part of data */
        Item {
            id: primaryZone
            anchors.left: symbolZone.right
            width: parent.width - symbolZone.width - graphZone.width
            height: parent.height * 0.5

            TextOverSelection {
                id: dataLoadingText
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: Const.smallPadding
                anchors.rightMargin: Const.smallPadding
                width: root.dataLoading ? implicitWidth : 0
                verticalAlignment: Text.AlignVCenter
                selection: selection
                elide: Text.ElideRight
                text: Const.loadingText
            }

            TextOverSelection {
                id: primaryText
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: dataLoadingText.right
                anchors.right: parent.right
                verticalAlignment: Text.AlignVCenter
                selection: selection
                elide: root.textElide
            }

            Item {
                id: primaryZoneContent
                anchors.fill: parent
            }
        }

        /*! The secondary zone contains the lower part of data */
        Item {
            id: secondaryZone
            anchors.top: primaryZone.bottom
            anchors.bottom: parent.bottom
            anchors.left: graphZone.right
            anchors.right: parent.right

            TextOverSelection {
                id: secondaryText
                anchors.fill: parent
                anchors.leftMargin: Const.mainPadding
                anchors.rightMargin: Const.mainPadding
                verticalAlignment: Text.AlignVCenter
                elide: root.textElide
                selection: root.selectionFillsItem ? selection : null
                asLabel: true
            }

            Item {
                id: secondaryZoneContent
                anchors.fill: parent
            }
        }
    }

    states: [
        State {
            name: ""
        },

        State {
            name : "retractedPrimaryOnly"
            PropertyChanges {
                target: root
                height: Const.listViewItemHeight
            }
            PropertyChanges {
                target: primaryZone
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: symbolZone.right
                anchors.right: parent.right
            }
            PropertyChanges {
                target: secondaryZone
                visible: false
            }
        },

        State {
            name : "retracted"
            PropertyChanges {
                target: root
                height: Const.listViewItemHeight
            }
            PropertyChanges {
                target: primaryZone
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: symbolZone.right
                anchors.right: undefined
                width: (parent.width - graphZone.width) * 0.5
            }
            PropertyChanges {
                target: secondaryZone
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: primaryZone.right
                anchors.right: parent.right
            }
        },

        State {
            name : "expanded"
            PropertyChanges {
                target: primaryZone
                anchors.top: parent.top
                anchors.bottom: undefined
                anchors.left: symbolZone.right
                anchors.right: parent.right
                height: parent.height * 0.5
            }
            PropertyChanges {
                target: secondaryZone
                anchors.top: primaryZone.bottom
                anchors.bottom: parent.bottom
                anchors.left: graphZone.right
                anchors.right: parent.right
            }
        }
    ]
}
