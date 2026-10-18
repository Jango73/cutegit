
// Qt
#include <QtTest>
#include <QTemporaryDir>
#include <QScopedPointer>
#include <QProcess>
#include <QFile>
#include <QTextStream>

// Application
#include "CRepository.h"
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
    // each named refresh loads one domain while the others stay stale
    void logPane_loadsOnlyOnDemand()
    {
        QTest::qWait(3000);

        QCOMPARE(m_pRepository->branchLogModel()->rowCount(), 0);
        QCOMPARE(m_pRepository->graphModel()->rowCount(), 0);
        QCOMPARE(m_pRepository->refLogModel()->rowCount(), 0);

        m_pRepository->refreshBranchLog();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->branchLogModel()->rowCount() > 0, s_iAsyncTimeoutMs);

        QCOMPARE(m_pRepository->graphModel()->rowCount(), 0);
        QCOMPARE(m_pRepository->refLogModel()->rowCount(), 0);

        m_pRepository->refreshGraph();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->graphModel()->rowCount() > 0, s_iAsyncTimeoutMs);

        m_pRepository->refreshRefLog();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->refLogModel()->rowCount() > 0, s_iAsyncTimeoutMs);

        // A refresh marks every domain stale without rereading, the next
        // named refresh reloads the domain
        m_pRepository->refresh();
        m_pRepository->refreshBranchLog();

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->branchLogModel()->rowCount() > 0, s_iAsyncTimeoutMs);
    }

    // The tracked seed carries versioned files only: ignored content lands
    // with its ignored status from the status batch, .git internals and
    // ignored folders never inflate the file models like the old crawl did
    void trackedSeed_hidesIgnoredAndGitFolder()
    {
        QDir tClone(m_sClonePath);

        QVERIFY(tClone.mkpath("ignored-seed"));

        QFile tJunk(tClone.filePath("ignored-seed/junk.bin"));
        QVERIFY(tJunk.open(QIODevice::WriteOnly));
        tJunk.write("junk");
        tJunk.close();

        QFile tIgnore(tClone.filePath(".gitignore"));
        QVERIFY(tIgnore.open(QIODevice::WriteOnly | QIODevice::Text));
        tIgnore.write("ignored-seed/\n");
        tIgnore.close();

        m_pRepository->refresh();

        QString sIgnoredKey = m_sClonePath + "/ignored-seed/junk.bin";

        QTRY_VERIFY_WITH_TIMEOUT(m_pRepository->repoFiles().containsKey(sIgnoredKey), s_iAsyncTimeoutMs);
        QCOMPARE(m_pRepository->repoFiles().itemByKey(sIgnoredKey)->status(), CEnums::eIgnored);

        for (CRepoFile* pFile : m_pRepository->repoFiles())
        {
            QVERIFY2(not pFile->fullName().contains("/.git/"), qPrintable(pFile->fullName()));
            QVERIFY2(not pFile->fullName().contains("/ignored-seed/") || pFile->status() == CEnums::eIgnored,
                     qPrintable(pFile->fullName()));
        }
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

        // Lightweight tags fall back to their object, annotated tags peel to their commit
        QVERIFY(gitOutput({"tag", "v-batch-light"}, sUnused));
        QVERIFY(gitOutput({"tag", "-a", "-m", "probe", "v-batch-annotated"}, sUnused));

        m_pRepository->refresh();

        CBranchModel* pTags = m_pRepository->tagModel();

        QTRY_VERIFY_WITH_TIMEOUT(not branchCommitId(pTags, "v-batch-light").isEmpty(), s_iAsyncTimeoutMs);
        QTRY_VERIFY_WITH_TIMEOUT(not branchCommitId(pTags, "v-batch-annotated").isEmpty(), s_iAsyncTimeoutMs);
    }

private:

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
