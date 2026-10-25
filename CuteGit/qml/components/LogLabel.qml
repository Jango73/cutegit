import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import CuteGit 1.0

Item {
    id: root
    width: labelText.width + Const.mainPadding

    property alias text: labelText.text
    property int type: CEnums.UnknownLabel

    Selection {
        anchors.fill: parent
        anchors.topMargin: Const.smallPadding
        anchors.bottomMargin: Const.smallPadding
        color: root.type === CEnums.LocalBranchLabel
               ? Const.labelLocalColor
               : root.type === CEnums.RemoteBranchLabel
                 ? Const.labelRemoteColor
                 : root.type === CEnums.UpToDateLabel
                   ? Const.labelUpToDateColor
                   : root.type === CEnums.UnsyncedLabel
                     ? Const.labelUnsyncedColor
                     : root.type === CEnums.DivergedLabel
                       ? Const.labelDivergedColor
                       : Const.labelTagColor
        show: true
    }

    StandardText {
        id: labelText
        anchors.centerIn: parent
        color: Material.background
    }
}
