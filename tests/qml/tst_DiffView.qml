import QtQuick 2.12
import QtTest 1.2
import CuteGit 1.0
import DiffTest 1.0

TestCase {
    id: testCase
    name: "DiffViewSelection"
    when: windowShown
    visible: true
    width: 400
    height: 600

    DiffTestHelper {
        id: helper
    }

    Loader {
        id: viewLoader
        anchors.fill: parent
    }

    SignalSpy {
        id: stageSpy
        target: helper.repository
        signalName: "stageDiffLinesCalled"
    }

    SignalSpy {
        id: unstageSpy
        target: helper.repository
        signalName: "unstageDiffLinesCalled"
    }

    property string diffViewUrl: "../../CuteGit/qml/views/DiffView.qml"

    function buildModel() {
        helper.reset()
        helper.addFileHeader("tracked.txt")
        helper.addHunk(0, "@@ -1,4 +1,5 @@ tracked.txt")
        helper.addLine(CEnums.None, " context1", 0)
        helper.addLine(CEnums.DiffAdd, "+added1", 0)
        helper.addLine(CEnums.DiffAdd, "+added2", 0)
        helper.addLine(CEnums.None, " context2", 0)
        helper.commitLines()
    }

    function createView(staged) {
        stageSpy.clear()
        unstageSpy.clear()
        buildModel()

        viewLoader.setSource(diffViewUrl, {
            model: helper.modelProxy,
            repository: helper.repository,
            staged: staged !== undefined ? staged : false,
            stagingEnabled: true,
            width: testCase.width,
            height: testCase.height
        })

        var view = viewLoader.item
        verify(view !== null, "view is loaded")

        return view
    }

    function delegateItem(view, row) {
        var item = null

        tryVerify(function() {
            item = findChild(view, "root.delegate." + row)
            return item !== null
        }, 2000, "delegate row " + row + " is created")

        return item
    }

    function clickRow(view, row, modifiers) {
        var item = delegateItem(view, row)
        mouseClick(item, item.width / 2, item.height / 2,
            Qt.LeftButton, modifiers === undefined ? Qt.NoModifier : modifiers)
    }

    function selectedAsText(view) {
        return view.selectedRows.join(",")
    }

    function test_clickSelectsSingleRow() {
        var view = createView()
        clickRow(view, 3)
        compare(selectedAsText(view), "3")
    }

    function test_plainClickReplacesSelection() {
        var view = createView()
        clickRow(view, 2)
        clickRow(view, 5, Qt.ControlModifier)
        compare(selectedAsText(view), "2,5")
        clickRow(view, 4)
        compare(selectedAsText(view), "4")
    }

    function test_ctrlClickTogglesRows() {
        var view = createView()
        clickRow(view, 3)
        clickRow(view, 4, Qt.ControlModifier)
        compare(selectedAsText(view), "3,4")
        clickRow(view, 3, Qt.ControlModifier)
        compare(selectedAsText(view), "4")
    }

    function test_shiftClickSelectsRange() {
        var view = createView()
        clickRow(view, 2)
        clickRow(view, 5, Qt.ShiftModifier)
        compare(selectedAsText(view), "2,3,4,5")
    }

    function test_shiftClickUpwardsSelectsRange() {
        var view = createView()
        clickRow(view, 5)
        clickRow(view, 2, Qt.ShiftModifier)
        compare(selectedAsText(view), "2,3,4,5")
    }

    function test_shiftDownExtendsSelection() {
        var view = createView()
        clickRow(view, 2)
        verify(view.activeFocus, "view keeps keyboard focus after click")
        keyClick(Qt.Key_Down, Qt.ShiftModifier)
        keyClick(Qt.Key_Down, Qt.ShiftModifier)
        compare(selectedAsText(view), "2,3,4")
    }

    function test_plainDownMovesSelection() {
        var view = createView()
        clickRow(view, 2)
        clickRow(view, 4, Qt.ControlModifier)
        keyClick(Qt.Key_Down, Qt.NoModifier)
        compare(selectedAsText(view), "5")
    }

    function test_ctrlASelectsAll() {
        var view = createView()
        clickRow(view, 3)
        keyClick(Qt.Key_A, Qt.ControlModifier)
        compare(selectedAsText(view), "0,1,2,3,4,5")
    }

    function test_spaceStagesSelection() {
        var view = createView(false)
        clickRow(view, 3)
        clickRow(view, 4, Qt.ControlModifier)
        keyClick(Qt.Key_Space, Qt.NoModifier)
        compare(stageSpy.count, 1)
        compare(unstageSpy.count, 0)
        verify(stageSpy.signalArguments[0][0].indexOf(3) !== -1, "staged rows hold 3")
        verify(stageSpy.signalArguments[0][0].indexOf(4) !== -1, "staged rows hold 4")
    }

    function test_spaceUnstagesWhenStaged() {
        var view = createView(true)
        clickRow(view, 3)
        keyClick(Qt.Key_Space, Qt.NoModifier)
        compare(unstageSpy.count, 1)
        compare(stageSpy.count, 0)
    }

    function test_spaceFallsBackToCurrentRow() {
        var view = createView(false)
        delegateItem(view, 0)
        view.forceActiveFocus()
        verify(view.activeFocus, "view takes keyboard focus")
        keyClick(Qt.Key_Space, Qt.NoModifier)
        compare(view.currentIndex, 0)
        compare(stageSpy.count, 1)
    }

    function test_spaceDisabledWithoutRepository() {
        stageSpy.clear()
        unstageSpy.clear()
        buildModel()

        viewLoader.setSource(diffViewUrl, {
            model: helper.modelProxy,
            stagingEnabled: false,
            width: testCase.width,
            height: testCase.height
        })

        var view = viewLoader.item
        verify(view !== null, "view is loaded")

        clickRow(view, 3)
        keyClick(Qt.Key_Space, Qt.NoModifier)
        compare(stageSpy.count, 0)
        compare(unstageSpy.count, 0)
    }
}
