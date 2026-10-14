// Qt
#include <QtTest>
#include <QDir>
#include <QProcess>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>

// Application
#include "CPatchBuilder.h"
#include "CDiffLine.h"
#include "CDiffModel.h"
#include "CDiffModelProxy.h"

//-------------------------------------------------------------------------------------------------

class TestPatchBuilder : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();

    void patch_singleAddLine();
    void patch_deleteBecomesContext();
    void patch_newFile();
    void patch_deletedFile();
    void patch_contextOnlySelection_returnsEmpty();
    void patch_emptySelection_returnsEmpty();

    void hunkRows_selectableOnly();
    void proxyRows_identityMapping();
    void proxyRows_filteredMapping();

    void roundTrip_stageSubset();

private:

    QString m_sFixtureRepo;

    static CDiffLine* makeLine(CEnums::EDiffOperation eOperation, const QString& sText, int iHunkId);
    static QString runGit(const QString& sWorkDir, const QStringList& lArgs);
};

//-------------------------------------------------------------------------------------------------

CDiffLine* TestPatchBuilder::makeLine(CEnums::EDiffOperation eOperation, const QString& sText, int iHunkId)
{
    CDiffLine* pLine = new CDiffLine();
    pLine->setOperation(eOperation);
    pLine->setText(sText);
    pLine->setHunkId(iHunkId);
    return pLine;
}

//-------------------------------------------------------------------------------------------------

QString TestPatchBuilder::runGit(const QString& sWorkDir, const QStringList& lArgs)
{
    QStringList lFullArgs;
    lFullArgs << "-C" << sWorkDir << lArgs;

    QProcess tProcess;
    tProcess.start("git", lFullArgs);

    if (not tProcess.waitForFinished(15000))
    {
        tProcess.kill();
        return QString("TIMEOUT: git %1").arg(lFullArgs.join(" "));
    }

    if (tProcess.exitCode() != 0)
        return QString("EXIT %1: %2").arg(tProcess.exitCode()).arg(QString::fromUtf8(tProcess.readAllStandardError()));

    return QString();
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::initTestCase()
{
    m_sFixtureRepo = qEnvironmentVariable("DIFF_FIXTURE_REPO");
    QVERIFY2(not m_sFixtureRepo.isEmpty(), "DIFF_FIXTURE_REPO environment variable is not set");
    QVERIFY2(QDir(m_sFixtureRepo + "/.git").exists(), "Fixture repository has no .git folder");
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::patch_singleAddLine()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "tracked.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,4 +1,5 @@ tracked.txt", 0),
        makeLine(CEnums::None, " context1", 0),
        makeLine(CEnums::DiffAdd, "+added1", 0),
        makeLine(CEnums::DiffAdd, "+added2", 0),
        makeLine(CEnums::None, " context2", 0),
    });

    // Select only the first added line: the second one is dropped from the patch
    QString sPatch = CPatchBuilder::patchFromSelection(&tModel, QSet<int>({3}), "tracked.txt");

    QString sExpected =
        "diff --git a/tracked.txt b/tracked.txt\n"
        "--- a/tracked.txt\n"
        "+++ b/tracked.txt\n"
        "@@ -1,2 +1,3 @@ tracked.txt\n"
        " context1\n"
        "+added1\n"
        " context2\n";

    QCOMPARE(sPatch, sExpected);
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::patch_deleteBecomesContext()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "tracked.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,4 +1,4 @@ tracked.txt", 0),
        makeLine(CEnums::None, " context1", 0),
        makeLine(CEnums::DiffDelete, "-deleted1", 0),
        makeLine(CEnums::DiffDelete, "-deleted2", 0),
        makeLine(CEnums::DiffAdd, "+added1", 0),
    });

    // Select only the added line: deleted lines are kept as context
    QString sPatch = CPatchBuilder::patchFromSelection(&tModel, QSet<int>({5}), "tracked.txt");

    QString sExpected =
        "diff --git a/tracked.txt b/tracked.txt\n"
        "--- a/tracked.txt\n"
        "+++ b/tracked.txt\n"
        "@@ -1,3 +1,4 @@ tracked.txt\n"
        " context1\n"
        " deleted1\n"
        " deleted2\n"
        "+added1\n";

    QCOMPARE(sPatch, sExpected);
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::patch_newFile()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "new.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -0,0 +1,2 @@ new.txt", 0),
        makeLine(CEnums::DiffAdd, "+line1", 0),
        makeLine(CEnums::DiffAdd, "+line2", 0),
    });

    QString sPatch = CPatchBuilder::patchFromSelection(&tModel, QSet<int>({2, 3}), "new.txt");

    QString sExpected =
        "diff --git a/new.txt b/new.txt\n"
        "--- /dev/null\n"
        "+++ b/new.txt\n"
        "@@ -0,0 +1,2 @@ new.txt\n"
        "+line1\n"
        "+line2\n";

    QCOMPARE(sPatch, sExpected);
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::patch_deletedFile()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "gone.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,2 +0,0 @@ gone.txt", 0),
        makeLine(CEnums::DiffDelete, "-line1", 0),
        makeLine(CEnums::DiffDelete, "-line2", 0),
    });

    QString sPatch = CPatchBuilder::patchFromSelection(&tModel, QSet<int>({2, 3}), "gone.txt");

    QString sExpected =
        "diff --git a/gone.txt b/gone.txt\n"
        "--- a/gone.txt\n"
        "+++ /dev/null\n"
        "@@ -1,2 +0,0 @@ gone.txt\n"
        "-line1\n"
        "-line2\n";

    QCOMPARE(sPatch, sExpected);
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::patch_contextOnlySelection_returnsEmpty()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "tracked.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,2 +1,3 @@ tracked.txt", 0),
        makeLine(CEnums::None, " context1", 0),
        makeLine(CEnums::DiffAdd, "+added1", 0),
    });

    // Selecting a context line only stages nothing
    QCOMPARE(CPatchBuilder::patchFromSelection(&tModel, QSet<int>({2}), "tracked.txt"), QString());
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::patch_emptySelection_returnsEmpty()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "tracked.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,2 +1,3 @@ tracked.txt", 0),
        makeLine(CEnums::DiffAdd, "+added1", 0),
    });

    QCOMPARE(CPatchBuilder::patchFromSelection(&tModel, QSet<int>(), "tracked.txt"), QString());
    QCOMPARE(CPatchBuilder::patchFromSelection(nullptr, QSet<int>({2}), "tracked.txt"), QString());
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::hunkRows_selectableOnly()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "tracked.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,3 +1,4 @@ tracked.txt", 0),
        makeLine(CEnums::None, " context1", 0),
        makeLine(CEnums::DiffDelete, "-deleted1", 0),
        makeLine(CEnums::DiffAdd, "+added1", 0),
        makeLine(CEnums::DiffHunk, "@@ -10,2 +11,2 @@ tracked.txt", 1),
        makeLine(CEnums::DiffAdd, "+other", 1),
    });

    QCOMPARE(CPatchBuilder::selectableSourceRowsForHunk(&tModel, 0), QList<int>({3, 4}));
    QCOMPARE(CPatchBuilder::selectableSourceRowsForHunk(&tModel, 1), QList<int>({6}));
    QVERIFY(CPatchBuilder::selectableSourceRowsForHunk(&tModel, 42).isEmpty());
    QVERIFY(CPatchBuilder::selectableSourceRowsForHunk(nullptr, 0).isEmpty());
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::proxyRows_identityMapping()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "tracked.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,2 +1,3 @@ tracked.txt", 0),
        makeLine(CEnums::None, " context1", 0),
        makeLine(CEnums::DiffAdd, "+added1", 0),
    });

    CDiffModelProxy tProxy(nullptr);
    tProxy.setSourceModel(&tModel);

    QSet<int> sSourceRows = CPatchBuilder::sourceRowsFromProxyRows(&tProxy, &tModel, QVariantList({2, 3}));

    QCOMPARE(sSourceRows, QSet<int>({2, 3}));
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::proxyRows_filteredMapping()
{
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "tracked.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,2 +1,3 @@ tracked.txt", 0),
        makeLine(CEnums::None, " context1", 0),
        makeLine(CEnums::DiffAdd, "+added1", 0),
    });

    CDiffModelProxy tProxy(nullptr);
    tProxy.setSourceModel(&tModel);
    tProxy.setTextFilter("added1");
    tProxy.filterChanged();

    // File headers stay visible as context: proxy rows are header (source 0)
    // and the matching added line (source 3)
    QCOMPARE(tProxy.rowCount(), 2);
    QCOMPARE(CPatchBuilder::sourceRowsFromProxyRows(&tProxy, &tModel, QVariantList({0, 1})), QSet<int>({0, 3}));

    // Out of range proxy rows are dropped
    QVERIFY(CPatchBuilder::sourceRowsFromProxyRows(&tProxy, &tModel, QVariantList({7})).isEmpty());
}

//-------------------------------------------------------------------------------------------------

void TestPatchBuilder::roundTrip_stageSubset()
{
    QTemporaryDir tWorkDir;
    QVERIFY2(tWorkDir.isValid(), "Unable to create temporary folder");

    QString sWork = tWorkDir.path() + "/work";
    QCOMPARE(runGit(tWorkDir.path(), {"clone", "-q", m_sFixtureRepo, sWork}), QString());

    // Commit a known file, then modify it in the working tree
    QString sFilePath = sWork + "/staged-subset.txt";
    QFile tFile(sFilePath);

    QVERIFY(tFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream(&tFile) << "A1\nA2\nA3\nA4\nA5\nA6\n";
    tFile.close();

    QCOMPARE(runGit(sWork, {"add", "staged-subset.txt"}), QString());
    QCOMPARE(runGit(sWork, {"-c", "user.name=Test", "-c", "user.email=test@example.com",
        "commit", "-qm", "Base file for subset staging"}), QString());

    QVERIFY(tFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QTextStream(&tFile) << "A1\nA2\nB3\nA4\nA5\nA6\nA7\n";
    tFile.close();

    // Mirror the real unstaged diff of that change
    CDiffModel tModel;
    tModel.setLines({
        makeLine(CEnums::DiffFileName, "staged-subset.txt", -1),
        makeLine(CEnums::DiffHunk, "@@ -1,6 +1,7 @@ staged-subset.txt", 0),
        makeLine(CEnums::None, " A1", 0),
        makeLine(CEnums::None, " A2", 0),
        makeLine(CEnums::DiffDelete, "-A3", 0),
        makeLine(CEnums::DiffAdd, "+B3", 0),
        makeLine(CEnums::None, " A4", 0),
        makeLine(CEnums::None, " A5", 0),
        makeLine(CEnums::None, " A6", 0),
        makeLine(CEnums::DiffAdd, "+A7", 0),
    });

    // Stage only the added B3 line, like the unstaged pane does
    QString sPatch = CPatchBuilder::patchFromSelection(&tModel, QSet<int>({5}), "staged-subset.txt");
    QVERIFY2(not sPatch.isEmpty(), "Patch must not be empty");

    QTemporaryFile tPatchFile(QDir::tempPath() + "/CuteGitTestXXXXXX.patch");
    QVERIFY(tPatchFile.open());
    tPatchFile.write(sPatch.toUtf8());
    tPatchFile.flush();

    QCOMPARE(runGit(sWork, {"apply", "--cached", "--unidiff-zero", tPatchFile.fileName()}), QString());

    // The index holds B3 but not A7
    QProcess tShow;
    tShow.start("git", QStringList({"-C", sWork, "diff", "--cached", "--", "staged-subset.txt"}));
    QVERIFY(tShow.waitForFinished(15000));
    QCOMPARE(tShow.exitCode(), 0);

    QString sStagedDiff = QString::fromUtf8(tShow.readAllStandardOutput());
    QVERIFY2(sStagedDiff.contains("+B3"), qPrintable("Staged diff misses +B3:\n" + sStagedDiff));
    QVERIFY2(not sStagedDiff.contains("+A7"), qPrintable("Staged diff must not hold +A7:\n" + sStagedDiff));
}

//-------------------------------------------------------------------------------------------------

QTEST_MAIN(TestPatchBuilder)

#include "tst_patchbuilder.moc"
