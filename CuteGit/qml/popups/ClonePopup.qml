import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import "../components"

StandardPopup {
    id: root
    width: parent.width * Const.popupWidthNorm
    title: Const.cloneRepositoryText

    property variant controller: null

    signal cloneBegins()

    controls: [
        TitleTextField {
            id: repositoryURL
            width: parent.width
            height: implicitHeight

            title: Const.repositoryURLText
        },

        FolderSelector {
            id: repositoryPath
            width: parent.width
            height: implicitHeight
            controller: root.controller

            title: Const.destinationFolderText
        }
    ]

    buttons: [
        StandardToolButton {
            action: Action {
                id: okButton
                text: Const.okText
                onTriggered: {
                    root.cloneBegins()
                    root.controller.cloneRepository(repositoryURL.text, repositoryPath.text)
                    root.close()
                }
            }
        },

        StandardToolButton {
            action: Action {
                id: cancelButton
                text: Const.cancelText
                onTriggered: {
                    root.close()
                }
            }
        }
    ]

    onOpened: {
        repositoryURL.text = root.controller.lastBrowsedRepositoryURL
        repositoryPath.text = root.controller.lastBrowsedRepositoryPath
    }

    onClosed: {
        root.controller.lastBrowsedRepositoryURL = repositoryURL.text
        root.controller.lastBrowsedRepositoryPath = repositoryPath.text
    }
}
