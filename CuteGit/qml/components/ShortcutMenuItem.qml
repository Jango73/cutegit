import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.3

MenuItem {
    id: root

    // The shortcut is read from the assigned action. The Material style does
    // not display action shortcuts by itself, hence this custom content item.
    // The mnemonic is underlined with styled text so Qt uses the underline
    // metrics of the font really used for the glyph (including the fallback
    // font for CJK glyphs) instead of a fixed offset below the Latin baseline.
    function escapeEntryHtml(value) {
        if (value === "&") {
            return "&amp;"
        }
        if (value === "<") {
            return "&lt;"
        }
        if (value === ">") {
            return "&gt;"
        }
        return value
    }

    function richEntryText(source) {
        var result = ""
        var index = 0
        while (index < source.length) {
            if (source[index] === "&" && index + 1 < source.length) {
                index++
                if (source[index] === "&") {
                    result += "&amp;"
                } else {
                    result += "<u>" + escapeEntryHtml(source[index]) + "</u>"
                }
            } else {
                result += escapeEntryHtml(source[index])
            }
            index++
        }
        return result
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
                textFormat: Text.StyledText
                text: root.richEntryText(root.text)
                font: root.font
                color: root.enabled ? Material.foreground : Material.hintTextColor
                elide: Text.ElideRight
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
