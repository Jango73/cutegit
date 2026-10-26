import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.3

MenuItem {
    id: root

    // The shortcut is read from the assigned action. The Material style does
    // not display action shortcuts by itself, hence this custom content item.
    // The mnemonic underline is also drawn manually: Qt draws it glued to the
    // glyphs with no offset setting, so it is shifted two pixels below the
    // baseline here.
    function strippedEntryText(source) {
        var result = ""
        for (var i = 0; i < source.length; i++) {
            if (source[i] === "&" && i + 1 < source.length) {
                i++
                result += source[i]
            } else {
                result += source[i]
            }
        }
        return result
    }

    function mnemonicPrefix(source) {
        for (var i = 0; i + 1 < source.length; i++) {
            if (source[i] === "&") {
                if (source[i + 1] === "&") {
                    i++
                } else {
                    return strippedEntryText(source.substring(0, i))
                }
            }
        }
        return ""
    }

    function mnemonicCharacter(source) {
        for (var i = 0; i + 1 < source.length; i++) {
            if (source[i] === "&") {
                if (source[i + 1] === "&") {
                    i++
                } else {
                    return source[i + 1]
                }
            }
        }
        return ""
    }

    contentItem: RowLayout {
        id: layout
        spacing: root.spacing

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: root.checkable && root.indicator ? root.indicator.width : 0
            visible: Layout.preferredWidth > 0
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: entryText.implicitHeight

            Label {
                id: entryText
                anchors.fill: parent
                verticalAlignment: Text.AlignTop
                text: root.strippedEntryText(root.text)
                font: root.font
                color: root.enabled ? Material.foreground : Material.hintTextColor
                elide: Text.ElideRight
            }

            FontMetrics {
                id: entryMetrics
                font: entryText.font
            }

            Rectangle {
                visible: root.mnemonicCharacter(root.text) !== "" && !entryText.truncated
                x: entryMetrics.advanceWidth(root.mnemonicPrefix(root.text))
                y: entryMetrics.ascent + 2
                width: entryMetrics.advanceWidth(root.mnemonicCharacter(root.text))
                height: 1
                color: entryText.color
            }
        }

        Label {
            objectName: "shortcutLabel"
            text: root.action && root.action.shortcut !== undefined ? root.action.shortcut : ""
            font: root.font
            color: Material.hintTextColor
            visible: text !== ""
        }
    }
}
