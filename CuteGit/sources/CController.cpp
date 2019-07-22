
// Qt
#include <QDebug>
#include <QUrl>
#include <QApplication>

// Application
#include "CController.h"
#include "CGitCommands.h"

//-------------------------------------------------------------------------------------------------

/*

  Workflow of GIT interactive rebase:
  * CuteGit-1-A runs (1 is first instance, A is main thread)
  * CuteGit-1-A calls CCommands::exec with "git rebase --interactive" and "GIT_SEQUENCE_EDITOR=<path-to-cutegit>"
  * CuteGit-1-B launches process (1 is first instance, B is process execution thread)
  * GIT starts
  * GIT launches process : <path-to-cutegit> <path-to-sequence-file>
  * CuteGit-2 starts in slave mode (because it has one argument)
  * CuteGit-2-A asks (via shared memory) for sequence file edition
  * CuteGit-1-A edits the sequence file for slave and notifies it
  * CuteGit-1-B is still waiting for GIT process to end
  * CuteGit-2 quits
  * if changing a commit:
  ** GIT launches process : <path-to-cutegit> <path-to-commit-file>
  ** CuteGit-2 starts in slave mode (because it has one argument)
  ** CuteGit-2-A asks (via shared memory) for commit file edition
  ** CuteGit-1-A edits the sequence file for slave and notifies it
  ** CuteGit-2 quits
  * GIT quits
  * CuteGit-1-B returns

 * */

//-------------------------------------------------------------------------------------------------

const QString sParamConfiguration = "Configuration";
const QString sParamCurrentRepository = "CurrentRepository";
const QString sParamRepositories = "Repositories";
const QString sParamRepository = "Repository";
const QString sParamPath = "Path";

const QString CController::m_sSharedKey = "CuteGit-Shared-Memory";

//-------------------------------------------------------------------------------------------------

CController::CController(QObject* parent)
    : QObject(parent)
    , m_pCommands(new CGitCommands())
    , m_pTreeFileModel(nullptr)
    , m_pTreeFileModelProxy(new CTreeFileModelProxy(this, this))
    , m_pFlatFileModel(nullptr)
    , m_pRepositoryModel(new QStringListModel(this))
    , m_pCommandOutputModel(new QStringListModel(this))
    , m_bShowClean(false)
    , m_bShowAdded(true)
    , m_bShowModified(true)
    , m_bShowDeleted(true)
    , m_bShowUntracked(false)
    , m_bMasterMode(true)
    , m_tShared(m_sSharedKey, this)
    , m_tSharedTimer(this)
{
    loadConfiguration();

    if (m_tShared.create(SHARED_MEMORY_MAX))
    {
        clearSharedMemory();

        connect(&m_tSharedTimer, &QTimer::timeout, this, &CController::onSharedTimerTick);
        m_tSharedTimer.start(500);
    }
    else
    {
        qWarning() << "Could not create shared memory segment";
    }
}

//-------------------------------------------------------------------------------------------------

CController::CController(QString sSequenceFileName, QObject* parent)
    : QObject(parent)
    , m_pCommands(new CGitCommands())
    , m_pTreeFileModel(nullptr)
    , m_pTreeFileModelProxy(nullptr)
    , m_pFlatFileModel(nullptr)
    , m_pRepositoryModel(nullptr)
    , m_pCommandOutputModel(nullptr)
    , m_bMasterMode(false)
    , m_tShared(m_sSharedKey, this)
    , m_tSharedTimer(this)
{
    Q_UNUSED(sSequenceFileName);

    if (m_tShared.attach())
    {
        connect(&m_tSharedTimer, &QTimer::timeout, this, &CController::onSharedTimerTick);
        m_tSharedTimer.start(500);

        setSequenceFileName(sSequenceFileName);
        setSharedOperation(eSOSlaveRequestEdit);
    }
    else
    {
        qWarning() << "Could not attach to shared memory segment";
    }
}

//-------------------------------------------------------------------------------------------------

CController::~CController()
{
    if (m_bMasterMode)
        saveConfiguration();

    if (m_tShared.detach() == false)
    {
        qWarning() << "Could not detach from shared memory segment";
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowClean(bool bValue)
{
    if (m_bShowClean != bValue)
    {
        m_bShowClean = bValue;
        if (m_pTreeFileModelProxy != nullptr)
            m_pTreeFileModelProxy->filterChanged();
        emit showCleanChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowAdded(bool bValue)
{
    if (m_bShowAdded != bValue)
    {
        m_bShowAdded = bValue;
        if (m_pTreeFileModelProxy != nullptr)
            m_pTreeFileModelProxy->filterChanged();
        emit showAddedChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowModified(bool bValue)
{
    if (m_bShowModified != bValue)
    {
        m_bShowModified = bValue;
        if (m_pTreeFileModelProxy != nullptr)
            m_pTreeFileModelProxy->filterChanged();
        emit showModifiedChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowDeleted(bool bValue)
{
    if (m_bShowDeleted != bValue)
    {
        m_bShowDeleted = bValue;
        if (m_pTreeFileModelProxy != nullptr)
            m_pTreeFileModelProxy->filterChanged();
        emit showDeletedChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowUntracked(bool bValue)
{
    if (m_bShowUntracked != bValue)
    {
        m_bShowUntracked = bValue;
        if (m_pTreeFileModelProxy != nullptr)
            m_pTreeFileModelProxy->filterChanged();
        emit showUntrackedChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setRepositoryPath(QString sPath)
{
    if (sPath.startsWith("file:"))
        sPath = QUrl(sPath).toLocalFile();

    // If repo path valid and different from current
    if (m_pTreeFileModel == nullptr || sPath != m_pTreeFileModel->rootPath())
    {
        // IF repo is a GIT repo
        if (QDir(QString("%1/.git").arg(sPath)).exists())
        {
            // Delete any existing file model
            if (m_pTreeFileModel != nullptr)
                m_pTreeFileModel->deleteLater();

            if (m_pFlatFileModel != nullptr)
                m_pFlatFileModel->deleteLater();

            // Create a file model
            m_pTreeFileModel = new CTreeFileModel(this, this);
            m_pFlatFileModel = new CFlatFileModel(this, this);

            m_pTreeFileModelProxy->setSourceModel(m_pTreeFileModel);
            m_pTreeFileModel->setRootPath(sPath);

            emit repositoryPathChanged();
            emit treeFileModelChanged();
            emit treeFileModelProxyChanged();
            emit flatFileModelChanged();

            connect(m_pTreeFileModel, &CTreeFileModel::currentFileFullName, this, &CController::onCurrentFileFullName);
            connect(m_pTreeFileModel, &CTreeFileModel::newOutput, this, &CController::onNewOutput);
            connect(m_pFlatFileModel, &CFlatFileModel::currentFileFullName, this, &CController::onCurrentFileFullName);

            // Add this path to repository model
            QStringList lRepositoryPaths = m_pRepositoryModel->stringList();

            if (lRepositoryPaths.contains(sPath) == false)
                lRepositoryPaths << sPath;

            m_pRepositoryModel->setStringList(lRepositoryPaths);

            // Clear command output
            m_pCommandOutputModel->setStringList(QStringList());
        }
        else
        {
            onNewOutput(QString(tr("%1 is not a GIT repository.\nPlease select a folder containing a GIT repository.")).arg(sPath));
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setSharedOperation(ESharedOperation eOperation)
{
    if (m_tShared.lock())
    {
        SMemoryStruct* pData = static_cast<SMemoryStruct*>(m_tShared.data());
        pData->eOperation = eOperation;
        m_tShared.unlock();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setSequenceFileName(const QString& sSequenceFileName)
{
    if (m_tShared.lock())
    {
        SMemoryStruct* pData = static_cast<SMemoryStruct*>(m_tShared.data());
        strcpy(pData->sSequenceFileName, sSequenceFileName.toUtf8().constData());
        m_tShared.unlock();
    }
}

//-------------------------------------------------------------------------------------------------

QString CController::repositoryPath() const
{
    if (m_pTreeFileModel != nullptr)
    {
        return m_pTreeFileModel->rootPath();
    }

    return "";
}

//-------------------------------------------------------------------------------------------------

CController::ESharedOperation CController::sharedOperation()
{
    ESharedOperation eReturnValue = eSONone;

    if (m_tShared.lock())
    {
        SMemoryStruct* pData = static_cast<SMemoryStruct*>(m_tShared.data());
        eReturnValue = pData->eOperation;
        m_tShared.unlock();
    }

    return eReturnValue;
}

//-------------------------------------------------------------------------------------------------

QString CController::sequenceFileName()
{
    QString sReturnValue;

    if (m_tShared.lock())
    {
        SMemoryStruct* pData = static_cast<SMemoryStruct*>(m_tShared.data());
        sReturnValue = QString::fromUtf8(pData->sSequenceFileName);
        m_tShared.unlock();
    }

    return sReturnValue;
}

//-------------------------------------------------------------------------------------------------

void CController::saveConfiguration()
{
    CXMLNode xConfig(sParamConfiguration);

    CXMLNode xCurrentRepository(sParamCurrentRepository);
    xCurrentRepository.attributes()[sParamPath] = m_pTreeFileModel->rootPath();
    xConfig << xCurrentRepository;

    CXMLNode xRepositories(sParamRepositories);

    // Add repository to model
    QStringList lRepositoryPaths = m_pRepositoryModel->stringList();

    for (QString sPath : lRepositoryPaths)
    {
        CXMLNode xRepository(sParamRepository);
        xRepository.attributes()[sParamPath] = sPath;
        xRepositories << xRepository;
    }

    xConfig << xRepositories;

    xConfig.saveXMLToFile(CONFIG_FILE_NAME);
}

//-------------------------------------------------------------------------------------------------

void CController::loadConfiguration()
{
    CXMLNode xConfig = CXMLNode::load(CONFIG_FILE_NAME);

    CXMLNode xCurrentRepository = xConfig.getNodeByTagName(sParamCurrentRepository);
    QString sCurrentPath = xCurrentRepository.attributes()[sParamPath];

    CXMLNode xRepositories = xConfig.getNodeByTagName(sParamRepositories);
    QVector<CXMLNode> xRepositoryList = xRepositories.getNodesByTagName(sParamRepository);

    QStringList lRepositoryPaths;

    for (CXMLNode xRepository : xRepositoryList)
    {
        lRepositoryPaths << xRepository.attributes()[sParamPath];
    }

    m_pRepositoryModel->setStringList(lRepositoryPaths);

    if (sCurrentPath.isEmpty() == false)
    {
        setRepositoryPath(sCurrentPath);
    }
    else if (lRepositoryPaths.count() > 0)
    {
        setRepositoryPath(lRepositoryPaths[0]);
    }
}

//-------------------------------------------------------------------------------------------------

void CController::clearSharedMemory()
{
    if (m_tShared.lock())
    {
        SMemoryStruct* pData = static_cast<SMemoryStruct*>(m_tShared.data());
        memset(pData, 0, sizeof(SMemoryStruct));
        m_tShared.unlock();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::quit()
{
    QApplication::quit();
}

//-------------------------------------------------------------------------------------------------

void CController::clearOutput()
{
    m_pCommandOutputModel->setStringList(QStringList());
}

//-------------------------------------------------------------------------------------------------

void CController::onCurrentFileFullName(QString sFileFullName)
{
    m_pCommands->unstagedFileDiff(repositoryPath(), sFileFullName);
    m_pCommands->fileLog(repositoryPath(), sFileFullName);
}

//-------------------------------------------------------------------------------------------------

void CController::onNewOutput(QString sOutput)
{
    QStringList lNewList = sOutput.split("\n");
    QStringList lData = m_pCommandOutputModel->stringList();
    bool bHasNewLine = false;

    for (QString sLine : lNewList)
    {
        sLine = sLine.trimmed();

        if (sLine.isEmpty() == false)
        {
            bHasNewLine = true;
            lData << sLine;
        }
    }

    if (bHasNewLine)
    {
        lData << "----------------------------------------------------------------------------------------------------";
    }

    while (lData.count() > 50)
        lData.removeFirst();

    m_pCommandOutputModel->setStringList(lData);
}

//-------------------------------------------------------------------------------------------------

void CController::onSharedTimerTick()
{
    if (m_bMasterMode)
    {
        if (sharedOperation() == eSOSlaveRequestEdit)
        {
            QString sFileName = sequenceFileName();

            m_pCommands->editSequenceFile(sFileName);

            setSharedOperation(eSOMasterFinishedEdit);
        }
    }
    else
    {
        if (sharedOperation() == eSOMasterFinishedEdit)
        {
            setSharedOperation(eSONone);

            quit();
        }
    }
}
