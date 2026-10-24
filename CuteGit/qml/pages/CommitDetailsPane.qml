import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import "../components"

Item {
    id: root

    property variant repository: null

    property string commitId: root.repository ? root.repository.commitDetailsId : ""
    property string commitAuthor: root.repository ? root.repository.commitDetailsAuthor : ""
    property variant commitDate: root.repository ? root.repository.commitDetailsDate : undefined
    property int addedCount: root.repository ? root.repository.commitDetailsAddedCount : 0
    property int removedCount: root.repository ? root.repository.commitDetailsRemovedCount : 0
    property string commitMessage: root.repository ? root.repository.commitDetailsMessage : ""

    StandardLabel {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: Const.listEmptyText
        visible: root.repository === null || root.commitId === ""
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Const.smallPadding
        visible: root.repository !== null && root.commitId !== ""

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Const.mainPadding
            rowSpacing: Const.verySmallPadding

            StandardLabel {
                text: Const.commitIdText
            }
            StandardText {
                Layout.fillWidth: true
                text: root.commitId
                wrapMode: Text.WrapAnywhere
            }

            StandardLabel {
                text: Const.authorText
            }
            StandardText {
                Layout.fillWidth: true
                text: root.commitAuthor
            }

            StandardLabel {
                text: Const.dateText
            }
            StandardText {
                Layout.fillWidth: true
                text: root.formattedDate()
            }

            StandardLabel {
                text: Const.addedLinesText
            }
            StandardText {
                Layout.fillWidth: true
                text: root.addedCount
            }

            StandardLabel {
                text: Const.removedLinesText
            }
            StandardText {
                Layout.fillWidth: true
                text: root.removedCount
            }
        }

        StandardLabel {
            Layout.fillWidth: true
            text: Const.messageText
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: Const.flickDecelerationWithoutInertia
            maximumFlickVelocity: Const.maximumFlickVelocityWithoutInertia
            contentWidth: width
            contentHeight: commitMessageText.height
            ScrollBar.vertical: ScrollBar {
            }

            StandardText {
                id: commitMessageText
                width: parent.width
                text: root.commitMessage
            }
        }
    }

    function isValidDate(dateValue) {
        return dateValue instanceof Date && !isNaN(dateValue)
    }

    function formattedDate() {
        if (isValidDate(root.commitDate))
            return Qt.formatDateTime(root.commitDate, Qt.DefaultLocaleLongDate)
        return ""
    }

    function focusPane() {
        root.forceActiveFocus()
    }
}
