
// Qt
#include <QtTest>
#include <QTemporaryDir>
#include <QScopedPointer>
#include <QProcess>
#include <QFile>
#include <QTextStream>

// Application
#include "CRepository.h"
#include "CLogModel.h"
#include "CGraphModel.h"
#include "CLabelModel.h"
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
    }

private slots:

    void initTestCase()
    {
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

 private:

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

    // Content of the output panel: surfaces git errors on failure
    QString outputLog()
    {
        QStringList lLines;

        QStringListModel* pOutput = m_pRepository->commandOutputModel();

        for (int iRow = 0; iRow < pOutput->rowCount(); iRow++)
            lLines << pOutput->data(pOutput->index(iRow, 0)).toString();

        return lLines.join("\n");
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
