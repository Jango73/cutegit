import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import "../components"

StandardPopup {
    id: root
    width: parent.width * Const.popupWidthNorm
    title: Const.settingsTitleText

    property variant controller: null
    property int materialTheme: Material.Dark

    signal requestDarkTheme()
    signal requestLightTheme()
    signal requestLanguage(var language)

    controls: [
        RowLayout {
            width: parent.width * Const.settingsControlsWidthFraction
            anchors.horizontalCenter: parent.horizontalCenter

            StandardText {
                Layout.fillWidth: true
                text: Const.themeText
            }

            ComboBox {
                id: themeCombo
                model: [Const.darkThemeMenuText, Const.lightThemeMenuText]
            }
        },

        RowLayout {
            width: parent.width * Const.settingsControlsWidthFraction
            anchors.horizontalCenter: parent.horizontalCenter

            StandardText {
                Layout.fillWidth: true
                text: Const.languageText
            }

            ComboBox {
                id: languageCombo
                textRole: "display"
                model: root.controller !== null ? root.controller.langModel : null
            }
        },

        RowLayout {
            width: parent.width * Const.settingsControlsWidthFraction
            anchors.horizontalCenter: parent.horizontalCenter

            StandardText {
                Layout.fillWidth: true
                text: Const.layoutText
            }

            ComboBox {
                id: layoutCombo
                model: root.controller !== null ? root.controller.layoutNames : null
                displayText: root.layoutDisplayName(currentText)
                delegate: ItemDelegate {
                    width: layoutCombo.popup.width
                    text: root.layoutDisplayName(modelData)
                    highlighted: layoutCombo.highlightedIndex === index
                }
            }

            StandardButton {
                text: Const.reloadLayoutsMenuText
                onClicked: {
                    if (root.controller !== null)
                        root.controller.reloadLayouts()
                }
            }
        },

        RowLayout {
            width: parent.width * Const.settingsControlsWidthFraction
            anchors.horizontalCenter: parent.horizontalCenter

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
                    if (root.controller !== null) {
                        root.controller.maxLoadedRepositories = maxLoadedRepositories.value

                        if (themeCombo.currentIndex === 0)
                            root.requestDarkTheme()
                        else
                            root.requestLightTheme()

                        if (languageCombo.currentIndex >= 0) {
                            var sLanguage = languageCombo.currentText.split(" ")[0]
                            root.requestLanguage(sLanguage)
                        }

                        if (layoutCombo.currentIndex >= 0)
                            root.controller.currentLayoutIndex = layoutCombo.currentIndex
                    }
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
        if (root.controller !== null) {
            maxLoadedRepositories.value = root.controller.maxLoadedRepositories
            themeCombo.currentIndex = root.materialTheme === Material.Light ? 1 : 0
            languageCombo.currentIndex = languageIndexForCode(root.controller.language)
            layoutCombo.currentIndex = root.controller.currentLayoutIndex
        }
    }

    function languageIndexForCode(sCode) {
        if (root.controller === null || root.controller.langModel === null)
            return -1
        var lLanguages = root.controller.langModel.stringList
        for (var iIndex = 0; iIndex < lLanguages.length; iIndex++) {
            if (lLanguages[iIndex].split(" ")[0] === sCode)
                return iIndex
        }
        return 0
    }

    function layoutDisplayName(sLayoutName) {
        if (sLayoutName === "Classic")
            return Const.classicLayoutText
        if (sLayoutName === "BranchesLeft")
            return Const.branchesLeftLayoutText
        if (sLayoutName === "Fallback")
            return Const.fallbackLayoutText
        return sLayoutName
    }
}
