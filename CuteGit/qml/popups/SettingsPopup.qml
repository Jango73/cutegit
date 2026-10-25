import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import "../components"

StandardPopup {
    id: root
    width: parent.width * Const.popupWidthNorm
    title: Const.settingsTitleText

    property variant controller: null

    controls: [
        RowLayout {
            width: parent.width

            StandardText {
                Layout.fillWidth: true
                text: Const.maxLoadedRepositoriesText
            }

            SpinBox {
                id: maxLoadedRepositories
                from: 1
                to: 99
                editable: true
            }
        }
    ]

    buttons: [
        StandardToolButton {
            action: Action {
                text: Const.okText
                onTriggered: {
                    if (root.controller !== null)
                        root.controller.maxLoadedRepositories = maxLoadedRepositories.value
                    root.close()
                }
            }
        },

        StandardToolButton {
            action: Action {
                text: Const.cancelText
                onTriggered: {
                    root.close()
                }
            }
        }
    ]

    onOpened: {
        if (root.controller !== null)
            maxLoadedRepositories.value = root.controller.maxLoadedRepositories
    }
}
