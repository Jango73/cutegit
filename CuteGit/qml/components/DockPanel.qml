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

    /*! Mirrors the repository view active tabs without a binding, so reporting the current tab never loops */
    property var activeTabIds: []

    property var visibleTabs: []
    property string currentTabId: root.computeCurrentTabId()
    property string preferredTabId: ""
    property bool adjustingIndex: false

    content: [
        TabBar {
            id: tabBar
            anchors.top: parent.top
            visible: root.visibleTabs.length > 1

            onCurrentIndexChanged: root.handleUserIndexChange()

            Repeater {
                model: root.visibleTabs

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
            visible: root.visibleTabs.length > 0

            onCurrentIndexChanged: {
                if (tabBar.currentIndex !== currentIndex)
                    tabBar.currentIndex = currentIndex
            }

            Repeater {
                model: root.visibleTabs

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
            visible: root.visibleTabs.length === 0
        }
    ]

    Connections {
        target: root.repository
        function onLogDataStaled() {
            root.loadCurrentTabData()
        }
    }

    Connections {
        target: root.viewRoot
        function onActiveTabIdsChanged() {
            root.syncActiveTabIds()
        }
    }

    Component.onCompleted: {
        root.syncActiveTabIds()
        root.updateVisibleTabs()
        root.clampCurrentIndex()
        root.reportActiveTab()
    }

    onTabIdsChanged: root.updateVisibleTabs()
    onActiveTabIdsChanged: root.updateVisibleTabs()

    onRepositoryChanged: {
        refreshTabItems()
        loadCurrentTabData()
    }
    onViewRootChanged: {
        root.syncActiveTabIds()
        refreshTabItems()
    }
    onFlatSelectionChanged: refreshTabItems()
    onPanelDefinitionChanged: {
        root.preferredTabId = ""
        root.clampCurrentIndex()
        refreshTabItems()
        loadCurrentTabData()
    }
    onVisibleTabsChanged: {
        root.restorePreferredTab()
    }
    onCurrentTabIdChanged: {
        root.loadCurrentTabData()
        root.reportActiveTab()
    }

    function tabCondition(tabDefinition) {
        if (tabDefinition === null || tabDefinition === undefined)
            return []
        var condition = tabDefinition[Const.panelTabVisibleWhenKey]
        if (condition === undefined || condition === null)
            return []
        if (typeof condition === "string")
            return condition.length === 0 ? [] : [condition]
        return condition
    }

    function isTabVisible(tabDefinition) {
        var condition = root.tabCondition(tabDefinition)
        if (condition.length === 0)
            return true
        for (var conditionIndex = 0; conditionIndex < condition.length; conditionIndex++) {
            if (root.activeTabIds.indexOf(condition[conditionIndex]) !== -1)
                return true
        }
        return false
    }

    function computeVisibleTabs() {
        var result = []
        for (var index = 0; index < root.tabIds.length; index++) {
            if (root.isTabVisible(root.tabIds[index]))
                result.push(root.tabIds[index])
        }
        return result
    }

    function visibleTabIdsEqual(first, second) {
        if (first.length !== second.length)
            return false
        for (var index = 0; index < first.length; index++) {
            if (first[index][Const.panelTabIdKey] !== second[index][Const.panelTabIdKey])
                return false
        }
        return true
    }

    function updateVisibleTabs() {
        var computedTabs = root.computeVisibleTabs()
        if (root.visibleTabIdsEqual(root.visibleTabs, computedTabs) === false)
            root.visibleTabs = computedTabs
    }

    function activeTabIdsEqual(first, second) {
        if (first.length !== second.length)
            return false
        for (var index = 0; index < first.length; index++) {
            if (second.indexOf(first[index]) === -1)
                return false
        }
        return true
    }

    function syncActiveTabIds() {
        var mirroredIds = []
        if (root.viewRoot !== null && root.viewRoot !== undefined && root.viewRoot.activeTabIds !== undefined)
            mirroredIds = root.viewRoot.activeTabIds
        if (root.activeTabIdsEqual(root.activeTabIds, mirroredIds) === false)
            root.activeTabIds = mirroredIds
    }

    function computeCurrentTabId() {
        if (root.visibleTabs.length === 0)
            return ""
        var index = tabBar.currentIndex
        if (index === undefined || index === null || index < 0 || index >= root.visibleTabs.length)
            return ""
        var tabDefinition = root.visibleTabs[index]
        if (tabDefinition === undefined || tabDefinition === null)
            return ""
        return tabDefinition[Const.panelTabIdKey]
    }

    function setCurrentIndex(index) {
        if (tabBar.currentIndex === index)
            return
        root.adjustingIndex = true
        tabBar.currentIndex = index
        root.adjustingIndex = false
    }

    function clampCurrentIndex() {
        if (root.visibleTabs.length === 0)
            return
        if (tabBar.currentIndex === undefined || tabBar.currentIndex === null
                || tabBar.currentIndex < 0 || tabBar.currentIndex >= root.visibleTabs.length)
            root.setCurrentIndex(0)
        if (root.preferredTabId === "")
            root.preferredTabId = root.computeCurrentTabId()
    }

    function restorePreferredTab() {
        if (root.visibleTabs.length === 0)
            return
        if (root.preferredTabId !== "") {
            var preferredIndex = root.tabIndexOf(root.preferredTabId)
            if (preferredIndex !== -1) {
                root.setCurrentIndex(preferredIndex)
                return
            }
        }
        root.clampCurrentIndex()
        root.preferredTabId = root.computeCurrentTabId()
    }

    function handleUserIndexChange() {
        if (root.adjustingIndex)
            return
        var tabId = root.computeCurrentTabId()
        if (tabId !== "")
            root.preferredTabId = tabId
    }

    function reportActiveTab() {
        if (root.viewRoot !== null && root.viewRoot !== undefined && "refreshActiveTabs" in root.viewRoot)
            root.viewRoot.refreshActiveTabs()
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
        for (var index = 0; index < root.visibleTabs.length; index++) {
            if (root.visibleTabs[index][Const.panelTabIdKey] === tabId)
                return index
        }
        return -1
    }

    function hasTab(tabId) {
        for (var index = 0; index < root.tabIds.length; index++) {
            if (root.tabIds[index][Const.panelTabIdKey] === tabId)
                return true
        }
        return false
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
        root.preferredTabId = tabId
        root.setCurrentIndex(index)
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
        root.preferredTabId = tabId
        root.setCurrentIndex(index)
        return true
    }

    function getSelectedFiles() {
        var filesTab = findTab(Const.filesTabId)
        if (filesTab !== null && filesTab !== undefined && "getSelectedFiles" in filesTab)
            return filesTab.getSelectedFiles()
        return []
    }
}
