import QtQuick 2.12
import QtQuick.Controls 2.5

ExtendablePane {
    id: root

    showTitle: false

    property variant repository: null
    property variant viewRoot: null
    property variant flatSelection: null
    property var panelDefinition: null

    property var tabIds: panelDefinition !== null && panelDefinition[Const.panelTabsKey] !== undefined
                         ? panelDefinition[Const.panelTabsKey] : []

    content: [
        TabBar {
            id: tabBar
            anchors.top: parent.top
            visible: root.tabIds.length > 1

            onCurrentIndexChanged: root.loadCurrentTabData()

            Repeater {
                model: root.tabIds

                StandardTabButton {
                    width: implicitWidth
                    text: root.tabTitle(modelData[Const.panelTabIdKey])
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
            visible: root.tabIds.length > 0

            onCurrentIndexChanged: {
                if (tabBar.currentIndex !== currentIndex)
                    tabBar.currentIndex = currentIndex
            }

            Repeater {
                model: root.tabIds

                Loader {
                    source: root.tabSource(modelData[Const.panelTabIdKey])

                    onLoaded: {
                        root.setupTabItem(item)
                        if (index === tabBar.currentIndex)
                            root.loadCurrentTabData()
                    }
                }
            }
        },

        StandardLabel {
            anchors.top: tabBar.visible ? tabBar.bottom : parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: Const.emptyPanelText
            visible: root.tabIds.length === 0
        }
    ]

    Connections {
        target: root.repository
        function onLogDataStaled() {
            root.loadCurrentTabData()
        }
    }

    onRepositoryChanged: {
        refreshTabItems()
        loadCurrentTabData()
    }
    onViewRootChanged: refreshTabItems()
    onFlatSelectionChanged: refreshTabItems()
    onPanelDefinitionChanged: {
        if (root.tabIds.length > 0) {
            if (tabBar.currentIndex < 0 || tabBar.currentIndex >= root.tabIds.length)
                tabBar.currentIndex = 0
        }
        refreshTabItems()
        loadCurrentTabData()
    }

    function tabTitle(tabId) {
        if (tabId in Const.tabTitles)
            return Const.tabTitles[tabId]
        console.warn("DockPanel: unknown tab identifier '" + tabId + "'")
        return tabId
    }

    function tabSource(tabId) {
        if (tabId in Const.tabSources)
            return Const.tabSources[tabId]
        console.warn("DockPanel: no source for unknown tab identifier '" + tabId + "'")
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

    function currentTabItem() {
        var index = tabBar.currentIndex
        if (index < 0 || index >= swipeView.count)
            return null
        var loader = swipeView.itemAt(index)
        if (loader === null || loader === undefined)
            return null
        return loader.item
    }

    function loadCurrentTabData() {
        var tabItem = root.currentTabItem()
        if (tabItem !== null && tabItem !== undefined && "loadData" in tabItem)
            tabItem.loadData()
    }

    function tabIndexOf(tabId) {
        for (var index = 0; index < root.tabIds.length; index++) {
            if (root.tabIds[index][Const.panelTabIdKey] === tabId)
                return index
        }
        return -1
    }

    function findTab(tabId) {
        var index = root.tabIndexOf(tabId)
        if (index === -1)
            return null
        var loader = swipeView.itemAt(index)
        if (loader !== null)
            return loader.item
        return null
    }

    function activateTab(tabId) {
        var index = root.tabIndexOf(tabId)
        if (index === -1)
            return false
        tabBar.currentIndex = index
        var loader = swipeView.itemAt(index)
        if (loader !== null && loader.item !== null && loader.item !== undefined) {
            if ("focusTab" in loader.item)
                loader.item.focusTab()
        }
        return true
    }

    function showTab(tabId) {
        var index = root.tabIndexOf(tabId)
        if (index === -1)
            return false
        tabBar.currentIndex = index
        return true
    }

    function getSelectedFiles() {
        var filesTab = findTab(Const.filesTabId)
        if (filesTab !== null && filesTab !== undefined && "getSelectedFiles" in filesTab)
            return filesTab.getSelectedFiles()
        return []
    }
}
