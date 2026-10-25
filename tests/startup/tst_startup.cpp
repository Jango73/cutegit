// Qt
#include <QtTest>
#include <QTemporaryDir>
#include <QScopedPointer>
#include <QProcess>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickStyle>
#include <QStandardPaths>

// Application
#include "CController.h"
#include "CRepositoryModel.h"
#include "CRepository.h"
#include "CEnums.h"

/*!
    \class tst_Startup
    Startup test: boots the real CController and loads the real main window
    (App.qml) headless, exactly like production does, then opens a repository
    cloned from the fixture. Any QML component that fails to load, any missing
    module, or any broken binding in the full view chain fails the test.
*/

static const int s_iAsyncTimeoutMs = 20000;

class tst_Startup : public QObject
{
    Q_OBJECT

public:

    tst_Startup()
        : m_sFixtureRepo(qgetenv("DIFF_FIXTURE_REPO"))
    {
    }

private slots:

    void initTestCase()
    {
        // Isolate the controller from any real configuration on disk:
        // CController persists open repositories and reloads them on
        // construction, so stale entries would break the rowCount() == 1
        // expectation below. Must run before any CController is created.
        QStandardPaths::setTestModeEnabled(true);

        QVERIFY2(not m_sFixtureRepo.isEmpty(), "DIFF_FIXTURE_REPO is not set");
        QVERIFY2(QFile::exists(m_sFixtureRepo + "/.git/HEAD"), "Fixture repository has no .git");
    }

    void startup_loadsMainWindow()
    {
        CController tController("");
        QQmlApplicationEngine tEngine;
        QStringList lWarnings;

        prepareEngine(tController, tEngine, lWarnings);

        tEngine.load(QUrl(QStringLiteral("qrc:/qml/App.qml")));

        QVERIFY2(not tEngine.rootObjects().isEmpty(), "App.qml produced no root object");
        QVERIFY2(not hasLoadError(lWarnings), qPrintable(formatWarnings(lWarnings)));
    }

    void startup_opensRepositoryView()
    {
        QTemporaryDir tParentDir;
        QVERIFY2(tParentDir.isValid(), "Could not create temporary folder");
        QString sClonePath = tParentDir.path() + "/repo";

        QCOMPARE(cloneFixture(sClonePath), 0);

        CController tController("");
        QQmlApplicationEngine tEngine;
        QStringList lWarnings;

        // Registers the CuteGit QML module and the Material style before any
        // QML file is parsed: creating a component first poisons the engine
        // with a "module CuteGit is not installed" import error
        prepareEngine(tController, tEngine, lWarnings);

        QQmlComponent tRepositoryView(&tEngine, QUrl(QStringLiteral("qrc:/qml/views/RepositoryView.qml")));

        // The application chrome loads without an open repository
        tEngine.load(QUrl(QStringLiteral("qrc:/qml/App.qml")));

        QVERIFY2(not tEngine.rootObjects().isEmpty(), "App.qml produced no root object");

        // The per-repository view loads with a real controller and a real repository.
        // It is instantiated directly because a Repeater inside a StackLayout does not
        // create delegates headless on Qt 6.2 (verified with a minimal QML case).
        tController.openRepository(sClonePath);

        CRepositoryModel* pModel =
            qobject_cast<CRepositoryModel*>(tController.property("openRepositoryModel").value<QObject*>());
        QVERIFY2(pModel != nullptr, "Controller has no openRepositoryModel");
        QTRY_VERIFY_WITH_TIMEOUT(pModel->rowCount() == 1, s_iAsyncTimeoutMs);

        QVERIFY2(tRepositoryView.isReady(), "RepositoryView.qml is not ready");

        QScopedPointer<QObject> pView(tRepositoryView.create());
        QVERIFY2(not pView.isNull(), "RepositoryView.qml produced no object");

        pView->setProperty("controller", QVariant::fromValue(&tController));
        pView->setProperty("repository", QVariant::fromValue(pModel->repositories().value(0)));

        QTRY_VERIFY_WITH_TIMEOUT(pView->findChild<QObject*>("repositoryStatus") != nullptr,
                                 s_iAsyncTimeoutMs);

        // Let one status poll cycle settle the asynchronous bindings
        QTest::qWait(3500);

        QVERIFY2(not hasLoadError(lWarnings), qPrintable(formatWarnings(lWarnings)));

        // The file diff pane instantiates both working tree diff views directly:
        // tab panels load lazily, so the repository view alone cannot prove it
        QQmlComponent tDiffPane(&tEngine, QUrl(QStringLiteral("qrc:/qml/pages/FileDiffPane.qml")));
        QVERIFY2(tDiffPane.isReady(), "FileDiffPane.qml is not ready");

        QScopedPointer<QObject> pPane(tDiffPane.create());
        QVERIFY2(not pPane.isNull(), "FileDiffPane.qml produced no object");

        pPane->setProperty("repository", QVariant::fromValue(pModel->repositories().value(0)));

        QTRY_VERIFY_WITH_TIMEOUT(pPane->findChild<QObject*>("unstagedDiffView") != nullptr,
                                 s_iAsyncTimeoutMs);
        QVERIFY2(pPane->findChild<QObject*>("stagedDiffView") != nullptr,
                 "The staged diff view inside the file diff pane was never instantiated");
        QVERIFY2(not hasLoadError(lWarnings), qPrintable(formatWarnings(lWarnings)));
    }

    void repositoryLimit_unloadsOldestTabs()
    {
        QTemporaryDir tParentDir;
        QVERIFY2(tParentDir.isValid(), "Could not create temporary folder");

        QString sPathA = tParentDir.path() + "/repoA";
        QString sPathB = tParentDir.path() + "/repoB";
        QString sPathC = tParentDir.path() + "/repoC";
        QVERIFY2(initGitRepo(sPathA), "Could not init repoA");
        QVERIFY2(initGitRepo(sPathB), "Could not init repoB");
        QVERIFY2(initGitRepo(sPathC), "Could not init repoC");

        {
            CController tController("");
            QCOMPARE(tController.maxLoadedRepositories(), 3);

            tController.setMaxLoadedRepositories(2);
            QCOMPARE(tController.maxLoadedRepositories(), 2);

            tController.openRepository(sPathA);
            tController.openRepository(sPathB);
            tController.openRepository(sPathC);

            CRepositoryModel* pModel = openRepositoryModel(tController);
            QVERIFY2(pModel != nullptr, "Controller has no openRepositoryModel");
            QCOMPARE(pModel->rowCount(), 3);

            // Drain the scans issued by the interactive opens
            QTRY_VERIFY_WITH_TIMEOUT(!isBusy(pModel), s_iAsyncTimeoutMs);

            // Visit every tab: only the two most recent visits stay loaded
            tController.setCurrentRepositoryIndex(0);
            tController.setCurrentRepositoryIndex(1);
            tController.setCurrentRepositoryIndex(2);

            QTRY_VERIFY_WITH_TIMEOUT(!pModel->repositories().value(0)->isLoaded(), s_iAsyncTimeoutMs);
            QVERIFY(pModel->repositories().value(1)->isLoaded());
            QVERIFY(pModel->repositories().value(2)->isLoaded());

            // Revisiting the unloaded tab reloads it and unloads the oldest visit
            tController.setCurrentRepositoryIndex(0);

            QTRY_VERIFY_WITH_TIMEOUT(pModel->repositories().value(0)->isLoaded(), s_iAsyncTimeoutMs);
            QTRY_VERIFY_WITH_TIMEOUT(!pModel->repositories().value(1)->isLoaded(), s_iAsyncTimeoutMs);
            QVERIFY(pModel->repositories().value(2)->isLoaded());

            tController.saveConfiguration();

            // The limit persisted through the configuration file, unloaded
            // repositories reload lazily like at application startup
            {
                CController tReloaded("");
                QCOMPARE(tReloaded.maxLoadedRepositories(), 2);

                CRepositoryModel* pReloadedModel = openRepositoryModel(tReloaded);
                QVERIFY2(pReloadedModel != nullptr, "Reloaded controller has no openRepositoryModel");
                QCOMPARE(pReloadedModel->rowCount(), 3);

                for (CRepository* pRepository : pReloadedModel->repositories())
                    QVERIFY(!pRepository->isLoaded());
            }

            // Lowering the limit unloads down to the current repository
            tController.setMaxLoadedRepositories(1);
            QTRY_VERIFY_WITH_TIMEOUT(!pModel->repositories().value(2)->isLoaded(), s_iAsyncTimeoutMs);
            QVERIFY(pModel->repositories().value(0)->isLoaded());
            QVERIFY(!pModel->repositories().value(1)->isLoaded());

            // Cleanup for the other test functions: no open repository left behind
            tController.setMaxLoadedRepositories(3);
            tController.removeRepository(2);
            tController.removeRepository(1);
            tController.removeRepository(0);
        }

        {
            // The cleanup persisted: a fresh controller starts empty again
            CController tClean("");
            QCOMPARE(tClean.maxLoadedRepositories(), 3);
            CRepositoryModel* pCleanModel = openRepositoryModel(tClean);
            QVERIFY2(pCleanModel != nullptr, "Clean controller has no openRepositoryModel");
            QCOMPARE(pCleanModel->rowCount(), 0);
        }
    }

    void cleanupTestCase()
    {
    }

private:

    QString m_sFixtureRepo;

    void prepareEngine(CController& tController, QQmlApplicationEngine& tEngine,
                       QStringList& lWarnings)
    {
        qmlRegisterUncreatableType<CEnums>("CuteGit", 1, 0, "CEnums", "Cannot create a CEnums instance.");
        tEngine.rootContext()->setContextProperty("controller", &tController);
        QQuickStyle::setStyle("Material");

        connect(&tEngine, &QQmlEngine::warnings, this,
                [&lWarnings](const QList<QQmlError>& lErrors)
                {
                    for (const QQmlError& tError : lErrors)
                        lWarnings << tError.toString();
                });
    }

    bool hasLoadError(const QStringList& lWarnings)
    {
        for (const QString& sWarning : lWarnings)
        {
            if (sWarning.contains("Failed to load", Qt::CaseInsensitive)
                || sWarning.contains("is not installed", Qt::CaseInsensitive)
                || sWarning.contains("ReferenceError", Qt::CaseSensitive)
                || sWarning.contains("TypeError", Qt::CaseSensitive))
                return true;
        }

        return false;
    }

    QString formatWarnings(const QStringList& lWarnings)
    {
        return QString("QML warnings:\n%1").arg(lWarnings.join("\n"));
    }

    int cloneFixture(const QString& sClonePath)
    {
        QProcess tGit;
        tGit.start("git", QStringList() << "clone" << "--quiet" << m_sFixtureRepo << sClonePath);
        tGit.waitForFinished(s_iAsyncTimeoutMs);
        return tGit.exitCode();
    }

    bool initGitRepo(const QString& sPath)
    {
        QProcess tGit;
        tGit.start("git", QStringList() << "init" << "--quiet" << sPath);
        return tGit.waitForFinished(s_iAsyncTimeoutMs) && tGit.exitCode() == 0;
    }

    CRepositoryModel* openRepositoryModel(CController& tController)
    {
        return qobject_cast<CRepositoryModel*>(tController.property("openRepositoryModel").value<QObject*>());
    }

    bool isBusy(CRepositoryModel* pModel)
    {
        for (CRepository* pRepository : pModel->repositories())
        {
            if (pRepository != nullptr && pRepository->commands() != nullptr && pRepository->commands()->isBusy())
                return true;
        }

        return false;
    }
};

QTEST_MAIN(tst_Startup)

#include "tst_startup.moc"
