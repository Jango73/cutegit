import QtQuick 2.12
import QtQuick.Controls 2.5
import CuteGit 1.0
import "../components"

Menu {
    id: root
    title: ""

    property bool canRebaseOn: false    // Set by owner of this component
    property bool canMerge: false       // Set by owner of this component
    property variant repository: null   // Set by owner of this component

    signal requestSwitchToBranch()
    signal requestRebaseOnBranch()
    signal requestMergeBranch()
    signal requestDeleteBranch()

    Action {
        text: Const.switchToText
        enabled: root.repository ? root.repository.can(CEnums.SwitchBranch) : false

        onTriggered: {
            root.requestSwitchToBranch()
        }
    }

    Action {
        enabled: root.canRebaseOn && (root.repository ? root.repository.can(CEnums.RebaseOnBranch) : false)
        text: Const.rebaseOnText

        onTriggered: {
            root.requestRebaseOnBranch()
        }
    }

    Action {
        enabled: root.canMerge && (root.repository ? root.repository.can(CEnums.MergeBranch) : false)
        text: Const.mergeText

        onTriggered: {
            root.requestMergeBranch()
        }
    }

    Action {
        text: Const.deleteText
        enabled: root.repository ? root.repository.can(CEnums.DeleteBranch) : false

        onTriggered: {
            root.requestDeleteBranch()
        }
    }
}
