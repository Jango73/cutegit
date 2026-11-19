
// Qt
#include <QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QScopedPointer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

// Application
#include "CController.h"
#include "CRepository.h"
#include "CLogModel.h"
#include "CGraphModel.h"
#include "CGraphLine.h"
#include "CGraphLineCollection.h"
#include "CLabelModel.h"
#include "commands/CGitCommands.h"
#include "CPatchBuilder.h"
#include "CDiffModel.h"
#include "CDiffModelProxy.h"
#include "CDiffLine.h"
#include "CEnums.h"

/*!
    \class tst_Repository
    Integration test: drives a real CRepository (real CGitCommands, real git
    repository cloned from the fixture) through the same calls the QML
    DiffView and DiffMenu make, and checks the git index as ground truth.
*/

static const int s_iAsyncTimeoutMs = 20000;

class tst_Repository : public QObject
{
    Q_OBJECT

public:

    tst_Repository()
        : m_sFixtureRepo(qgetenv("DIFF_FIXTURE_REPO"))
    {
        // No parameters needed to run the binary directly: fall back to the
        // fixture shipped next to it (tests/bin -> tests/fixtures/diff/repo)
        if (m_sFixtureRepo.isEmpty())
        {
            QString sFallback = QCoreApplication::applicationDirPath() + "/../fixtures/diff/repo";

            if (QDir(sFallback).exists())
                m_sFixtureRepo = QDir(sFallback).canonicalPath();
        }
    }

private slots:

    void initTestCase()
    {
        // Isolate any controller created by a test from the real configuration
        QStandardPaths::setTestModeEnabled(true);

        QVERIFY2(not m_sFixtureRepo.isEmpty(), "DIFF_FIXTURE_REPO is not set");

        QDir tFixture(m_sFixtureRepo);

        QVERIFY2(tFixture.exists(), qPrintable(QString("Fixture repository is missing: %1").arg(m_sFixtureRepo)));
        QVERIFY2(QFile::exists(m_sFixtureRepo + "/.git/HEAD"), "Fixture repository has no .git");
    }

    void init()
    {
        cloneFixture();
        appendWorktreeLines();

        // Real repository object, no controller: file proxies show everything.
        // Construction is lazy, the first refresh issues the scan like the tab view does
        m_pRepository.reset(new CRepository(m_sClonePath, nullptr));
        m_pRepository->refresh();
    }

    void cleanup()
    {
        // Destroy the repository (and its worker thread) before removing the clone
        m_pRepository.reset();
        m_pWorkDir.reset();
    }

    void cleanupTestCase()
    {
    }

    // Stages every added line: the index must contain them, the working tree diff must be empty
    void stageLines_movesLineToIndex()
    {
        selectFileAndWaitForDiff();

        CDiffModel* pUnstaged = m_pRepository->unstagedFileDiffModel();
        QList<int> lAdded = addedSourceRows(pUnstaged, "line-B");

        QCOMPARE(lAdded.count(), 3);

        QVariantList lProxyRows = proxyRows(pUnstaged, lAdded);

        m_pRepository->stageDiffLines(lProxyRows);

        QString sCached;
        QVERIFY(gitOutput({"diff", "--cached", "--", m_sRelativeName}, sCached));

        QVERIFY2(sCached.contains("+line-B1"), qPrintable(sCached + "\n--- output ---\n" + outputLog()));
        QVERIFY2(sCached.contains("+line-B2"), qPrintable(sCached));
        QVERIFY2(sCached.contains("+line-B3"), qPrintable(sCached));

        QString sUnstaged;
        QVERIFY(gitOutput({"diff", "--", m_sRelativeName}, sUnstaged));

        QVERIFY2(sUnstaged.trimmed().isEmpty(), qPrintable(sUnstaged));
    }

    // Stages a single line without re-requesting diffs: the status cycle must
    // refresh both panes on its own, like in the running application
    void stageSingleLine_refreshesBothModels()
    {
        selectFileAndWaitForDiff();

        CDiffModel* pUnstaged = m_pRepository->unstagedFileDiffModel();
        QList<int> lAdded = addedSourceRows(pUnstaged, "line-B1");

        QCOMPARE(lAdded.count(), 1);

        m_pRepository->stageDiffLines(proxyRows(pUnstaged, lAdded));

        QString sCached;
        QVERIFY(gitOutput({"diff", "--cached", "--", m_sRelativeName}, sCached));

        QVERIFY2(sCached.contains("+line-B1"), qPrintable(sCached));
        QVERIFY2(not sCached.contains("line-B2"), qPrintable(sCached));

        CDiffModel* pStaged = m_pRepository->stagedFileDiffModel();

        QTRY_VERIFY_WITH_TIMEOUT(addedContains(pStaged, "line-B1"), s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(not addedContains(pUnstaged, "line-B1"), s_iAsyncTimeoutMs);
        // Staged and unstaged refreshes are independent asynchronous payloads:
        // the unstaged pane must be awaited, not assumed already there
        QTRY_VERIFY_WITH_TIMEOUT(addedContains(pUnstaged, "line-B2"), s_iAsyncTimeoutMs);
    }

    // Stages two lines, re-requests diffs, unstages one of them
    void unstageLines_movesLineBack()
    {
        selectFileAndWaitForDiff();

        CDiffModel* pUnstaged = m_pRepository->unstagedFileDiffModel();
        QList<int> lAdded = addedSourceRows(pUnstaged, "line-B");

        QCOMPARE(lAdded.count(), 3);

        // Stage B1 and B2 only: the file stays dirty so models are never cleared
        QList<int> lToStage;

        for (int iRow : lAdded)
        {
            QString sText = pUnstaged->lineAt(iRow)->text();

            if (sText.contains("line-B1") || sText.contains("line-B2"))
                lToStage << iRow;
        }

        QCOMPARE(lToStage.count(), 2);

        m_pRepository->stageDiffLines(proxyRows(pUnstaged, lToStage));

        // Re-request diffs: same path twice is a no-op, clear first
        QMetaObject::invokeMethod(m_pRepository.data(), "onCurrentFileFullName", Q_ARG(QString, QString("")));
        QMetaObject::invokeMethod(m_pRepository.data(), "onCurrentFileFullName", Q_ARG(QString, m_sFullName));

        CDiffModel* pStaged = m_pRepository->stagedFileDiffModel();

        QTRY_VERIFY_WITH_TIMEOUT(modelContains(pStaged, "line-B1"), s_iAsyncTimeoutMs);
        QVERIFY(modelContains(pStaged, "line-B2"));

        QList<int> lStaged = addedSourceRows(pStaged, "line-B1");

        QCOMPARE(lStaged.count(), 1);

        CDiffModelProxy* pStagedProxy = m_pRepository->stagedFileDiffModelProxy();

        m_pRepository->unstageDiffLines(proxyRows(pStagedProxy, pStaged, lStaged));

        QString sCached;
        QVERIFY(gitOutput({"diff", "--cached", "--", m_sRelativeName}, sCached));

        QVERIFY2(not sCached.contains("line-B1"), qPrintable(sCached));
        QVERIFY2(sCached.contains("+line-B2"), qPrintable(sCached));

        QString sUnstaged;
        QVERIFY(gitOutput({"diff", "--", m_sRelativeName}, sUnstaged));

        QVERIFY2(sUnstaged.contains("+line-B1"), qPrintable(sUnstaged));
    }

    // Empty selection is a silent no-op: the index stays untouched
    void stageEmptySelection_doesNothing()
    {
        selectFileAndWaitForDiff();

        m_pRepository->stageDiffLines(QVariantList());

        QString sCached;
        QVERIFY(gitOutput({"diff", "--cached", "--", m_sRelativeName}, sCached));

        QVERIFY2(sCached.trimmed().isEmpty(), qPrintable(sCached));

        QString sUnstaged;
        QVERIFY(gitOutput({"diff", "--", m_sRelativeName}, sUnstaged));

        QVERIFY2(sUnstaged.contains("+line-B1"), qPrintable(sUnstaged));
    }

    // Hidden log panes are never read: refresh alone leaves log models empty,
    // each named refresh loads one domain while the others stay stale.
    // The working tree line may show from file status alone: it never counts
    // as a loaded log row.
    void logPane_loadsOnlyOnDemand()
    {
        QTest::qWait(3000);

        QVERIFY(m_pRepository->graphModel()->rowCount() <= 1);
        QCOMPARE(m_pRepository->refLogModel()->rowCount(), 0);

        if (m_pRepository->graphModel()->rowCount() == 1)
        {
            QModelIndex tIndex = m_pRepository->graphModel()->index(0, 0);
            QCOMPARE(m_pRepository->graphModel()->data(tIndex, CGraphModel::eIsWorkingTreeRole).toBool(), true);
        }

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        QCOMPARE(m_pRepository->refLogModel()->rowCount(), 0);

        m_pRepository->refreshRefLog();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->refLogModel()->rowCount() > 0, s_iAsyncTimeoutMs);

        // A refresh marks every domain stale without rereading, the next
        // named refresh reloads the domain
        m_pRepository->refresh();
        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);
    }

    // Graph filter hides every row on a nonsense string and restores them
    // once cleared, through the proxy like the tab view does
    void graphFilter_filtersRows()
    {
        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 0, s_iAsyncTimeoutMs);

        m_pRepository->setGraphFilter("zzz-no-such-commit-zzz");

        QTRY_COMPARE_WITH_TIMEOUT(m_pRepository->graphModelProxy()->rowCount(), 0, s_iAsyncTimeoutMs);

        m_pRepository->setGraphFilter("");

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModelProxy()->rowCount() > 0, s_iAsyncTimeoutMs);
    }

    // Ignored files stay out of the default refresh: the status batch
    // skips them and the models never inflate like the old crawl did.
    // They arrive through the on-demand batch and leave on purge
    void trackedSeed_defersIgnoredUntilRequested()
    {
        QDir tClone(m_sClonePath);

        QVERIFY(tClone.mkpath("ignored-seed"));

        QFile tJunk(tClone.filePath("ignored-seed/junk.bin"));
        QVERIFY(tJunk.open(QIODevice::WriteOnly));
        tJunk.write("junk");
        tJunk.close();

        QFile tPlain(tClone.filePath("plain.txt"));
        QVERIFY(tPlain.open(QIODevice::WriteOnly));
        tPlain.write("plain");
        tPlain.close();

        QFile tIgnore(tClone.filePath(".gitignore"));
        QVERIFY(tIgnore.open(QIODevice::WriteOnly | QIODevice::Text));
        tIgnore.write("ignored-seed/\n");
        tIgnore.close();

        m_pRepository->refresh();

        QString sIgnoredKey = m_sClonePath + "/ignored-seed/junk.bin";
        QString sPlainKey = m_sClonePath + "/plain.txt";

        // Plain untracked files still list one by one through the status
        // batch while ignored content stays out entirely
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->repoFiles().containsKey(sPlainKey), s_iAsyncTimeoutMs);
        QCOMPARE(m_pRepository->repoFiles().itemByKey(sPlainKey)->status(), CEnums::eUntracked);
        QTest::qWait(1000);
        QVERIFY(not m_pRepository->repoFiles().containsKey(sIgnoredKey));

        for (CRepoFile* pFile : m_pRepository->repoFiles())
        {
            QVERIFY2(not pFile->fullName().contains("/.git/"), qPrintable(pFile->fullName()));
            QVERIFY2(not pFile->fullName().contains("/ignored-seed/"), qPrintable(pFile->fullName()));
        }

        // The on-demand batch brings the ignored rows with their status
        m_pRepository->checkIgnoredFileStatus();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->repoFiles().containsKey(sIgnoredKey), s_iAsyncTimeoutMs);
        QCOMPARE(m_pRepository->repoFiles().itemByKey(sIgnoredKey)->status(), CEnums::eIgnored);

        // Purging drops them again without touching the other rows
        m_pRepository->purgeIgnoredFiles();

        QVERIFY(not m_pRepository->repoFiles().containsKey(sIgnoredKey));
        QVERIFY(m_pRepository->repoFiles().containsKey(sPlainKey));
    }

    // Labels survive on-demand loading whatever the arrival order: branch
    // data lands at refresh while lines load when the tab shows, each side
    // syncs the other so loaded rows end up labeled
    void graphLabels_syncOnLoad()
    {
        // Settle: capture the current fingerprint and let a full poll tick
        // pass, so no pending refresh relabels rows behind this test
        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->localBranchModel()->rowCount() > 0, s_iAsyncTimeoutMs);
        QTest::qWait(4500);

        // Load the log on demand like the tab view does. The wait below
        // stays under one poll interval: without a sync on load, no later
        // refresh can bring the labels in time
        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 0, s_iAsyncTimeoutMs);
        QTest::qWait(500);

        QVERIFY2(graphHasLabeledRow(), "graph rows arrived without labels");
    }

    // Batched branch reads fill every model with one process per domain:
    // head commits land on branches, tracking counts reach the status bar,
    // tag commits resolve annotated and lightweight tags
    void branchBatch_fillsHeadsAndAheadBehind()
    {        QTRY_VERIFY_WITH_TIMEOUT(not m_pRepository->currentBranch().isEmpty(), s_iAsyncTimeoutMs);

        QString sCurrent = m_pRepository->currentBranch();
        CBranchModel* pLocal = m_pRepository->localBranchModel();

        // Head commit of the checked out branch arrives from the batched for-each-ref
        QTRY_VERIFY_WITH_TIMEOUT(not branchCommitId(pLocal, sCurrent).isEmpty(), s_iAsyncTimeoutMs);

        // One commit ahead of origin shows up through the tracking parser
        QString sUnused;

        QVERIFY(gitOutput({"-c", "user.name=probe", "-c", "user.email=probe@local", "commit", "-qam", "batch-probe"}, sUnused));

        m_pRepository->refresh();

        QTRY_COMPARE_WITH_TIMEOUT(m_pRepository->commitCountAhead(), 1, s_iAsyncTimeoutMs);

        // Lightweight tags fall back to their object, annotated tags peel to their commit.
        // The annotated tag carries its own identity like the commit above:
        // clones never inherit user.name/user.email and CI runners have no
        // global git identity
        QVERIFY(gitOutput({"tag", "v-batch-light"}, sUnused));
        QVERIFY(gitOutput({"-c", "user.name=probe", "-c", "user.email=probe@local",
                           "tag", "-a", "-m", "probe", "v-batch-annotated"}, sUnused));

        m_pRepository->refresh();

        CBranchModel* pTags = m_pRepository->tagModel();

        QTRY_VERIFY_WITH_TIMEOUT(not branchCommitId(pTags, "v-batch-light").isEmpty(), s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(not branchCommitId(pTags, "v-batch-annotated").isEmpty(), s_iAsyncTimeoutMs);
    }

    // A push moves the remote tracking label onto the pushed commit: after
    // pushing, origin/<branch> points at the local head and the graph shows
    // it there instead of leaving it stuck on the previous commit
    void push_movesRemoteLabel()
    {
        QTemporaryDir tOriginDir;

        QVERIFY(tOriginDir.isValid());

        QString sOriginPath = tOriginDir.path() + "/origin.git";

        QCOMPARE(QProcess::execute("git", {"init", "--bare", "--quiet", sOriginPath}), 0);

        QString sUnused;

        {
            QString sIgnored;

            gitOutput({"remote", "remove", "origin"}, sIgnored);
        }

        QVERIFY(gitOutput({"remote", "add", "origin", sOriginPath}, sUnused));

        QString sCurrent;

        QTRY_VERIFY_WITH_TIMEOUT(not m_pRepository->currentBranch().isEmpty(), s_iAsyncTimeoutMs);

        sCurrent = m_pRepository->currentBranch();

        // Publish the current head so origin/<branch> exists, through the
        // same push path the QML action uses
        m_pRepository->push();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitCountAhead() == 0 || not m_pRepository->hasUpstreamBranch(), s_iAsyncTimeoutMs);

        // Wait until the remote tracking head matches the local head
        QTRY_VERIFY_WITH_TIMEOUT(branchCommitId(m_pRepository->remoteBranchModel(), "origin/" + sCurrent) == branchCommitId(m_pRepository->localBranchModel(), sCurrent) && not branchCommitId(m_pRepository->localBranchModel(), sCurrent).isEmpty(), s_iAsyncTimeoutMs);

        QVERIFY(gitOutput({"-c", "user.name=probe", "-c", "user.email=probe@local", "commit", "-qam", "push-probe"}, sUnused));

        QString sNewHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sNewHead));

        sNewHead = sNewHead.trimmed();

        QVERIFY(not sNewHead.isEmpty());

        m_pRepository->push();

        // The remote head follows the push: exact match, not a short prefix
        QTRY_VERIFY_WITH_TIMEOUT(branchCommitId(m_pRepository->remoteBranchModel(), "origin/" + sCurrent) == sNewHead, s_iAsyncTimeoutMs);

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        // The pushed commit carries the remote label, the previous one lost it
        QTRY_VERIFY_WITH_TIMEOUT(graphRowHasLabel(sNewHead, "origin/" + sCurrent), s_iAsyncTimeoutMs);
    }

    // A branch moved outside the application must move its label without a
    // graph reload: the reloaded branch heads refresh the two touched rows,
    // the stale line decorations must not resurrect the label on the old row
    void graphLabels_followMovedBranchWithoutReload()
    {
        QString sUnused;

        QVERIFY(gitOutput({"config", "user.name", "probe"}, sUnused));
        QVERIFY(gitOutput({"config", "user.email", "probe@local"}, sUnused));

        QVERIFY(gitOutput({"add", "-A"}, sUnused));
        QVERIFY(gitOutput({"commit", "-qm", "probe-label-base"}, sUnused));
        QVERIFY(gitOutput({"commit", "--allow-empty", "-qm", "probe-label-middle"}, sUnused));
        QVERIFY(gitOutput({"commit", "--allow-empty", "-qm", "probe-label-top"}, sUnused));

        QString sOldHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD~2"}, sOldHead));

        sOldHead = sOldHead.trimmed();

        QString sNewHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sNewHead));

        sNewHead = sNewHead.trimmed();

        QVERIFY(not sOldHead.isEmpty());
        QVERIFY(sOldHead != sNewHead);

        QVERIFY(gitOutput({"branch", "probe-label", sOldHead}, sUnused));

        m_pRepository->refresh();
        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(graphRowHasLabel(sOldHead, "probe-label"), s_iAsyncTimeoutMs);

        // Move the branch behind the graph back: only the status and branch
        // batches reload, the loaded rows keep their decorations
        QVERIFY(gitOutput({"update-ref", "refs/heads/probe-label", sNewHead}, sUnused));

        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(branchCommitId(m_pRepository->localBranchModel(), "probe-label") == sNewHead, s_iAsyncTimeoutMs);

        QTRY_VERIFY_WITH_TIMEOUT(graphRowHasLabel(sNewHead, "probe-label"), s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(not graphRowHasLabel(sOldHead, "probe-label"), s_iAsyncTimeoutMs);
    }

    // A tag push publishes its ref on the remote: the tag context menu action
    // goes through the same path and origin ends up carrying refs/tags/<name>
    void pushTag_publishesOnRemote()
    {
        QTemporaryDir tOriginDir;

        QVERIFY(tOriginDir.isValid());

        QString sOriginPath = tOriginDir.path() + "/origin.git";

        QCOMPARE(QProcess::execute("git", {"init", "--bare", "--quiet", sOriginPath}), 0);

        {
            QString sIgnored;

            gitOutput({"remote", "remove", "origin"}, sIgnored);
        }

        QString sUnused;

        QVERIFY(gitOutput({"remote", "add", "origin", sOriginPath}, sUnused));

        const QString sTagName = "v-push-probe";

        QVERIFY(gitOutput({"tag", sTagName}, sUnused));

        QString sExpectedCommit;

        QVERIFY(gitOutput({"rev-parse", sTagName}, sExpectedCommit));

        sExpectedCommit = sExpectedCommit.trimmed();

        QVERIFY(not sExpectedCommit.isEmpty());

        m_pRepository->pushTag(sTagName);

        QTRY_COMPARE_WITH_TIMEOUT(remoteRefCommit(sOriginPath, "refs/tags/" + sTagName),
                                  sExpectedCommit, s_iAsyncTimeoutMs);
    }

    // A local branch without upstream reports no upstream and no ahead count,
    // so the QML push action stays enabled through hasUpstreamBranch
    void branchWithoutUpstream_reportsMissingUpstream()
    {
        QString sUnused;

        QVERIFY(gitOutput({"checkout", "-b", "no-upstream-probe"}, sUnused));

        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->currentBranch() == "no-upstream-probe", s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(not m_pRepository->hasUpstreamBranch(), s_iAsyncTimeoutMs);
        QCOMPARE(m_pRepository->commitCountAhead(), 0);
    }

    // A branch without upstream tracking may still need a force push when the
    // remote branch exists under the same name: the action must stay enabled
    void forcePush_enabledWithoutUpstream()
    {
        QString sUnused;

        QVERIFY(gitOutput({"checkout", "-B", "force-push-probe"}, sUnused));

        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->currentBranch() == "force-push-probe", s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(not m_pRepository->hasUpstreamBranch(), s_iAsyncTimeoutMs);
        QVERIFY(m_pRepository->canForcePush());
    }

    // The working tree line shows on top of the graph when the tree is dirty,
    // with its parent on the current head and its message showing the diff state
    void workingTreeLine_showsOnTopWhenDirty()
    {
        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        CGraphModel* pModel = m_pRepository->graphModel();
        QModelIndex tGhostIndex = pModel->index(0, 0);

        QCOMPARE(pModel->data(tGhostIndex, CGraphModel::eCommitIdRole).toString(), m_pRepository->workingTreeCommitId());
        QCOMPARE(pModel->data(tGhostIndex, CGraphModel::eIsWorkingTreeRole).toBool(), true);

        const CLogLine* pGhostLine = pModel->findLine(m_pRepository->workingTreeCommitId());
        QVERIFY(pGhostLine != nullptr);

        const CGraphLine* pGhostGraphLine = qobject_cast<const CGraphLine*>(pGhostLine);
        QVERIFY(pGhostGraphLine != nullptr);
        QCOMPARE(pGhostGraphLine->parents().count(), 1);

        QModelIndex tHeadIndex = pModel->index(1, 0);
        QString sHeadId = pModel->data(tHeadIndex, CGraphModel::eCommitIdRole).toString();
        QVERIFY(not sHeadId.isEmpty());
        QCOMPARE(pGhostGraphLine->parents().first(), sHeadId);

        QTRY_VERIFY_WITH_TIMEOUT(pModel->data(pModel->index(0, 0), CGraphModel::eMessageRole).toString().startsWith("Changes: +"), s_iAsyncTimeoutMs);
    }

    // The working tree line stays on top when the tree is clean, with No changes
    void workingTreeLine_showsNoChangesWhenClean()
    {
        QString sUnused;

        QVERIFY(gitOutput({"checkout", "--", m_sRelativeName}, sUnused));

        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(not m_pRepository->hasModifiedFiles(), s_iAsyncTimeoutMs);

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 0, s_iAsyncTimeoutMs);

        CGraphModel* pModel = m_pRepository->graphModel();
        QModelIndex tGhostIndex = pModel->index(0, 0);

        QCOMPARE(pModel->data(tGhostIndex, CGraphModel::eCommitIdRole).toString(), m_pRepository->workingTreeCommitId());
        QCOMPARE(pModel->data(tGhostIndex, CGraphModel::eIsWorkingTreeRole).toBool(), true);
        QTRY_COMPARE_WITH_TIMEOUT(pModel->data(tGhostIndex, CGraphModel::eMessageRole).toString(), QString("No changes"), s_iAsyncTimeoutMs);
    }

    // A working tree holding only untracked files reports them explicitly:
    // no diff ever shows untracked content, so the file statuses are the only
    // source for this count and the line must never stay on a blank state
    void workingTreeLine_reportsUntrackedFiles()
    {
        QString sUnused;

        QVERIFY(gitOutput({"checkout", "--", m_sRelativeName}, sUnused));
        QVERIFY(createUntrackedFile("untracked-probe.txt", "untracked content\n"));

        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->hasModifiedFiles(), s_iAsyncTimeoutMs);

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        CGraphModel* pModel = m_pRepository->graphModel();
        QModelIndex tGhostIndex = pModel->index(0, 0);

        QCOMPARE(pModel->data(tGhostIndex, CGraphModel::eCommitIdRole).toString(), m_pRepository->workingTreeCommitId());
        QTRY_COMPARE_WITH_TIMEOUT(pModel->data(tGhostIndex, CGraphModel::eMessageRole).toString(),
                                  QString("Changes: 1 untracked file(s)"), s_iAsyncTimeoutMs);
    }

    // Switching from a commit diff to the working tree line replaces the
    // counters at once: the shared file diff model still holds the commit
    // diff, and reading it blindly reports that commit's numbers for the tree
    void workingTreeLine_doesNotInheritCommitDiffCounters()
    {
        QString sUnused;

        QVERIFY(gitOutput({"checkout", "--", m_sRelativeName}, sUnused));
        QVERIFY(createUntrackedFile("untracked-probe.txt", "untracked content\n"));

        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->hasModifiedFiles(), s_iAsyncTimeoutMs);

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        CGraphModel* pModel = m_pRepository->graphModel();
        QString sHeadId = pModel->data(pModel->index(1, 0), CGraphModel::eCommitIdRole).toString();

        QVERIFY(not sHeadId.isEmpty());

        // The root commit adds every line of the fixture: its counters are
        // never zero, so any leak into the ghost line is visible
        m_pRepository->commitDiffPrevious(sHeadId);

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsAddedCount() > 0, s_iAsyncTimeoutMs);
        QCOMPARE(m_pRepository->commitDetailsId(), sHeadId);

        m_pRepository->commitDiffPrevious(m_pRepository->workingTreeCommitId());

        QCOMPARE(m_pRepository->commitDetailsId(), m_pRepository->workingTreeCommitId());
        QCOMPARE(m_pRepository->commitDetailsAddedCount(), 0);
        QCOMPARE(m_pRepository->commitDetailsRemovedCount(), 0);

        // The pending working tree diff lands with no line: the counters stay zero
        QTRY_COMPARE_WITH_TIMEOUT(m_pRepository->fileDiffModel()->rowCount(), 0, s_iAsyncTimeoutMs);
        QCOMPARE(m_pRepository->commitDetailsAddedCount(), 0);
        QCOMPARE(m_pRepository->commitDetailsRemovedCount(), 0);
    }

    // The working tree diff shows staged and unstaged changes together,
    // like the tree state the working tree line stands for
    void workingTreeDiff_showsStagedUnstaged()
    {
        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        m_pRepository->commitDiffPrevious(m_pRepository->workingTreeCommitId());

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->fileDiffModel()->rowCount() > 0, s_iAsyncTimeoutMs);
        QCOMPARE(m_pRepository->commitDetailsId(), m_pRepository->workingTreeCommitId());
        QVERIFY(modelContains(m_pRepository->fileDiffModel(), "line-B1"));
        QVERIFY(m_pRepository->commitDetailsAddedCount() > 0);

        // Stage one line: the working tree diff keeps showing it together
        // with the remaining unstaged lines
        selectFileAndWaitForDiff();

        CDiffModel* pUnstaged = m_pRepository->unstagedFileDiffModel();
        QList<int> lAdded = addedSourceRows(pUnstaged, "line-B1");

        QCOMPARE(lAdded.count(), 1);

        m_pRepository->stageDiffLines(proxyRows(pUnstaged, lAdded));
        m_pRepository->commitDiffPrevious(m_pRepository->workingTreeCommitId());

        QTRY_VERIFY_WITH_TIMEOUT(modelContains(m_pRepository->fileDiffModel(), "line-B1"), s_iAsyncTimeoutMs);
        QVERIFY(modelContains(m_pRepository->fileDiffModel(), "line-B2"));
    }

    // A shrinking history (squash, rebase, reset) drops dead rows on refresh:
    // without the tail trim the graph keeps unknown revisions and the commit
    // diff fails with "revision inconnue" when one is selected
    void graphRefresh_shrinkingHistory_dropsStaleRows()
    {
        CGraphModel tModel(nullptr);

        tModel.setWorkingTreePresent(true);

        QCOMPARE(tModel.rowCount(), 1);

        CGraphLineCollection tFull;
        tFull.setStartIndex(0);
        tFull.setPotentialCount(4);
        tFull.add(makeGraphLine("commit-A", QStringList() << "commit-B", "feat A"));
        tFull.add(makeGraphLine("commit-B", QStringList() << "commit-C", "feat B"));
        tFull.add(makeGraphLine("commit-C", QStringList() << "commit-D", "feat C"));
        tFull.add(makeGraphLine("commit-D", QStringList(), "feat D"));

        tModel.setLines(tFull);

        QCOMPARE(tModel.rowCount(), 5);
        QVERIFY(tModel.findLine("commit-D") != nullptr);

        CGraphLineCollection tShrunk;
        tShrunk.setStartIndex(0);
        tShrunk.setPotentialCount(2);
        tShrunk.add(makeGraphLine("squashed-A", QStringList() << "squashed-B", "squashed"));
        tShrunk.add(makeGraphLine("squashed-B", QStringList(), "feat D"));

        tModel.setLines(tShrunk);

        QCOMPARE(tModel.rowCount(), 3);
        QVERIFY(tModel.findLine("commit-A") == nullptr);
        QVERIFY(tModel.findLine("commit-B") == nullptr);
        QVERIFY(tModel.findLine("commit-C") == nullptr);
        QVERIFY(tModel.findLine("squashed-A") != nullptr);
        QVERIFY(tModel.findLine("squashed-B") != nullptr);
        QCOMPARE(tModel.data(tModel.index(0, 0), CGraphModel::eIsWorkingTreeRole).toBool(), true);
    }

    // An all-duplicate refresh on a dirty repository must keep the loaded
    // rows instead of trimming them down to the working tree line alone
    void graphRefresh_allDuplicateRefresh_preservesLoadedRows()
    {
        CGraphModel tModel(nullptr);

        tModel.setWorkingTreePresent(true);

        QCOMPARE(tModel.rowCount(), 1);

        CGraphLineCollection tFull;
        tFull.setStartIndex(0);
        tFull.setPotentialCount(4);
        tFull.add(makeGraphLine("commit-A", QStringList() << "commit-B", "feat A"));
        tFull.add(makeGraphLine("commit-B", QStringList() << "commit-C", "feat B"));
        tFull.add(makeGraphLine("commit-C", QStringList() << "commit-D", "feat C"));
        tFull.add(makeGraphLine("commit-D", QStringList(), "feat D"));

        tModel.setLines(tFull);

        QCOMPARE(tModel.rowCount(), 5);
        QVERIFY(tModel.findLine("commit-D") != nullptr);

        // Repeat the identical page: every incoming row is a duplicate of an
        // already loaded row, so the filtered incoming count is zero
        CGraphLineCollection tDuplicate;
        tDuplicate.setStartIndex(0);
        tDuplicate.setPotentialCount(4);
        tDuplicate.add(makeGraphLine("commit-A", QStringList() << "commit-B", "feat A"));
        tDuplicate.add(makeGraphLine("commit-B", QStringList() << "commit-C", "feat B"));
        tDuplicate.add(makeGraphLine("commit-C", QStringList() << "commit-D", "feat C"));
        tDuplicate.add(makeGraphLine("commit-D", QStringList(), "feat D"));

        tModel.setLines(tDuplicate);

        QCOMPARE(tModel.rowCount(), 5);
        QVERIFY(tModel.findLine("commit-A") != nullptr);
        QVERIFY(tModel.findLine("commit-D") != nullptr);
        QCOMPARE(tModel.data(tModel.index(0, 0), CGraphModel::eIsWorkingTreeRole).toBool(), true);
    }

    // An identical refresh must not re-emit dataChanged for unchanged rows:
    // redundant emissions are the flicker source during the first display
    void graphRefresh_identicalRefresh_emitsNoDataChanged()
    {
        CGraphModel tModel(nullptr);

        tModel.setWorkingTreePresent(true);

        CGraphLineCollection tFull;
        tFull.setStartIndex(0);
        tFull.setPotentialCount(4);
        tFull.add(makeGraphLine("commit-A", QStringList() << "commit-B", "feat A"));
        tFull.add(makeGraphLine("commit-B", QStringList() << "commit-C", "feat B"));
        tFull.add(makeGraphLine("commit-C", QStringList() << "commit-D", "feat C"));
        tFull.add(makeGraphLine("commit-D", QStringList(), "feat D"));

        tModel.setLines(tFull);

        CGraphLineCollection tDuplicate;
        tDuplicate.setStartIndex(0);
        tDuplicate.setPotentialCount(4);
        tDuplicate.add(makeGraphLine("commit-A", QStringList() << "commit-B", "feat A"));
        tDuplicate.add(makeGraphLine("commit-B", QStringList() << "commit-C", "feat B"));
        tDuplicate.add(makeGraphLine("commit-C", QStringList() << "commit-D", "feat C"));
        tDuplicate.add(makeGraphLine("commit-D", QStringList(), "feat D"));

        QSignalSpy tSpy(&tModel, &CGraphModel::dataChanged);

        tModel.setLines(tDuplicate);

        QCOMPARE(tSpy.count(), 0);
    }

    // A pushed main rewritten locally (reword of an old pushed commit) then
    // scrolled down without an intermediate refresh: the appended page is
    // read from the shifted history on top of the stale first page and must
    // neither duplicate rows nor cut lanes
    void graphDivergedHistory_scrollingDown_keepsLanesContinuous()
    {
        // Dedicated pushed main with dozens of commits: the bare origin holds
        // the published history, the clone below makes it diverge like after
        // a series of change-message on pushed commits. No server needed, the
        // origin is a local bare repository cloned through a file path
        QTemporaryDir tSourceDir;
        QVERIFY(tSourceDir.isValid());
        QTemporaryDir tOriginDir;
        QVERIFY(tOriginDir.isValid());
        QTemporaryDir tWorkDir;
        QVERIFY(tWorkDir.isValid());

        QString sUnused;

        QVERIFY(gitInDir(tSourceDir.path(), {"init", "-qb", "main"}, sUnused));
        QVERIFY(gitInDir(tSourceDir.path(), {"config", "user.name", "probe"}, sUnused));
        QVERIFY(gitInDir(tSourceDir.path(), {"config", "user.email", "probe@local"}, sUnused));

        QFile tFile(tSourceDir.path() + "/probe.txt");

        QVERIFY(tFile.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream tStream(&tFile);

        const int iCommitCount = 55;

        // Fixed past committer dates keep the published chain strictly older
        // than the rewritten one, with no same-second ties to interleave them
        for (int iCommit = 0; iCommit < iCommitCount; iCommit++)
        {
            tStream << "probe-line-" << iCommit << "\n";
            tStream.flush();

            QVERIFY(gitInDir(tSourceDir.path(), {"add", "probe.txt"}, sUnused));

            QString sDate = QString("@%1 +0100").arg(1700000000 + iCommit);

            QVERIFY(gitInDirWithDate(tSourceDir.path(),
                                     {"commit", "-qm", QString("probe-commit-%1").arg(iCommit)},
                                     sDate, sUnused));
        }

        tFile.close();

        QVERIFY(gitInDir(tOriginDir.path(), {"init", "--quiet", "--bare"}, sUnused));
        QVERIFY(gitInDir(tOriginDir.path(), {"symbolic-ref", "HEAD", "refs/heads/main"}, sUnused));
        QVERIFY(gitInDir(tSourceDir.path(), {"remote", "add", "origin", tOriginDir.path()}, sUnused));
        QVERIFY(gitInDir(tSourceDir.path(), {"push", "--quiet", "-u", "origin", "main"}, sUnused));

        QString sClonePath = tWorkDir.path() + "/clone";

        QCOMPARE(QProcess::execute("git", {"clone", "--quiet", tOriginDir.path(), sClonePath}), 0);
        QVERIFY(gitInDir(sClonePath, {"config", "user.name", "probe"}, sUnused));
        QVERIFY(gitInDir(sClonePath, {"config", "user.email", "probe@local"}, sUnused));

        QScopedPointer<CRepository> pRepo(new CRepository(sClonePath, nullptr));
        pRepo->refresh();

        CGraphModel* pModel = pRepo->graphModel();

        pRepo->refreshGraph();

        // First page only: the file status always prepends its working tree
        // line, so 50 git rows plus one working tree row. The remaining rows
        // load through fetchMore below, like the view scrolling down
        QTRY_COMPARE_WITH_TIMEOUT(pModel->rowCount(), 51, s_iAsyncTimeoutMs);

        QString sBaselineCut = checkGraphLanesContinuous(pModel);

        QVERIFY2(sBaselineCut.isEmpty(), qPrintable(sBaselineCut));

        QString sOldHead;

        QVERIFY(gitInDir(sClonePath, {"rev-parse", "HEAD"}, sOldHead));

        sOldHead = sOldHead.trimmed();

        QString sOriginHead;

        QVERIFY(gitInDir(sClonePath, {"rev-parse", "origin/main"}, sOriginHead));

        sOriginHead = sOriginHead.trimmed();

        QCOMPARE(sOriginHead, sOldHead);

        // Reword an old pushed commit outside the application: the local main
        // deviates from origin/main and every descendant hash shifts, like a
        // series of change-message on pushed commits. Fake editors, hermetic,
        // no app slave binary needed
        QTemporaryDir tEditorDir;

        QVERIFY(tEditorDir.isValid());

        QString sSeqPath = tEditorDir.path() + "/seq-editor.sh";
        QFile tSeq(sSeqPath);

        QVERIFY(tSeq.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream(&tSeq) << "#!/bin/sh\nsed -i '1s/^pick /reword /' \"$1\"\n";
        tSeq.close();

        QVERIFY(QFile::setPermissions(sSeqPath,
                                      QFile::permissions(sSeqPath)
                                      | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther));

        QString sMsgPath = tEditorDir.path() + "/msg-editor.sh";
        QFile tMsg(sMsgPath);

        QVERIFY(tMsg.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream(&tMsg) << "#!/bin/sh\nprintf '%s\\n' 'Reworded old probe commit' > \"$1\"\n";
        tMsg.close();

        QVERIFY(QFile::setPermissions(sMsgPath,
                                      QFile::permissions(sMsgPath)
                                      | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther));

        QVERIFY(gitInDir(sClonePath, {"-c", "sequence.editor=" + sSeqPath,
                                      "-c", "core.editor=" + sMsgPath,
                                      "rebase", "-i", "HEAD~41"}, sUnused));

        QString sNewHead;

        QVERIFY(gitInDir(sClonePath, {"rev-parse", "HEAD"}, sNewHead));

        sNewHead = sNewHead.trimmed();

        QVERIFY(not sNewHead.isEmpty());
        QVERIFY(sNewHead != sOldHead);

        QString sOriginAfter;

        QVERIFY(gitInDir(sClonePath, {"rev-parse", "origin/main"}, sOriginAfter));

        QCOMPARE(sOriginAfter.trimmed(), sOldHead);

        // Scroll down without refreshing first: the next pages are read from
        // the shifted history on top of the stale first page
        scrollGraphToQuiescence(pModel);

        QString sCut = checkGraphLanesContinuous(pModel);

        QVERIFY2(sCut.isEmpty(),
                 qPrintable(QString("%1 (rows=%2 potential=%3)")
                            .arg(sCut).arg(pModel->rowCount()).arg(pModel->potentialCount())));

        // A refresh heals the stale base: both the rewritten branch and the
        // published one load fully, and their lanes converge without cuts
        pRepo->refresh();
        pRepo->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(pModel->findLine(sNewHead) != nullptr, s_iAsyncTimeoutMs);

        scrollGraphToQuiescence(pModel);

        QString sHealedCut = checkGraphLanesContinuous(pModel);

        QVERIFY2(sHealedCut.isEmpty(),
                 qPrintable(QString("%1 (rows=%2 potential=%3)")
                            .arg(sHealedCut).arg(pModel->rowCount()).arg(pModel->potentialCount())));
    }

    // Rewording a commit rewrites its hash and every descendant hash: the
    // details pane must follow the new commit instead of going empty on the
    // dead identifier
    void changeMessage_detailsFollowNewHash()
    {
        QString sUnused;

        QVERIFY(gitOutput({"config", "user.name", "probe"}, sUnused));
        QVERIFY(gitOutput({"config", "user.email", "probe@local"}, sUnused));

        // The dirty worktree blocks the rebase, like the disabled menu entry:
        // commit it so the tree is clean
        QVERIFY(gitOutput({"add", "-A"}, sUnused));
        QVERIFY(gitOutput({"commit", "-qm", "probe-top-commit"}, sUnused));

        QString sOldHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sOldHead));

        sOldHead = sOldHead.trimmed();

        QVERIFY(not sOldHead.isEmpty());

        // Fake editors: the todo pick becomes a reword, the message file gets
        // the new subject. Hermetic, no app slave binary needed.
        QString sNewMessage = "Reworded probe commit subject";

        QTemporaryDir tEditorDir;

        QVERIFY(tEditorDir.isValid());

        QString sScriptPath = tEditorDir.path() + "/fake-editor.sh";
        QFile tScript(sScriptPath);

        QVERIFY(tScript.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream(&tScript) << "#!/bin/sh\n"
                              << "if grep -q '^pick ' \"$1\"; then\n"
                              << "  sed -i 's/^pick /reword /' \"$1\"\n"
                              << "else\n"
                              << "  printf '%s\\n' '" << sNewMessage << "' > \"$1\"\n"
                              << "fi\n";
        tScript.close();

        QVERIFY(QFile::setPermissions(sScriptPath,
                                      QFile::permissions(sScriptPath)
                                      | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther));

        CController tController(sScriptPath);
        QScopedPointer<CRepository> pRepo(new CRepository(m_sClonePath, &tController));

        pRepo->refresh();
        pRepo->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(pRepo->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        pRepo->commitDiffPrevious(sOldHead);

        QTRY_COMPARE_WITH_TIMEOUT(pRepo->commitDetailsId(), sOldHead, s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(pRepo->commitDetailsMessage().contains("probe-top-commit"),
                                 s_iAsyncTimeoutMs);

        pRepo->changeCommitMessage(sOldHead, sNewMessage);

        // The sequencer detaches HEAD while running: only a moved branch with
        // no rebase directory left proves the rewrite finished
        QTRY_VERIFY_WITH_TIMEOUT(rebaseFinished(sOldHead), s_iAsyncTimeoutMs);

        QString sNewHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sNewHead));

        sNewHead = sNewHead.trimmed();

        QVERIFY(not sNewHead.isEmpty());
        QVERIFY(sNewHead != sOldHead);

        // Mark the graph domain stale so the reload actually runs: the rebase
        // happened outside the application, so nothing marked it stale.
        pRepo->refresh();
        pRepo->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(pRepo->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        // The graph reload is asynchronous: the details pane follows the
        // rewritten commit only once adoptRewordedCommit() runs against the
        // freshly loaded rows. Re-run it explicitly until it adopts, instead
        // of relying on a single implicit update that may race the reload.
        QTRY_VERIFY_WITH_TIMEOUT(
            ([&]() {
                pRepo->refreshCommitDetails();
                return pRepo->commitDetailsId() == sNewHead;
            })(),
            s_iAsyncTimeoutMs);

        QTRY_COMPARE_WITH_TIMEOUT(pRepo->commitDetailsMessage().trimmed(), sNewMessage,
                                  s_iAsyncTimeoutMs);
    }

    // Amending outside the application rewrites the hash with no pending
    // tracking: after the poll refresh and a click on the reloaded row, the
    // details pane must show the new commit instead of an empty pane that
    // keeps the old added and removed counts
    void externalAmend_detailsFollowNewHash()
    {
        QString sUnused;

        QVERIFY(gitOutput({"config", "user.name", "probe"}, sUnused));
        QVERIFY(gitOutput({"config", "user.email", "probe@local"}, sUnused));

        // A clean tree keeps the graph free of a working tree line, like the
        // reported screenshots
        QVERIFY(gitOutput({"add", "-A"}, sUnused));
        QVERIFY(gitOutput({"commit", "-qm", "probe-top-commit"}, sUnused));

        QString sOldHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sOldHead));

        sOldHead = sOldHead.trimmed();

        QVERIFY(not sOldHead.isEmpty());

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        // Baseline: details shown on the old head, like a click before the amend
        m_pRepository->commitDiffPrevious(sOldHead);

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsAuthor() == "probe",
                                 s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsMessage().contains("probe-top-commit"),
                                 s_iAsyncTimeoutMs);

        // External amend in a terminal: same tree, new message, new hash
        QString sNewMessage = "Amended probe commit subject";

        QVERIFY(gitOutput({"commit", "--amend", "-qm", sNewMessage}, sUnused));

        QString sNewHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sNewHead));

        sNewHead = sNewHead.trimmed();

        QVERIFY(not sNewHead.isEmpty());
        QVERIFY(sNewHead != sOldHead);

        // Poll-style refresh, like the external change polling does
        m_pRepository->refresh();
        m_pRepository->refreshGraph();

        // Wait until the reloaded graph carries the new head
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->findLine(sNewHead) != nullptr,
                                 s_iAsyncTimeoutMs);

        // Click on the reloaded row, like checking the amended commit
        m_pRepository->commitDiffPrevious(sNewHead);

        QTRY_COMPARE_WITH_TIMEOUT(m_pRepository->commitDetailsId(), sNewHead, s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsAuthor() == "probe",
                                 s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsMessage().trimmed() == sNewMessage,
                                 s_iAsyncTimeoutMs);
    }

    // Same external amend, but the graph view is hidden: no reload and no
    // click happen at amend time. When the graph finally reloads, the shown
    // details identifier is dead: the details must follow the rewritten
    // commit instead of going empty
    void externalAmend_hiddenGraph_detailsFollowNewHash()
    {
        QString sUnused;

        QVERIFY(gitOutput({"config", "user.name", "probe"}, sUnused));
        QVERIFY(gitOutput({"config", "user.email", "probe@local"}, sUnused));

        QVERIFY(gitOutput({"add", "-A"}, sUnused));
        QVERIFY(gitOutput({"commit", "-qm", "probe-top-commit"}, sUnused));

        QString sOldHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sOldHead));

        sOldHead = sOldHead.trimmed();

        QVERIFY(not sOldHead.isEmpty());

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 1, s_iAsyncTimeoutMs);

        // Baseline: details shown on the old head, like a click before the amend
        m_pRepository->commitDiffPrevious(sOldHead);

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsAuthor() == "probe",
                                 s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsMessage().contains("probe-top-commit"),
                                 s_iAsyncTimeoutMs);

        QString sNewMessage = "Amended probe commit subject";

        QVERIFY(gitOutput({"commit", "--amend", "-qm", sNewMessage}, sUnused));

        QString sNewHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sNewHead));

        sNewHead = sNewHead.trimmed();

        QVERIFY(not sNewHead.isEmpty());
        QVERIFY(sNewHead != sOldHead);

        // The graph view is hidden: the poll marks the domain stale but
        // nothing reloads yet, then the user shows the graph again
        m_pRepository->refresh();
        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->findLine(sNewHead) != nullptr,
                                 s_iAsyncTimeoutMs);

        // No click: the details must still follow the rewritten commit
        QTRY_COMPARE_WITH_TIMEOUT(m_pRepository->commitDetailsId(), sNewHead, s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsAuthor() == "probe",
                                 s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->commitDetailsMessage().trimmed() == sNewMessage,
                                 s_iAsyncTimeoutMs);
    }

    // Linked worktree heads carry no branch or tag decoration, so their lane
    // would show with no label: the graph marks them with a worktree label
    // carrying the worktree folder name
    void worktreeHead_showsWorktreeLabel()
    {
        QString sUnused;
        QString sOldHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sOldHead));

        sOldHead = sOldHead.trimmed();

        QVERIFY(not sOldHead.isEmpty());

        // Move HEAD forward so the worktree commit keeps no branch label
        QVERIFY(gitOutput({"-c", "user.name=probe", "-c", "user.email=probe@local",
                           "commit", "--allow-empty", "-qam", "worktree-probe"}, sUnused));

        // The clone keeps origin/main on the old head: drop the remote so the
        // worktree commit carries no decoration at all, like a void lane tip
        {
            QString sIgnored;

            gitOutput({"remote", "remove", "origin"}, sIgnored);
        }

        QString sWorktreePath = m_pWorkDir->path() + "/probe-wt";

        QVERIFY(gitOutput({"worktree", "add", "--detach", sWorktreePath, sOldHead}, sUnused));

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(graphRowHasWorktreeLabel(sOldHead, "probe-wt"), s_iAsyncTimeoutMs);

        QVERIFY(gitOutput({"worktree", "remove", "--force", sWorktreePath}, sUnused));
    }

    // Porcelain worktree listing maps head commits to folder names, skipping
    // bare repositories while keeping prunable entries labeled
    void worktreeHeads_parsesPorcelain()
    {
        QString sOutput =
            "worktree /tmp/main\n"
            "HEAD aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
            "branch refs/heads/main\n"
            "\n"
            "worktree /tmp/opencode/baseline\n"
            "HEAD bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n"
            "detached\n"
            "\n"
            "worktree /tmp/bare.git\n"
            "HEAD cccccccccccccccccccccccccccccccccccccccc\n"
            "bare\n"
            "\n"
            "worktree /tmp/gone\n"
            "HEAD dddddddddddddddddddddddddddddddddddddddd\n"
            "detached\n"
            "prunable gitdir file points to non-existent location\n";

        QMap<QString, QStringList> hHeads = CGitCommands::parseWorktreeHeads(sOutput);

        QCOMPARE(hHeads.count(), 3);
        QCOMPARE(hHeads.value("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), QStringList() << "main");
        QCOMPARE(hHeads.value("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"), QStringList() << "baseline");
        QCOMPARE(hHeads.value("dddddddddddddddddddddddddddddddddddddddd"), QStringList() << "gone");
        QVERIFY(not hHeads.contains("cccccccccccccccccccccccccccccccccccccccc"));
    }

    // The graph menu checkout leaves a detached HEAD: the current branch
    // reports the detached description while no fake branch entry enters the
    // branch list, and switching back restores the attached branch
    void commitCheckout_detachesHead()
    {
        m_pRepository->refresh();

        QTRY_VERIFY_WITH_TIMEOUT(not m_pRepository->currentBranch().isEmpty(), s_iAsyncTimeoutMs);

        QString sBranch = m_pRepository->currentBranch();
        int iBranchCount = m_pRepository->localBranchModel()->rowCount();
        QString sHead;

        QVERIFY(gitOutput({"rev-parse", "HEAD"}, sHead));

        m_pRepository->commitCheckout(sHead.trimmed());

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->currentBranch().startsWith("("), s_iAsyncTimeoutMs);
        QTRY_COMPARE_WITH_TIMEOUT(m_pRepository->localBranchModel()->rowCount(), iBranchCount, s_iAsyncTimeoutMs);

        m_pRepository->setCurrentBranch(sBranch);

        QTRY_COMPARE_WITH_TIMEOUT(m_pRepository->currentBranch(), sBranch, s_iAsyncTimeoutMs);
    }

 private:

    // Runs git in an arbitrary folder and returns standard output
    static bool gitInDir(const QString& sWorkDir, const QStringList& lArguments, QString& sOutput)
    {
        QProcess tProcess;

        tProcess.setWorkingDirectory(sWorkDir);
        tProcess.start("git", lArguments);

        if (not tProcess.waitForFinished(10000))
            return false;

        if (tProcess.exitCode() != 0)
            return false;

        sOutput = QString::fromUtf8(tProcess.readAllStandardOutput());

        return true;
    }

    // Runs git with fixed author and committer dates, for deterministic history order
    static bool gitInDirWithDate(const QString& sWorkDir, const QStringList& lArguments, const QString& sDate, QString& sOutput)
    {
        QProcess tProcess;

        tProcess.setWorkingDirectory(sWorkDir);

        QProcessEnvironment tEnvironment = QProcessEnvironment::systemEnvironment();
        tEnvironment.insert("GIT_AUTHOR_DATE", sDate);
        tEnvironment.insert("GIT_COMMITTER_DATE", sDate);
        tProcess.setProcessEnvironment(tEnvironment);

        tProcess.start("git", lArguments);

        if (not tProcess.waitForFinished(10000))
            return false;

        if (tProcess.exitCode() != 0)
            return false;

        sOutput = QString::fromUtf8(tProcess.readAllStandardOutput());

        return true;
    }

    // True when a row draws a lane segment open toward the checked side
    static bool rowCarriesLane(CGraphModel* pModel, int iRow, int iLane, bool bFromAbove)
    {
        QVariantList lVerticals = pModel->data(pModel->index(iRow, 0),
                                               CGraphModel::eGraphVerticalsRole).toList();

        for (const QVariant& vVertical : lVerticals)
        {
            QVariantList lVertical = vVertical.toList();

            if (lVertical.count() != 3 || lVertical[0].toInt() != iLane)
                continue;

            bool bOpen = bFromAbove ? not lVertical[2].toBool() : not lVertical[1].toBool();

            if (bOpen)
                return true;
        }

        return false;
    }

    // Returns an empty string when every lane of the loaded graph rows is
    // continuous, otherwise a description of the first cut found. A lane
    // going past a row must reappear in the next one, a lane coming from
    // above must come from the previous one, and a lane stopping mid-list
    // needs a merge join: only root rows may end a lane bare. The last row
    // may dangle while the model reports more rows to load. Duplicate
    // commit rows always count as corruption
    static QString checkGraphLanesContinuous(CGraphModel* pModel)
    {
        int iRows = pModel->rowCount();
        QSet<QString> sSeenIds;

        for (int iRow = 0; iRow < iRows; iRow++)
        {
            QString sCommitId = pModel->data(pModel->index(iRow, 0),
                                             CGraphModel::eCommitIdRole).toString();

            if (sCommitId.isEmpty())
                return QString("row %1 has no commit id").arg(iRow);

            if (sSeenIds.contains(sCommitId))
                return QString("row %1 duplicates commit %2").arg(iRow).arg(sCommitId);

            sSeenIds << sCommitId;
        }

        for (int iRow = 0; iRow < iRows; iRow++)
        {
            QModelIndex tIndex = pModel->index(iRow, 0);
            QVariantList lVerticals = pModel->data(tIndex, CGraphModel::eGraphVerticalsRole).toList();
            QVariantList lConnections = pModel->data(tIndex, CGraphModel::eGraphConnectionsRole).toList();

            for (const QVariant& vVertical : lVerticals)
            {
                QVariantList lVertical = vVertical.toList();

                if (lVertical.count() != 3)
                    return QString("row %1 has a malformed lane segment").arg(iRow);

                int iLane = lVertical[0].toInt();
                bool bTop = lVertical[1].toBool();
                bool bBottom = lVertical[2].toBool();

                if (not bBottom)
                {
                    // The lane runs past this row: the next row must carry it
                    if (iRow + 1 >= iRows)
                    {
                        if (pModel->canFetchMore(QModelIndex()))
                            continue;

                        return QString("row %1 leaves lane %2 hanging past the last row")
                            .arg(iRow).arg(iLane);
                    }

                    if (not rowCarriesLane(pModel, iRow + 1, iLane, false))
                    {
                        return QString("row %1 cuts lane %2: missing below").arg(iRow).arg(iLane);
                    }
                }

                if (not bTop && iRow > 0)
                {
                    // The lane comes from above: the previous row must carry it
                    if (not rowCarriesLane(pModel, iRow - 1, iLane, true))
                    {
                        return QString("row %1 cuts lane %2: missing above").arg(iRow).arg(iLane);
                    }
                }

                if (bBottom)
                {
                    bool bJoined = false;

                    for (const QVariant& vConnection : lConnections)
                    {
                        QVariantList lConnection = vConnection.toList();

                        if (lConnection.count() != 3)
                            continue;

                        if (lConnection[0].toInt() == iLane || lConnection[1].toInt() == iLane
                                || lConnection[2].toInt() == iLane)
                        {
                            bJoined = true;
                            break;
                        }
                    }

                    if (not bJoined)
                    {
                        const CLogLine* pLine = pModel->lineAt(iRow);
                        const CGraphLine* pGraphLine = qobject_cast<const CGraphLine*>(pLine);

                        if (pGraphLine == nullptr)
                        {
                            return QString("row %1 stops lane %2 on a non-graph line")
                                .arg(iRow).arg(iLane);
                        }

                        if (not pGraphLine->parents().isEmpty())
                        {
                            return QString("row %1 stops lane %2 mid-history").arg(iRow).arg(iLane);
                        }
                    }
                }
            }
        }

        return QString();
    }

    // Scrolls the graph model down like the view does, until no more rows
    // arrive: a page may hold nothing but rows already loaded
    static void scrollGraphToQuiescence(CGraphModel* pModel)
    {
        QElapsedTimer tScroll;
        tScroll.start();

        int iFetches = 0;

        while (pModel->canFetchMore(QModelIndex()) && iFetches < 10)
        {
            QVERIFY2(tScroll.elapsed() < s_iAsyncTimeoutMs, "scrolling down stalled");
            iFetches++;

            int iBefore = pModel->rowCount();
            pModel->fetchMore(QModelIndex());

            bool bProgress = false;
            QElapsedTimer tPage;
            tPage.start();

            while (tPage.elapsed() < 10000)
            {
                if (pModel->rowCount() != iBefore)
                {
                    bProgress = true;
                    break;
                }

                QTest::qWait(50);
            }

            if (not bProgress)
                break;
        }
    }

    // Builds an owned graph line for the synthetic shrinking-history test
    static CGraphLine* makeGraphLine(const QString& sId, const QStringList& lParents, const QString& sMessage)
    {
        CGraphLine* pLine = new CGraphLine();

        pLine->setCommitId(sId);
        pLine->setParents(lParents);
        pLine->setDecorations(QStringList());
        pLine->setAuthor("tester");
        pLine->setDate(QDateTime::fromString("2026-10-31T09:00:00+01:00", Qt::ISODate));
        pLine->setMessage(sMessage);
        pLine->setMessageIsComplete(true);

        return pLine;
    }

    // True when the graph row for a commit carries a label with the name
    static bool graphRowHasLabelStatic(CGraphModel* pModel, const QString& sCommitId, const QString& sLabelName)
    {
        for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
        {
            QModelIndex tIndex = pModel->index(iRow, 0);

            if (pModel->data(tIndex, CGraphModel::eCommitIdRole).toString() != sCommitId)
                continue;

            QVariant vLabels = pModel->data(tIndex, CGraphModel::eLabelsRole);
            CLabelModel* pLabels = vLabels.value<CLabelModel*>();

            if (pLabels == nullptr)
                return false;

            for (int iLabel = 0; iLabel < pLabels->rowCount(); iLabel++)
            {
                if (pLabels->data(pLabels->index(iLabel, 0), CLabelModel::eNameRole).toString() == sLabelName)
                    return true;
            }

            return false;
        }

        return false;
    }

    bool graphRowHasLabel(const QString& sCommitId, const QString& sLabelName) const
    {
        return graphRowHasLabelStatic(m_pRepository->graphModel(), sCommitId, sLabelName);
    }

    // True when the graph row for a commit carries a worktree label with the name
    static bool graphRowHasWorktreeLabelStatic(CGraphModel* pModel, const QString& sCommitId, const QString& sLabelName)
    {
        for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
        {
            QModelIndex tIndex = pModel->index(iRow, 0);

            if (pModel->data(tIndex, CGraphModel::eCommitIdRole).toString() != sCommitId)
                continue;

            QVariant vLabels = pModel->data(tIndex, CGraphModel::eLabelsRole);
            CLabelModel* pLabels = vLabels.value<CLabelModel*>();

            if (pLabels == nullptr)
                return false;

            for (int iLabel = 0; iLabel < pLabels->rowCount(); iLabel++)
            {
                if (pLabels->data(pLabels->index(iLabel, 0), CLabelModel::eNameRole).toString() == sLabelName
                        && pLabels->data(pLabels->index(iLabel, 0), CLabelModel::eTypeRole).toInt() == CEnums::WorktreeLabel)
                    return true;
            }

            return false;
        }

        return false;
    }

    bool graphRowHasWorktreeLabel(const QString& sCommitId, const QString& sLabelName) const
    {
        return graphRowHasWorktreeLabelStatic(m_pRepository->graphModel(), sCommitId, sLabelName);
    }

    // True when any loaded graph row carries at least one label
    bool graphHasLabeledRow() const
    {
        CGraphModel* pModel = m_pRepository->graphModel();

        for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
        {
            QVariant vLabels = pModel->data(pModel->index(iRow), CGraphModel::eLabelsRole);
            CLabelModel* pLabels = vLabels.value<CLabelModel*>();

            if (pLabels != nullptr && pLabels->rowCount() > 0)
                return true;
        }

        return false;
    }

    QString m_sFixtureRepo;
    QScopedPointer<QTemporaryDir> m_pWorkDir;
    QString m_sClonePath;
    QScopedPointer<CRepository> m_pRepository;
    QString m_sFullName;
    QString m_sRelativeName;

    // Clones the fixture to a private folder so tests never touch it
    void cloneFixture()
    {
        m_pWorkDir.reset(new QTemporaryDir());

        QVERIFY(m_pWorkDir->isValid());

        m_sClonePath = m_pWorkDir->path() + "/work";

        QCOMPARE(QProcess::execute("git", {"clone", "--quiet", m_sFixtureRepo, m_sClonePath}), 0);

        m_sRelativeName = "source/module-7.txt";
        m_sFullName = m_sClonePath + "/" + m_sRelativeName;

        QVERIFY2(QFile::exists(m_sFullName), qPrintable(m_sFullName));
    }

    // Dirties the working tree with deterministic added lines
    void appendWorktreeLines()
    {
        QFile tFile(m_sFullName);

        QVERIFY(tFile.open(QIODevice::Append | QIODevice::Text));

        QTextStream tStream(&tFile);

        tStream << "line-B1\nline-B2\nline-B3\n";
        tFile.close();
    }

    // Writes an untracked file in the clone work tree
    bool createUntrackedFile(const QString& sRelativeName, const QString& sContent)
    {
        QFile tFile(m_sClonePath + "/" + sRelativeName);

        if (not tFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QTextStream tStream(&tFile);

        tStream << sContent;

        return true;
    }

    // Runs git in the clone and returns standard output
    bool gitOutput(const QStringList& lArguments, QString& sOutput)
    {
        QProcess tProcess;

        tProcess.setWorkingDirectory(m_sClonePath);
        tProcess.start("git", lArguments);

        if (not tProcess.waitForFinished(10000))
            return false;

        if (tProcess.exitCode() != 0)
            return false;

        sOutput = QString::fromUtf8(tProcess.readAllStandardOutput());

        return true;
    }

    // Selects the file like the UI does and waits for the async diff to land
    void selectFileAndWaitForDiff()
    {
        QMetaObject::invokeMethod(m_pRepository.data(), "onCurrentFileFullName", Q_ARG(QString, m_sFullName));

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->unstagedFileDiffModel()->rowCount() > 0, s_iAsyncTimeoutMs);
    }

    // Source rows of added lines whose text contains the marker
    QList<int> addedSourceRows(CDiffModel* pModel, const QString& sMarker)
    {
        QList<int> lRows;

        for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
        {
            CDiffLine* pLine = pModel->lineAt(iRow);

            if (pLine != nullptr
                    && pLine->operation() == CEnums::DiffAdd
                    && pLine->text().contains(sMarker))
            {
                lRows << iRow;
            }
        }

        return lRows;
    }

    // Maps source rows to proxy rows, as the QML view passes proxy rows
    QVariantList proxyRows(CDiffModel* pModel, const QList<int>& lSourceRows)
    {
        return proxyRows(m_pRepository->unstagedFileDiffModelProxy(), pModel, lSourceRows);
    }

    QVariantList proxyRows(CDiffModelProxy* pProxy, CDiffModel* pModel, const QList<int>& lSourceRows)
    {
        QVariantList lProxyRows;

        for (int iRow : lSourceRows)
            lProxyRows << pProxy->mapFromSource(pModel->index(iRow, 0)).row();

        return lProxyRows;
    }

    bool modelContains(CDiffModel* pModel, const QString& sMarker)
    {
        for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
        {
            CDiffLine* pLine = pModel->lineAt(iRow);

            if (pLine != nullptr && pLine->text().contains(sMarker))
                return true;
        }

        return false;
    }

    // True when an added line contains the marker (context lines share the text)
    bool addedContains(CDiffModel* pModel, const QString& sMarker)
    {
        for (int iRow : addedSourceRows(pModel, sMarker))
        {
            Q_UNUSED(iRow);
            return true;
        }

        return false;
    }

    // True once the rewrite moved the branch and the sequencer left no
    // rebase directory behind: HEAD alone is not enough, it detaches first
    bool rebaseFinished(const QString& sOldHead)
    {
        QString sHead;

        if (not gitOutput({"rev-parse", "HEAD"}, sHead))
            return false;

        if (sHead.trimmed().isEmpty() || sHead.trimmed() == sOldHead)
            return false;

        QDir tGitDir(m_sClonePath + "/.git");

        return not tGitDir.exists("rebase-merge") && not tGitDir.exists("rebase-apply");
    }

    // Content of the output panel: surfaces git errors on failure
    QString outputLog()
    {
        QStringList lLines;

        QStringListModel* pOutput = m_pRepository->commandOutputModel();

        for (int iRow = 0; iRow < pOutput->rowCount(); iRow++)
            lLines << pOutput->data(pOutput->index(iRow, 0)).toString();

        return lLines.join("\n");
    }

    // Commit id a remote repository holds for a ref, empty when missing
    static QString remoteRefCommit(const QString& sRepositoryPath, const QString& sRefName)
    {
        QProcess tProcess;

        tProcess.setWorkingDirectory(sRepositoryPath);
        tProcess.start("git", {"rev-parse", "--verify", "--quiet", sRefName});

        if (not tProcess.waitForFinished(10000))
            return QString();

        return QString::fromUtf8(tProcess.readAllStandardOutput()).trimmed();
    }

    // Commit id held by a branch or tag model for a name, empty when missing
    QString branchCommitId(CBranchModel* pModel, const QString& sName)
    {
        for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
        {
            QModelIndex tIndex = pModel->index(iRow, 0);

            if (pModel->data(tIndex, CBranchModel::eNameRole).toString() == sName)
                return pModel->data(tIndex, CBranchModel::eCommitIdRole).toString();
        }

        return QString();
    }
};

QTEST_MAIN(tst_Repository)

#include "tst_repository.moc"
