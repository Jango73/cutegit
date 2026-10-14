import QtQuick 2.12
import QtQuick.Controls 2.5

ExtendablePane {
    id: root

    showTitle: false

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null
    property var panelDefinition: null

    property var tabIds: panelDefinition !== null ? panelDefinition.tabs : []

    content: [
        TabBar {
            id: tabBar
            anchors.top: parent.top
            visible: root.tabIds.length > 1

            Repeater {
                model: root.tabIds

                StandardTabButton {
                    width: implicitWidth
                    text: root.tabTitle(modelData.tabId)
                }
            }
        },

        StandardTabView {
            id: swipeView
            anchors.top: tabBar.visible ? tabBar.bottom : parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Const.smallPadding
            currentIndex: tabBar.currentIndex
            clip: true

            Repeater {
                model: root.tabIds

                Loader {
                    source: root.tabSource(modelData.tabId)

                    onLoaded: {
                        root.setupTabItem(item)
                    }
                }
            }
        }
    ]

    onRepositoryChanged: refreshTabItems()
    onViewRootChanged: refreshTabItems()
    onFlatSelectionChanged: refreshTabItems()
    onPanelDefinitionChanged: {
        tabBar.currentIndex = 0
        refreshTabItems()
    }

    function tabTitle(tabId) {
        if (tabId === "Files")
            return Const.filesText
        if (tabId === "Branches")
            return Const.branchesText
        if (tabId === "Tags")
            return Const.tagsText
        if (tabId === "BranchLog")
            return Const.branchLogText
        if (tabId === "Graph")
            return Const.graphText
        if (tabId === "RefLog")
            return Const.refLogText
        if (tabId === "Diff")
            return Const.diffText
        if (tabId === "FileLog")
            return Const.logText
        if (tabId === "Blame")
            return Const.blameText
        if (tabId === "Output")
            return Const.outputText
        return tabId
    }

    function tabSource(tabId) {
        if (tabId === "Files")
            return "../tabs/FilesTab.qml"
        if (tabId === "Branches")
            return "../tabs/BranchesTab.qml"
        if (tabId === "Tags")
            return "../tabs/TagsTab.qml"
        if (tabId === "BranchLog")
            return "../tabs/BranchLogTab.qml"
        if (tabId === "Graph")
            return "../tabs/GraphTab.qml"
        if (tabId === "RefLog")
            return "../tabs/RefLogTab.qml"
        if (tabId === "Diff")
            return "../tabs/DiffTab.qml"
        if (tabId === "FileLog")
            return "../tabs/FileLogTab.qml"
        if (tabId === "Blame")
            return "../tabs/BlameTab.qml"
        if (tabId === "Output")
            return "../tabs/OutputTab.qml"
        return ""
    }

    function setupTabItem(tabItem) {
        if (tabItem === null || tabItem === undefined)
            return
        if ("repository" in tabItem)
            tabItem.repository = root.repository
        if ("viewRoot" in tabItem)
            tabItem.viewRoot = root.viewRoot
        if ("flatSelection" in tabItem)
            tabItem.flatSelection = root.flatSelection
    }

    function refreshTabItems() {
        for (var index = 0; index < swipeView.count; index++) {
            var loader = swipeView.itemAt(index)
            if (loader !== null && loader.item !== null && loader.item !== undefined)
                setupTabItem(loader.item)
        }
    }

    function findTab(tabId) {
        for (var index = 0; index < root.tabIds.length; index++) {
            if (root.tabIds[index].tabId === tabId) {
                var loader = swipeView.itemAt(index)
                if (loader !== null)
                    return loader.item
                return null
            }
        }
        return null
    }

    function activateTab(tabId) {
        for (var index = 0; index < root.tabIds.length; index++) {
            if (root.tabIds[index].tabId === tabId) {
                tabBar.currentIndex = index
                var loader = swipeView.itemAt(index)
                if (loader !== null && loader.item !== null && loader.item !== undefined) {
                    if ("focusTab" in loader.item)
                        loader.item.focusTab()
                }
                return true
            }
        }
        return false
    }

    function getSelectedFiles() {
        var filesTab = findTab("Files")
        if (filesTab !== null && filesTab !== undefined && "getSelectedFiles" in filesTab)
            return filesTab.getSelectedFiles()
        return []
    }
}
