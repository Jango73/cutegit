import QtQuick 2.12
import QtQuick.Controls 2.5
import CuteGit 1.0
import "../components"

Menu {
    id: root
    title: ""

    property variant repository: null

    signal requestPushTag()

    Action {
        text: Const.pushText
        enabled: root.repository ? root.repository.can(CEnums.Push) : false

        onTriggered: {
            root.requestPushTag()
        }
    }
}
