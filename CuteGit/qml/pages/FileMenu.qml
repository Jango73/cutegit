import QtQuick 2.12
import QtQuick.Controls 2.5
import CuteGit 1.0
import "../components"

Menu {
    id: root
    title: ""

    property variant fileNames: null
    property variant repository: null

    signal requestDelete(var fileNames)
    signal requestBlame(var fileNames)
    signal requestOpenSubmodule(var fileNames)

    MenuItem {
        text: Const.openSubmoduleMenuText
        visible: root.repository !== null
                 && root.fileNames !== null
                 && root.fileNames.length === 1
                 && root.repository.isSubmodule(root.fileNames[0])

        onTriggered: {
            root.requestOpenSubmodule(root.fileNames)
        }
    }

    Action {
        text: Const.fileDeleteMenuText

        onTriggered: {
            if (root.name !== "") {
                root.requestDelete(root.fileNames)
            }
        }
    }

    Action {
        text: Const.fileBlameMenuText

        onTriggered: {
            if (root.name !== "") {
                root.requestBlame(root.fileNames)
            }
        }
    }
}
