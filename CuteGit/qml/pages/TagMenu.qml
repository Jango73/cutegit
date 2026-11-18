import QtQuick 2.12
import QtQuick.Controls 2.5
import CuteGit 1.0
import "../components"

Menu {
    id: root
    title: ""

    property variant repository: null

    signal requestPushTag()
    signal requestDeleteTag()

    Action {
        text: Const.pushText
        enabled: root.repository ? root.repository.can(CEnums.Push) : false

        onTriggered: {
            root.requestPushTag()
        }
    }

    Action {
        text: Const.deleteText
        enabled: root.repository ? root.repository.can(CEnums.DeleteTag) : false

        onTriggered: {
            root.requestDeleteTag()
        }
    }
}
