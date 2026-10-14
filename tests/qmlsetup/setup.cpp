// Qt
#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QtQml>

// Application
#include "CEnums.h"
#include "CDiffLine.h"
#include "CDiffModel.h"
#include "CDiffModelProxy.h"

//-------------------------------------------------------------------------------------------------

class FakeDiffRepository : public QObject
{
    Q_OBJECT

public:

    explicit FakeDiffRepository(QObject* parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void stageDiffLines(const QVariantList& lProxyRows) { emit stageDiffLinesCalled(lProxyRows); }
    Q_INVOKABLE void unstageDiffLines(const QVariantList& lProxyRows) { emit unstageDiffLinesCalled(lProxyRows); }
    Q_INVOKABLE void stageDiffHunk(int iHunkId) { emit stageDiffHunkCalled(iHunkId); }
    Q_INVOKABLE void unstageDiffHunk(int iHunkId) { emit unstageDiffHunkCalled(iHunkId); }

signals:

    void stageDiffLinesCalled(const QVariantList& lProxyRows);
    void unstageDiffLinesCalled(const QVariantList& lProxyRows);
    void stageDiffHunkCalled(int iHunkId);
    void unstageDiffHunkCalled(int iHunkId);
};

//-------------------------------------------------------------------------------------------------

class DiffTestHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(CDiffModelProxy* modelProxy READ modelProxy CONSTANT)
    Q_PROPERTY(FakeDiffRepository* repository READ repository CONSTANT)

public:

    explicit DiffTestHelper(QObject* parent = nullptr)
        : QObject(parent)
        , m_pModel(new CDiffModel(this))
        , m_pProxy(new CDiffModelProxy(nullptr, this))
        , m_pRepository(new FakeDiffRepository(this))
    {
        m_pProxy->setSourceModel(m_pModel);
    }

    CDiffModelProxy* modelProxy() const { return m_pProxy; }
    FakeDiffRepository* repository() const { return m_pRepository; }

    Q_INVOKABLE void reset() { m_lPending.clear(); }

    Q_INVOKABLE void addFileHeader(const QString& sName) { addLine(CEnums::DiffFileName, sName, -1); }
    Q_INVOKABLE void addHunk(int iHunkId, const QString& sHeader) { addLine(CEnums::DiffHunk, sHeader, iHunkId); }

    Q_INVOKABLE void addLine(int iOperation, const QString& sText, int iHunkId)
    {
        CDiffLine* pLine = new CDiffLine();
        pLine->setOperation(static_cast<CEnums::EDiffOperation>(iOperation));
        pLine->setText(sText);
        pLine->setHunkId(iHunkId);
        m_lPending << pLine;
    }

    Q_INVOKABLE void commitLines()
    {
        m_pModel->setLines(m_lPending);
        m_lPending.clear();
    }

private:

    CDiffModel* m_pModel;
    CDiffModelProxy* m_pProxy;
    FakeDiffRepository* m_pRepository;
    QList<CDiffLine*> m_lPending;
};

//-------------------------------------------------------------------------------------------------

class DiffTestSetup : public QObject
{
    Q_OBJECT

public slots:

    void applicationAvailable() {}

    void qmlEngineAvailable(QQmlEngine* pEngine)
    {
        Q_UNUSED(pEngine)
        qmlRegisterUncreatableType<CEnums>("CuteGit", 1, 0, "CEnums", "Cannot create a CEnums instance.");
        qmlRegisterType<DiffTestHelper>("DiffTest", 1, 0, "DiffTestHelper");
    }
};

//-------------------------------------------------------------------------------------------------

QUICK_TEST_MAIN_WITH_SETUP(DiffViewTests, DiffTestSetup)

#include "setup.moc"
