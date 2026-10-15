// Qt
#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>

// Application
#include "CController.h"
#include "CLayout.h"
#include "CUtils.h"
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

const QString sParameterConfiguration = "Configuration";
const QString sParameterOpenRepositories = "OpenRepositories";
const QString sParameterKnownRepositories = "KnownRepositories";
const QString sParameterCurrentRepository = "CurrentRepository";
const QString sParameterRepository = "Repository";
const QString sParameterPath = "Path";
const QString sParameterName = "Name";
const QString sParameterHistory = "History";
const QString sParameterLastBrowsedRepositoryURL = "LastBrowsedRepositoryURL";
const QString sParameterLastBrowsedRepositoryPath = "LastBrowsedRepositoryPath";
const QString sParameterLanguage = "Language";
const QString sParameterTheme = "Theme";
const QString sParameterLayoutIndex = "LayoutIndex";
const QString sThemeMaterialDark = "MaterialDark";
const QString sThemeMaterialLight = "MaterialLight";

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
    , m_sTheme(sThemeMaterialDark)
    , m_sStubFileName(sStubFileName)
    , m_lLayoutNames()
    , m_iCurrentLayoutIndex(0)
    , m_lCurrentLayoutPanels()
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
    , m_tPollTimer(this)
    , m_iPollIndex(0)
    , m_pCloneCommands(nullptr)
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

    reloadLayouts();
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

    connect(&m_tPollTimer, &QTimer::timeout, this, &CController::onPollTimerTick);
    m_tPollTimer.start(REPOSITORY_POLL_INTERVAL_MS);
}

//-------------------------------------------------------------------------------------------------

CController::CController(QString sStubFileName, QString sSequenceFileName, QObject* parent)
    : QObject(parent)
    , m_sVersion(VERSION_STRING)
    , m_pStatusTextHistory(nullptr)
    , m_pKnownRepositoryModel(nullptr)
    , m_pOpenRepositoryModel(nullptr)
    , m_pCurrentRepository(nullptr)
    , m_pLangModel(nullptr)
    , m_sLanguage("en")
    , m_sTheme(sThemeMaterialDark)
    , m_sStubFileName(sStubFileName)
    , m_lLayoutNames()
    , m_iCurrentLayoutIndex(0)
    , m_lCurrentLayoutPanels()
    , m_iCurrentRepositoryIndex(-1)
    , m_bShowClean(false)
    , m_bShowAdded(true)
    , m_bShowModified(true)
    , m_bShowRenamed(true)
    , m_bShowDeleted(true)
    , m_bShowMissing(true)
    , m_bShowUntracked(false)
    , m_bShowIgnored(false)
    , m_bMasterMode(false)
    , m_tShared(m_sSharedKey, this)
    , m_tSharedTimer(this)
    , m_tPollTimer(this)
    , m_iPollIndex(0)
    , m_pCloneCommands(nullptr)
    , m_pTranslator(new QTranslator(this))
{
    reloadLayouts();

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
            {
                setCurrentRepository(pRepository);

                if (pRepository->needsRefresh())
                    pRepository->refresh();
            }
        }
        else
        {
            setCurrentRepository(nullptr);
        }
    }
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn static int clampLayoutIndex(int iValue, int iCount)
    Clamps a layout index to the valid range, single place for the clamp logic.
*/
static int clampLayoutIndex(int iValue, int iCount)
{
    if (iCount <= 0)
        return 0;
    if (iValue < 0)
        return 0;
    if (iValue >= iCount)
        return iCount - 1;
    return iValue;
}

//-------------------------------------------------------------------------------------------------

void CController::setCurrentLayoutIndex(int iValue)
{
    if (m_vLayouts.isEmpty())
        return;

    int iClampedValue = clampLayoutIndex(iValue, m_vLayouts.count());
    bool bIndexChanged = (m_iCurrentLayoutIndex != iClampedValue);

    if (bIndexChanged || m_lCurrentLayoutPanels.isEmpty())
    {
        m_iCurrentLayoutIndex = iClampedValue;
        if (bIndexChanged)
            emit currentLayoutIndexChanged();
        setCurrentLayoutPanels(CLayout::toVariantPanels(m_vLayouts[m_iCurrentLayoutIndex]));
    }
}

//-------------------------------------------------------------------------------------------------

void CController::reloadLayouts()
{
    static const QString sLayoutsLoadedMessage = "Layouts loaded from %1";

    QStringList lCandidates;
    lCandidates << QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg(CLayout::sFileName);
    lCandidates << QString(CLayout::sResourceFileName);

    QString sError;
    QString sUsedFile;
    QVector<SLayoutDefinition> vLayoutDefinitions;
    for (const QString& sCandidate : lCandidates)
    {
        QString sCandidateError;
        QVector<SLayoutDefinition> vCandidate = CLayout::loadLayouts(sCandidate, &sCandidateError);
        if (sCandidateError.isEmpty())
        {
            vLayoutDefinitions = vCandidate;
            sUsedFile = sCandidate;
            break;
        }
        sError = sCandidateError;
    }

    if (vLayoutDefinitions.isEmpty())
        vLayoutDefinitions = CLayout::defaultLayouts();
    m_vLayouts = vLayoutDefinitions;

    if (sError.isEmpty() == false)
    {
        qWarning() << sError;
        setStatusText(sError);
    }
    else if (sUsedFile.isEmpty() == false)
    {
        setStatusText(sLayoutsLoadedMessage.arg(sUsedFile));
    }

    QStringList lLayoutNames;
    for (const SLayoutDefinition& tLayoutDefinition : m_vLayouts)
        lLayoutNames << tLayoutDefinition.sName;
    setLayoutNames(lLayoutNames);

    int iIndex = clampLayoutIndex(m_iCurrentLayoutIndex, m_vLayouts.count());
    m_iCurrentLayoutIndex = -1;
    setCurrentLayoutIndex(iIndex);
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
    CXMLNode xConfig(sParameterConfiguration);

    // Save history items
    CXMLNode xHistory(sParameterHistory);
    xHistory.attributes()[sParameterLastBrowsedRepositoryURL] = m_sLastBrowsedRepositoryURL;
    xHistory.attributes()[sParameterLastBrowsedRepositoryPath] = m_sLastBrowsedRepositoryPath;
    xConfig << xHistory;

    // Save known repository list
    CXMLNode xKnownRepositories(sParameterKnownRepositories);
    QStringList lKnownRepositoryPaths = m_pKnownRepositoryModel->stringList();
    for (QString sPath : lKnownRepositoryPaths)
    {
        CXMLNode xRepository(sParameterRepository);
        xRepository.attributes()[sParameterPath] = sPath;
        xKnownRepositories << xRepository;
    }
    xConfig << xKnownRepositories;

    // Save open repository list
    CXMLNode xOpenRepositories(sParameterOpenRepositories);
    for (CRepository* pRepository : m_pOpenRepositoryModel->repositories())
    {
        CXMLNode xOpenRepository(sParameterRepository);
        xOpenRepository.attributes()[sParameterPath] = pRepository->repositoryPath();
        xOpenRepositories << xOpenRepository;
    }
    xConfig << xOpenRepositories;

    // Save current repository
    if (m_pCurrentRepository != nullptr)
    {
        CXMLNode xCurrentRepository(sParameterCurrentRepository);
        xCurrentRepository.attributes()[sParameterPath] = m_pCurrentRepository->repositoryPath();
        xConfig << xCurrentRepository;
    }

    // Save language
    CXMLNode xLanguage(sParameterLanguage);
    xLanguage.attributes()[sParameterName] = m_sLanguage;
    xConfig << xLanguage;

    // Save theme
    CXMLNode xTheme(sParameterTheme);
    xTheme.attributes()[sParameterName] = m_sTheme;
    xConfig << xTheme;

    // Save layout index
    CXMLNode xLayout(sParameterLayoutIndex);
    xLayout.attributes()[sParameterName] = QString::number(m_iCurrentLayoutIndex);
    xConfig << xLayout;

    xConfig.saveXMLToFile(configFileName());
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn CRepository* CController::addOpenRepository(const QString& sRepositoryPath)
    Validates sRepositoryPath and appends a lazy repository item to the open
    model. The item issues no git command: refresh() runs when the tab
    becomes visible. Returns null when the folder holds no known repository.
*/
CRepository* CController::addOpenRepository(const QString& sRepositoryPath)
{
    if (sRepositoryPath.isEmpty() || not QDir(sRepositoryPath).exists())
        return nullptr;

    if (m_pOpenRepositoryModel->hasRepository(sRepositoryPath))
        return nullptr;

    CEnums::ERepositoryType eType = CRepository::getRepositoryTypeFromFolder(sRepositoryPath);

    if (eType == CEnums::UnknownRepositoryType)
        return nullptr;

    // Create a new repository item
    CRepository* pRepository = new CRepository(sRepositoryPath, this, this);
    connect(pRepository, &CRepository::statusTextRequested, this, &CController::setStatusText);

    m_pOpenRepositoryModel->addRepository(pRepository);

    // Add this path to known repository model
    QStringList lRepositoryPaths = m_pKnownRepositoryModel->stringList();
    if (not lRepositoryPaths.contains(sRepositoryPath))
        lRepositoryPaths << sRepositoryPath;
    m_pKnownRepositoryModel->setStringList(lRepositoryPaths);

    return pRepository;
}

//-------------------------------------------------------------------------------------------------

void CController::loadConfiguration()
{
    CXMLNode xConfig = CXMLNode::load(configFileName());

    // Load history items
    CXMLNode xHistory = xConfig.getNodeByTagName(sParameterHistory);
    m_sLastBrowsedRepositoryURL = xHistory.attributes()[sParameterLastBrowsedRepositoryURL];
    m_sLastBrowsedRepositoryPath = xHistory.attributes()[sParameterLastBrowsedRepositoryPath];

    // Load known repository list
    CXMLNode xKnownRepositories = xConfig.getNodeByTagName(sParameterKnownRepositories);
    CXMLNodeList xKnownRepositoryList = xKnownRepositories.getNodesByTagName(sParameterRepository);

    QStringList lKnownRepositoryPaths;
    for (CXMLNode xRepository : xKnownRepositoryList)
    {
        lKnownRepositoryPaths << xRepository.attributes()[sParameterPath];
    }
    m_pKnownRepositoryModel->setStringList(lKnownRepositoryPaths);

    // Load open repositories without scanning them: only the visible tab
    // refreshes, once the QML view selects it through currentRepositoryIndex
    CXMLNode xOpenRepositories = xConfig.getNodeByTagName(sParameterOpenRepositories);
    CXMLNodeList xOpenRepositoryList = xOpenRepositories.getNodesByTagName(sParameterRepository);
    for (CXMLNode xOpenRepository : xOpenRepositoryList)
    {
        QString sPath = CUtils::localFilePath(xOpenRepository.attributes()[sParameterPath]);
        addOpenRepository(sPath);
    }

    // Load current repository
    CXMLNode xCurrentRepository = xConfig.getNodeByTagName(sParameterCurrentRepository);
    m_sCurrentRepositoryPath = xCurrentRepository.attributes()[sParameterPath];

    // Load language
    CXMLNode xLanguage = xConfig.getNodeByTagName(sParameterLanguage);
    setLanguage(xLanguage.attributes()[sParameterName]);

    // Load theme
    CXMLNode xTheme = xConfig.getNodeByTagName(sParameterTheme);
    m_sTheme = xTheme.attributes()[sParameterName];
    if (m_sTheme != sThemeMaterialDark && m_sTheme != sThemeMaterialLight)
        m_sTheme = sThemeMaterialDark;

    // Load layout index
    CXMLNode xLayout = xConfig.getNodeByTagName(sParameterLayoutIndex);
    bool bLayoutConversionSucceeded = false;
    int iLayoutIndex = xLayout.attributes()[sParameterName].toInt(&bLayoutConversionSucceeded);
    if (bLayoutConversionSucceeded)
        setCurrentLayoutIndex(iLayoutIndex);
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
    sRepositoryPath = CUtils::localFilePath(sRepositoryPath);

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
    sRepositoryPath = CUtils::localFilePath(sRepositoryPath);

    if (m_pOpenRepositoryModel->hasRepository(sRepositoryPath))
        return;

    if (not sRepositoryPath.isEmpty())
    {
        if (QDir(sRepositoryPath).exists())
        {
            // Remember the repository to show in the tab view
            m_sCurrentRepositoryPath = sRepositoryPath;

            CRepository* pRepository = addOpenRepository(sRepositoryPath);

            // If specified directory is a repository
            if (pRepository != nullptr)
            {
                // Interactive open scans immediately, background tabs stay lazy
                pRepository->refresh();

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

void CController::onPollTimerTick()
{
    if (m_pOpenRepositoryModel == nullptr)
        return;

    QList<CRepository*> lRepositories = m_pOpenRepositoryModel->repositories();

    if (lRepositories.isEmpty())
        return;

    // The current repository is polled on every tick, other repositories round-robin
    // so that any number of open repositories costs at most two cheap commands per tick
    if (m_pCurrentRepository != nullptr)
        m_pCurrentRepository->pollExternalChanges();

    m_iPollIndex = m_iPollIndex % lRepositories.count();

    CRepository* pRepository = lRepositories[m_iPollIndex];
    m_iPollIndex++;

    if (pRepository != nullptr && pRepository != m_pCurrentRepository)
        pRepository->pollExternalChanges();
}

//-------------------------------------------------------------------------------------------------

void CController::onSharedTimerTick()
{
    if (m_bMasterMode)
    {        // This code is executed by a master instance

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
