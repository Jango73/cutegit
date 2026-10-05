
// Qt
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>

// Application
#include "CController.h"
#include "Strings.h"

//-------------------------------------------------------------------------------------------------

/*!
    \class CController
    \inmodule CuteGit
    \section1 General
    This is the high level controller of the application.
    It also servers as the unique interface between UI and business logic.
    \section1 Workflow of Git interactive rebase
    * CuteGit-1-A runs (1 is first instance, A is main thread)
    * CuteGit-1-A calls CCommands::exec with "git rebase --interactive" and "GIT_SEQUENCE_EDITOR=<path-to-cutegit>"
    * CuteGit-1-B launches process (1 is first instance, B is process execution thread)
    * Git starts
    * Git launches process : <path-to-cutegit> <path-to-sequence-file>
    * CuteGit-2 starts in slave mode (because it has one argument)
    * CuteGit-2-A asks (via shared memory) for sequence file edition
    * CuteGit-1-A edits the sequence file for slave and notifies it
    * CuteGit-1-B is still waiting for Git process to end
    * CuteGit-2 quits
    * If changing a commit message or squashing a commit:
    ** Git launches process : <path-to-cutegit> <path-to-commit-file>
    ** CuteGit-2 starts in slave mode (because it has one argument)
    ** CuteGit-2-A asks (via shared memory) for commit file edition
    ** CuteGit-1-A edits the sequence file for slave and notifies it
    ** CuteGit-2 quits
    * Git quits
    * CuteGit-1-B returns

    \code

                      /)-._
                     Y. ' _]
              ,.._   |`--"=
             /    "-/   \
    /)  sk  |   |_     `\|___
    \:::::::\___/_\__\_______\

    \endcode
*/

//-------------------------------------------------------------------------------------------------

const QString sParamConfiguration = "Configuration";
const QString sParamOpenRepositories = "OpenRepositories";
const QString sParamKnownRepositories = "KnownRepositories";
const QString sParamCurrentRepository = "CurrentRepository";
const QString sParamRepository = "Repository";
const QString sParamPath = "Path";
const QString sParamName = "Name";
const QString sParamHistory = "History";
const QString sParamLastBrowsedRepositoryURL = "LastBrowsedRepositoryURL";
const QString sParamLastBrowsedRepositoryPath = "LastBrowsedRepositoryPath";
const QString sParamLanguage = "Language";
const QString sParamTheme = "Theme";

const QString CController::m_sSharedKey = "CuteGitSharedMemory";

//-------------------------------------------------------------------------------------------------

CController::CController(QString sStubFileName, QObject* parent)
    : QObject(parent)
    , m_sVersion(VERSION_STRING)
    , m_pStatusTextHistory(new QStringListModel(this))
    , m_pKnownRepositoryModel(new QStringListModel(this))
    , m_pOpenRepositoryModel(new CRepositoryModel(this))
    , m_pCurrentRepository(nullptr)
    , m_pLangModel(new QStringListModel(this))
    , m_sLanguage("en")
    , m_sStubFileName(sStubFileName)
    , m_iCurrentRepositoryIndex(-1)
    , m_bShowClean(false)
    , m_bShowAdded(true)
    , m_bShowModified(true)
    , m_bShowRenamed(true)
    , m_bShowDeleted(true)
    , m_bShowMissing(true)
    , m_bShowUntracked(false)
    , m_bShowIgnored(false)
    , m_bMasterMode(true)
    , m_tShared(m_sSharedKey, this)
    , m_tSharedTimer(this)
    , m_pTranslator(new QTranslator(this))
{
    QCoreApplication::installTranslator(m_pTranslator);
    QDir().mkpath(appDataPath());

    QStringList lLang;
    lLang << "en (English)";
    lLang << "fr (Français)";
    lLang << "de (Deutsch)";
    lLang << "es (Español)";
    lLang << "ja (日本語)";
    lLang << "zh (中文)";
    m_pLangModel->setStringList(lLang);

    loadConfiguration();

    if (m_tShared.create(SHARED_MEMORY_MAX))
    {
        clearSharedMemory();
        connect(&m_tSharedTimer, &QTimer::timeout, this, &CController::onSharedTimerTick);
        m_tSharedTimer.start(500);
    }
    else
    {
        if (m_tShared.attach())
        {
            clearSharedMemory();
            connect(&m_tSharedTimer, &QTimer::timeout, this, &CController::onSharedTimerTick);
            m_tSharedTimer.start(500);
        }
        else
        {
            qWarning() << "Could not create or attach to shared memory segment";
            qWarning() << m_tShared.errorString();
        }
    }
}

//-------------------------------------------------------------------------------------------------

CController::CController(QString sStubFileName, QString sSequenceFileName, QObject* parent)
    : QObject(parent)
    , m_sVersion(VERSION_STRING)
    , m_pStatusTextHistory(nullptr)
    , m_pKnownRepositoryModel(nullptr)
    , m_pOpenRepositoryModel(nullptr)
    , m_pCurrentRepository(nullptr)
    , m_sStubFileName(sStubFileName)
    , m_iCurrentRepositoryIndex(-1)
    , m_bShowClean(false)
    , m_bShowAdded(true)
    , m_bShowModified(true)
    , m_bShowDeleted(true)
    , m_bShowUntracked(false)
    , m_bMasterMode(false)
    , m_tShared(m_sSharedKey, this)
    , m_tSharedTimer(this)
    , m_pTranslator(new QTranslator(this))
{
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

    if (not m_tShared.detach())
    {
        qWarning() << "Could not detach from shared memory segment";
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setLanguage(QString sLang)
{
    if (not sLang.isEmpty() && m_sLanguage != sLang)
    {
        if (sLang == "en")
        {
            m_sLanguage = sLang;
            emit languageChanged();
        }
        else
        {
            if (m_pTranslator->load(QString(":/i18n/CuteGit_%1").arg(sLang)))
            {
                m_sLanguage = sLang;
                emit languageChanged();
            }
            else
            {
                qWarning() << QString("Could not set language to [%1]").arg(sLang);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setCurrentRepositoryIndex(int iValue)
{
    if (m_iCurrentRepositoryIndex != iValue)
    {
        m_iCurrentRepositoryIndex = iValue;
        emit currentRepositoryIndexChanged();

        if (m_iCurrentRepositoryIndex != -1 && m_pOpenRepositoryModel->rowCount() > 0)
        {
            CRepository* pRepository = m_pOpenRepositoryModel->repositories()[m_iCurrentRepositoryIndex];

            if (pRepository != nullptr)
                setCurrentRepository(pRepository);
        }
        else
        {
            setCurrentRepository(nullptr);
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowClean(bool bValue)
{
    if (m_bShowClean != bValue)
    {
        m_bShowClean = bValue;
        emit showCleanChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowAdded(bool bValue)
{
    if (m_bShowAdded != bValue)
    {
        m_bShowAdded = bValue;
        emit showAddedChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowModified(bool bValue)
{
    if (m_bShowModified != bValue)
    {
        m_bShowModified = bValue;
        emit showModifiedChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowRenamed(bool bValue)
{
    if (m_bShowRenamed != bValue)
    {
        m_bShowRenamed = bValue;
        emit showRenamedChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowDeleted(bool bValue)
{
    if (m_bShowDeleted != bValue)
    {
        m_bShowDeleted = bValue;
        emit showDeletedChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowMissing(bool bValue)
{
    if (m_bShowMissing != bValue)
    {
        m_bShowMissing = bValue;
        emit showMissingChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowUntracked(bool bValue)
{
    if (m_bShowUntracked != bValue)
    {
        m_bShowUntracked = bValue;
        emit showUntrackedChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CController::setShowIgnored(bool bValue)
{
    if (m_bShowIgnored != bValue)
    {
        m_bShowIgnored = bValue;
        emit showIgnoredChanged();

        m_pOpenRepositoryModel->fileFiltersChanged();
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

QString CController::appDataPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

//-------------------------------------------------------------------------------------------------

QString CController::configFileName() const
{
    return QString("%1/%2").arg(appDataPath()).arg(CONFIG_FILE_NAME);
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

    // Save history items
    CXMLNode xHistory(sParamHistory);
    xHistory.attributes()[sParamLastBrowsedRepositoryURL] = m_sLastBrowsedRepositoryURL;
    xHistory.attributes()[sParamLastBrowsedRepositoryPath] = m_sLastBrowsedRepositoryPath;
    xConfig << xHistory;

    // Save known repository list
    CXMLNode xKnownRepositories(sParamKnownRepositories);
    QStringList lKnownRepositoryPaths = m_pKnownRepositoryModel->stringList();
    for (QString sPath : lKnownRepositoryPaths)
    {
        CXMLNode xRepository(sParamRepository);
        xRepository.attributes()[sParamPath] = sPath;
        xKnownRepositories << xRepository;
    }
    xConfig << xKnownRepositories;

    // Save open repository list
    CXMLNode xOpenRepositories(sParamOpenRepositories);
    for (CRepository* pRepository : m_pOpenRepositoryModel->repositories())
    {
        CXMLNode xOpenRepository(sParamRepository);
        xOpenRepository.attributes()[sParamPath] = pRepository->repositoryPath();
        xOpenRepositories << xOpenRepository;
    }
    xConfig << xOpenRepositories;

    // Save current repository
    if (m_pCurrentRepository != nullptr)
    {
        CXMLNode xCurrentRepository(sParamCurrentRepository);
        xCurrentRepository.attributes()[sParamPath] = m_pCurrentRepository->repositoryPath();
        xConfig << xCurrentRepository;
    }

    // Save language
    CXMLNode xLanguage(sParamLanguage);
    xLanguage.attributes()[sParamName] = m_sLanguage;
    xConfig << xLanguage;

    // Save theme
    CXMLNode xTheme(sParamTheme);
    xTheme.attributes()[sParamName] = m_sTheme;
    xConfig << xTheme;

    xConfig.saveXMLToFile(configFileName());
}

//-------------------------------------------------------------------------------------------------

void CController::loadConfiguration()
{
    CXMLNode xConfig = CXMLNode::load(configFileName());

    // Load history items
    CXMLNode xHistory = xConfig.getNodeByTagName(sParamHistory);
    m_sLastBrowsedRepositoryURL = xHistory.attributes()[sParamLastBrowsedRepositoryURL];
    m_sLastBrowsedRepositoryPath = xHistory.attributes()[sParamLastBrowsedRepositoryPath];

    // Load known repository list
    CXMLNode xKnownRepositories = xConfig.getNodeByTagName(sParamKnownRepositories);
    CXMLNodeList xKnownRepositoryList = xKnownRepositories.getNodesByTagName(sParamRepository);

    QStringList lKnownRepositoryPaths;
    for (CXMLNode xRepository : xKnownRepositoryList)
    {
        lKnownRepositoryPaths << xRepository.attributes()[sParamPath];
    }
    m_pKnownRepositoryModel->setStringList(lKnownRepositoryPaths);

    // Load open repositories
    CXMLNode xOpenRepositories = xConfig.getNodeByTagName(sParamOpenRepositories);
    CXMLNodeList xOpenRepositoryList = xOpenRepositories.getNodesByTagName(sParamRepository);
    for (CXMLNode xOpenRepository : xOpenRepositoryList)
    {
        QString sPath = xOpenRepository.attributes()[sParamPath];
        openRepository(sPath);
    }

    // Load current repository
    CXMLNode xCurrentRepository = xConfig.getNodeByTagName(sParamCurrentRepository);
    m_sCurrentRepositoryPath = xCurrentRepository.attributes()[sParamPath];

    // Load language
    CXMLNode xLanguage = xConfig.getNodeByTagName(sParamLanguage);
    setLanguage(xLanguage.attributes()[sParamName]);

    // Load theme
    CXMLNode xTheme = xConfig.getNodeByTagName(sParamTheme);
    m_sTheme = xTheme.attributes()[sParamName];
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

void CController::cloneRepository(QString sRepositoryURL, QString sRepositoryPath)
{
    if (sRepositoryPath.startsWith("file:"))
        sRepositoryPath = QUrl(sRepositoryPath).toLocalFile();

    CEnums::ERepositoryType eType = CRepository::getRepositoryTypeFromURL(sRepositoryURL);

    if (eType != CEnums::UnknownRepositoryType)
    {
        // Remember the repository to show in the tab view
        m_sCurrentRepositoryPath = sRepositoryPath;

        QUrl url(sRepositoryURL);
        QString sRepositoryName = url.fileName().split(".").first();

        m_sCloneCommandsRepositoryPath = sRepositoryPath + "/" + sRepositoryName;
        m_pCloneCommands = CRepository::getCommandsForRepositoryType(this, eType);

        connect(m_pCloneCommands, &CCommands::newOutputString, this, &CController::onNewCloneOutput);

        m_pCloneCommands->cloneRepository(sRepositoryURL, sRepositoryPath);
    }
}

//-------------------------------------------------------------------------------------------------

void CController::openRepository(QString sRepositoryPath)
{
    if (sRepositoryPath.startsWith("file:"))
        sRepositoryPath = QUrl(sRepositoryPath).toLocalFile();

    if (m_pOpenRepositoryModel->hasRepository(sRepositoryPath))
        return;

    if (not sRepositoryPath.isEmpty())
    {
        if (QDir(sRepositoryPath).exists())
        {
            CEnums::ERepositoryType eType = CRepository::getRepositoryTypeFromFolder(sRepositoryPath);

            // If specified directory is a repository
            if (eType != CEnums::UnknownRepositoryType)
            {
                // Remember the repository to show in the tab view
                m_sCurrentRepositoryPath = sRepositoryPath;

                // Create a new repository item
                CRepository* pRepository = new CRepository(sRepositoryPath, this, this);

                m_pOpenRepositoryModel->addRepository(pRepository);

                // Add this path to known repository model
                QStringList lRepositoryPaths = m_pKnownRepositoryModel->stringList();
                if (not lRepositoryPaths.contains(sRepositoryPath))
                    lRepositoryPaths << sRepositoryPath;
                m_pKnownRepositoryModel->setStringList(lRepositoryPaths);

                setStatusText(QString(Strings::s_sSomeObjectNameOpened).arg(pRepository->repositoryName()));
            }
            else
            {
                setStatusText(QString(Strings::s_sSomeObjectNameIsNotARepository).arg(sRepositoryPath));
            }
        }
        else
        {
            // Clears this path from known repository model
            QStringList lKnownRepositoryPaths = m_pKnownRepositoryModel->stringList();
            lKnownRepositoryPaths.removeAll(sRepositoryPath);
            m_pKnownRepositoryModel->setStringList(lKnownRepositoryPaths);

            setStatusText(QString(Strings::s_sSomeObjectDoesNotExist).arg(sRepositoryPath));
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CController::removeRepository(int iRepositoryIndex)
{
    if (iRepositoryIndex < m_pOpenRepositoryModel->repositories().count())
    {
        QString sName = m_pOpenRepositoryModel->repositories()[iRepositoryIndex]->repositoryName();
        setStatusText(QString(Strings::s_sSomeObjectNameClosed).arg(sName));

        m_pOpenRepositoryModel->removeRepository(iRepositoryIndex);
    }
}

//-------------------------------------------------------------------------------------------------

int CController::currentRepositoryIndexToSet()
{
    int iReturnValue = -1;

    if (not m_sCurrentRepositoryPath.isEmpty())
    {
        for (int iIndex = 0; iIndex < m_pOpenRepositoryModel->repositories().count(); iIndex++)
        {
            if (m_pOpenRepositoryModel->repositories()[iIndex]->repositoryPath() == m_sCurrentRepositoryPath)
            {
                m_sCurrentRepositoryPath.clear();
                iReturnValue = iIndex;
                break;
            }
        }
    }

    return iReturnValue;
}

//-------------------------------------------------------------------------------------------------

QString CController::repositoryNameFromPath(const QString& sPath) const
{
    if (sPath.isEmpty())
        return "";

    return sPath.split(PATH_SEP).last();
}

//-------------------------------------------------------------------------------------------------

QString CController::browseForExistingFolder(QString sTitle)
{
    return QFileDialog::getExistingDirectory(nullptr, sTitle, lastBrowsedRepositoryPath());
}

//-------------------------------------------------------------------------------------------------

void CController::onNewCloneOutput(CEnums::EProcessCommand eCommand, QString sOutput)
{
    switch (eCommand)
    {
    default:
    case CEnums::eCloneRepository:
    {
        QStringList lNewTextLines = sOutput.split(NEW_LINE);
        setStatusText(lNewTextLines.last());

        QStringList lHistory = m_pStatusTextHistory->stringList();
        lHistory << lNewTextLines;
        while (lHistory.count() > 50)
            lHistory.takeFirst();
        m_pStatusTextHistory->setStringList(lHistory);
        break;
    }

    case CEnums::eCloneRepositoryFinished:
    {
        if (m_pCloneCommands != nullptr)
            delete m_pCloneCommands;

        m_pCloneCommands = nullptr;

        // Open the cloned repository
        if (true)
        {
            openRepository(m_sCloneCommandsRepositoryPath);
        }

        break;
    }
    }
}

//-------------------------------------------------------------------------------------------------

void CController::onSharedTimerTick()
{
    if (m_bMasterMode)
    {
        // This code is executed by a master instance

        if (sharedOperation() == eSOSlaveRequestEdit)
        {
            QString sFileName = sequenceFileName();

            if (m_pCurrentRepository != nullptr && m_pCurrentRepository->commands() != nullptr)
                m_pCurrentRepository->commands()->editSequenceFile(sFileName);

            setSharedOperation(eSOMasterFinishedEdit);
        }
    }
    else
    {
        // This code is executed by a slave instance

        if (sharedOperation() == eSOMasterFinishedEdit)
        {
            setSharedOperation(eSONone);

            quit();
        }
    }
}
