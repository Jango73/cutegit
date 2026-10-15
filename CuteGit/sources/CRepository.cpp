
// Qt
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QUrl>

// Application
#include "CUtils.h"
#include "CController.h"
#include "Strings.h"
#include "CFlatFileModel.h"
#include "CFlatFileModelProxy.h"
#include "CStagedFileModelProxy.h"
#include "CBranchModel.h"
#include "CRepoFile.h"
#include "CLogModel.h"
#include "CLogModelProxy.h"
#include "CDiffModel.h"
#include "CDiffModelProxy.h"
#include "CGraphModel.h"
#include "CBranch.h"
#include "CLogLineCollection.h"
#include "CDiffLine.h"
#include "CGraphLine.h"
#include "commands/CGitCommands.h"
#include "commands/CGerritCommands.h"
#include "commands/CSvnCommands.h"
#include "commands/CHgCommands.h"

//-------------------------------------------------------------------------------------------------

/*!
    \class CRepository
    \inmodule CuteGit
    \section1 How this class works
    Whenever the user triggers a repository command from the UI:
    \list
    \li A method from this class is called.
    \li The method in turn calls one or more methods in CCommands.
    \li CCommands executes some process and emits a result.
    \li The result is caught in this class' slots, like onNewOutputString.
    \li The data is put into models which in turn notify the UI.

        ___
     __/_  `.  .-"""-.
     \_,` | \-'  /   )`-')
      "") `"`    \  ((`"`
     ___Y  ,    .'7 /|
    (_,___/...-` (_/_/ sk
*/

//-------------------------------------------------------------------------------------------------

CRepository::CRepository(const QString& sPath, CController* pController, QObject* parent, CEnums::ERepositoryType eType)
    : QObject(parent)
    , m_eRepositoryType(CEnums::UnknownRepositoryType)
    , m_eRepositoryStatus(CEnums::NoMerge)
    , m_sRepositoryPath(sPath)
    , m_pController(pController)
    , m_pCommands(nullptr)
    , m_pFlatFileModel(new CFlatFileModel(this, this))
    , m_pFlatFileModelProxy(new CFlatFileModelProxy(pController, this))
    , m_pStagedFileModelProxy(new CStagedFileModelProxy(pController, this))
    , m_pRemoteBranchModel(new CBranchModel(this))
    , m_pLocalBranchModel(new CBranchModel(this))
    , m_pTagModel(new CBranchModel(this))
    , m_pBranchLogModel(new CLogModel(this, this))
    , m_pBranchLogModelProxy(new CLogModelProxy(this, this))
    , m_pFileLogModel(new CLogModel(this, this))
    , m_pGraphModel(new CGraphModel(this, this))
    , m_pRefLogModel(new CLogModel(this, this))
    , m_pFileDiffModel(new CDiffModel(this))
    , m_pFileDiffModelProxy(new CDiffModelProxy(this, this))
    , m_pStagedFileDiffModel(new CDiffModel(this))
    , m_pStagedFileDiffModelProxy(new CDiffModelProxy(this, this))
    , m_pUnstagedFileDiffModel(new CDiffModel(this))
    , m_pUnstagedFileDiffModelProxy(new CDiffModelProxy(this, this))
    , m_pFileBlameModel(new CDiffModel(this))
    , m_pFileBlameModelProxy(new CDiffModelProxy(this, this))
    , m_pCommandOutputModel(new QStringListModel(this))
    , m_iMaxCommandOutputLines(100)
    , m_iCommitCountAhead(0)
    , m_iCommitCountBehind(0)
    , m_bHasModifiedFiles(false)
    , m_bHasCommitableFiles(false)
    , m_bHasPushableCommits(false)
    , m_bHasPullableCommits(false)
    , m_bHasStashableFiles(false)
    , m_sPollFingerprint("")
    , m_bNeedsRefresh(true)
    , m_bFileInventoryDone(false)
    , m_bPollPending(false)
{
    if (eType == CEnums::UnknownRepositoryType)
    {
        eType = getRepositoryTypeFromFolder(sPath);
    }

    setRepositoryType(eType);

    m_sRepositoryName = sPath.split(PATH_SEP).last();

    // Connect versioning system command signals
    connect(m_pCommands, &CCommands::newOutputString, this, &CRepository::onNewOutputString);
    connect(m_pCommands, &CCommands::newOutputKeyValue, this, &CRepository::onNewOutputKeyValue);
    connect(m_pCommands, &CCommands::newOutputStringList, this, &CRepository::onNewOutputStringList);
    connect(m_pCommands, &CCommands::newOutputListOfCBranch, this, &CRepository::onNewOutputListOfCBranch);
    connect(m_pCommands, &CCommands::newOutputListOfCRepoFile, this, &CRepository::onNewOutputListOfCRepoFile);
    connect(m_pCommands, &CCommands::newOutputCLogLineCollection, this, &CRepository::onNewOutputCLogLineCollection);
    connect(m_pCommands, &CCommands::newOutputListOfCDiffLine, this, &CRepository::onNewOutputListOfCDiffLine);
    connect(m_pCommands, &CCommands::newOutputCGraphLineCollection, this, &CRepository::onNewOutputCGraphLineCollection);

    // Connect model signals
    connect(m_pBranchLogModel, &CLogModel::requestLogData, this, &CRepository::onRequestBranchLogData);
    connect(m_pFileLogModel, &CLogModel::requestLogData, this, &CRepository::onRequestFileLogData);
    connect(m_pGraphModel, &CGraphModel::requestGraphData, this, &CRepository::onRequestGraphData);
    connect(m_pFlatFileModel, &CFlatFileModel::currentFileFullName, this, &CRepository::onCurrentFileFullName);

    // Connect proxies with their models
    m_pFlatFileModelProxy->setSourceModel(m_pFlatFileModel);
    m_pStagedFileModelProxy->setSourceModel(m_pFlatFileModel);
    m_pBranchLogModelProxy->setSourceModel(m_pBranchLogModel);
    m_pFileDiffModelProxy->setSourceModel(m_pFileDiffModel);
    m_pStagedFileDiffModelProxy->setSourceModel(m_pStagedFileDiffModel);
    m_pUnstagedFileDiffModelProxy->setSourceModel(m_pUnstagedFileDiffModel);
    m_pFileBlameModelProxy->setSourceModel(m_pFileBlameModel);

    // Connect signals
    connect(this, &CRepository::diffToCommitIdChanged, this, &CRepository::onDiffCommitIdChanged);
    connect(this, &CRepository::diffFromCommitIdChanged, this, &CRepository::onDiffCommitIdChanged);

    // Fill-up of model data is lazy: the controller issues refresh() when the
    // repository becomes visible, so opening N repositories costs nothing here
}

//-------------------------------------------------------------------------------------------------

CRepository::~CRepository()
{
    qDeleteAll(m_lRepoFiles);

    if (m_pCommands != nullptr)
        delete m_pCommands;
}

//-------------------------------------------------------------------------------------------------

void CRepository::setRepositoryType(CEnums::ERepositoryType eType)
{
    if (m_eRepositoryType != eType)
    {
        m_eRepositoryType = eType;

        if (m_pCommands != nullptr)
            delete m_pCommands;

        // Create the command interface
        m_pCommands = getCommandsForRepositoryType(m_pController, m_eRepositoryType);

        emit repositoryTypeChanged();
        emit repositoryTypeStringChanged();
    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::setCurrentBranch(QString sValue)
{
    if (m_sCurrentBranch != sValue)
    {
        m_pCommands->setCurrentBranch(m_sRepositoryPath, sValue);
    }
}

//-------------------------------------------------------------------------------------------------

CRepoFile* CRepository::fileByFullName(const QString& sFullName) const
{
    auto itr = std::find_if(m_lRepoFiles.begin(), m_lRepoFiles.end(), [sFullName](CRepoFile* pFile) { return pFile->fullName() == sFullName; });

    if (itr != m_lRepoFiles.end())
        return *itr;

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

bool CRepository::isLoaded() const
{
    return m_bFileInventoryDone;
}

//-------------------------------------------------------------------------------------------------

QList<CLabel*> CRepository::labelsForCommit(const QString& sCommitId) const
{
    QList<CLabel*> lReturnValue;

    lReturnValue << m_pRemoteBranchModel->labelsForCommit(sCommitId);
    lReturnValue << m_pLocalBranchModel->labelsForCommit(sCommitId);
    lReturnValue << m_pTagModel->labelsForCommit(sCommitId);

    return lReturnValue;
}

//-------------------------------------------------------------------------------------------------

QString CRepository::repositoryTypeString() const
{
    switch(m_eRepositoryType)
    {
    case CEnums::UnknownRepositoryType: return "?";
    case CEnums::Git: return "Git";
    case CEnums::Gerrit: return "Gerrit";
    case CEnums::CVS: return "CVS";
    case CEnums::SVN: return "SVN";
    case CEnums::HG: return "Mercurial";
    }

    return "";
}

//-------------------------------------------------------------------------------------------------

void CRepository::clearOutput()
{
    m_pCommandOutputModel->setStringList(QStringList());
}

//-------------------------------------------------------------------------------------------------

void CRepository::copyText(const QString& sText)
{
    QClipboard* pClipboard = QGuiApplication::clipboard();
    pClipboard->setText(sText);
}


//-------------------------------------------------------------------------------------------------

void CRepository::openFiles(QStringList lFullNames)
{
    for (QString sFullName : lFullNames)
        QDesktopServices::openUrl(QUrl(sFullName));
}

//-------------------------------------------------------------------------------------------------

bool CRepository::can(CEnums::ECapability eWhat)
{
    return m_pCommands->can(eWhat);
}

//-------------------------------------------------------------------------------------------------

void CRepository::checkRepositoryStatus(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->repositoryStatus(sPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::checkAllFileStatus(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->allFileStatus(sPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::checkChangedFileStatus(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->changedFileStatus(sPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::checkFileStatus(const QString& sFileFullName)
{
    m_pCommands->fileStatus(m_sRepositoryPath, sFileFullName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::refresh()
{
    // The clean file inventory runs once, on the first visible refresh: it
    // crawls the working folder and must not run for background tabs
    if (not m_bFileInventoryDone)
    {
        m_bFileInventoryDone = true;
        checkAllFileStatus();
    }

    setDiffFromCommitId("");
    setDiffToCommitId("");
    setNeedsRefresh(false);

    checkRepositoryStatus();
    checkChangedFileStatus();
    getBranches();
    getTags();
    getBranchLog();
    getGraph();
    getRefLog();
}

//-------------------------------------------------------------------------------------------------

void CRepository::pollExternalChanges()
{
    // Never loaded repositories already need a refresh, polling them would
    // spend one git process per tab for no information
    if (not m_bFileInventoryDone)
        return;

    if (m_bPollPending)
        return;

    if (m_pCommands == nullptr || m_pCommands->isBusy())
        return;

    m_bPollPending = true;
    m_pCommands->pollFingerprint(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::toggleStaged(QStringList lFullNames)
{
    m_pFlatFileModel->setFilesChangingByFullName(lFullNames, true);

    for (QString sFullName : lFullNames)
    {
        if (not sFullName.isEmpty())
        {
            m_pCommands->toggleStaged(m_sRepositoryPath, sFullName);
            checkChangedFileStatus();
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::deleteFiles(QStringList lFullNames)
{
    m_pFlatFileModel->setFilesChangingByFullName(lFullNames, true);

    for (QString sFullName : lFullNames)
    {
        if (not sFullName.isEmpty())
        {
            CRepoFile* pFile = fileByFullName(sFullName);

            if (pFile->status() == CEnums::eUntracked)
            {
                QFile file(sFullName);
                file.remove();
            }
            else
            {
                m_pCommands->deleteFile(m_sRepositoryPath, sFullName);
            }

            checkChangedFileStatus();
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::stageSelection(QStringList lFullNames)
{
    m_pFlatFileModel->setFilesChangingByFullName(lFullNames, true);

    for (QString sFullName : lFullNames)
    {
        if (not sFullName.isEmpty())
        {
            m_pCommands->stageFile(m_sRepositoryPath, sFullName, true);
        }
    }
    checkChangedFileStatus();
}

//-------------------------------------------------------------------------------------------------

void CRepository::unstageSelection(QStringList lFullNames)
{
    m_pFlatFileModel->setFilesChangingByFullName(lFullNames, true);

    for (QString sFullName : lFullNames)
    {
        if (not sFullName.isEmpty())
        {
            m_pCommands->stageFile(m_sRepositoryPath, sFullName, false);
        }
    }
    checkChangedFileStatus();
}

//-------------------------------------------------------------------------------------------------

void CRepository::stageAll()
{
    m_pFlatFileModel->setAllFilesChanging(true);
    m_pCommands->stageAll(m_sRepositoryPath, true);
    checkChangedFileStatus();
}

//-------------------------------------------------------------------------------------------------

void CRepository::unstageAll()
{
    m_pFlatFileModel->setAllFilesChanging(true);
    m_pCommands->stageAll(m_sRepositoryPath, false);
    checkChangedFileStatus();
}

//-------------------------------------------------------------------------------------------------

void CRepository::revertSelection(QStringList lFullNames)
{
    m_pFlatFileModel->setFilesChangingByFullName(lFullNames, true);

    for (QString sFullName : lFullNames)
    {
        CRepoFile* pFile = fileByFullName(sFullName);

        if (pFile != nullptr)
        {
            if (pFile->status() == CEnums::eDeleted)
                m_pCommands->undeleteFile(m_sRepositoryPath, sFullName);
            else
                m_pCommands->revertFile(m_sRepositoryPath, sFullName);
        }
    }
    checkChangedFileStatus();
}

//-------------------------------------------------------------------------------------------------

void CRepository::commit(const QString& sMessage, bool bAmend)
{
    setStatusText(bAmend ? Strings::s_sAmendStarted : Strings::s_sCommitStarted);

    if (not bAmend)
    {
        m_pCommands->commit(m_sRepositoryPath, sMessage);
    }
    else
    {
        m_pCommands->amend(m_sRepositoryPath);
    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::continueRebase()
{
    m_pCommands->continueRebase(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::abortRebase()
{
    m_pCommands->abortRebase(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::push()
{
    setStatusText(Strings::s_sPushStarted);

    m_pCommands->push(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::forcePush()
{
    setStatusText(Strings::s_sPushStarted);

    m_pCommands->forcePush(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::pushAsWIP()
{
    setStatusText(Strings::s_sPushStarted);

    m_pCommands->pushAsWIP(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::pull()
{
    setStatusText(Strings::s_sPullStarted);

    m_pCommands->pull(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::fetch()
{
    setStatusText(Strings::s_sFetchStarted);

    m_pCommands->fetch(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::stashSave()
{
    m_pCommands->stashSave(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::stashPop()
{
    m_pCommands->stashPop(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::commitReset(const QString& sCommitId)
{
    m_pCommands->commitReset(m_sRepositoryPath, sCommitId);
}

//-------------------------------------------------------------------------------------------------

void CRepository::commitRebase(const QString& sCommitId)
{
    m_pCommands->commitRebase(m_sRepositoryPath, sCommitId);
}

//-------------------------------------------------------------------------------------------------

void CRepository::commitSquash(const QString& sCommitId)
{
    m_pCommands->commitSquash(m_sRepositoryPath, sCommitId);
}

//-------------------------------------------------------------------------------------------------

void CRepository::commitBranchFrom(const QString& sCommitId, const QString& sBranchName)
{
    m_pCommands->createBranchOnCommit(m_sRepositoryPath, sCommitId, sBranchName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::changeCommitMessage(const QString& sCommitId, const QString& sMessage)
{
    m_pCommands->changeCommitMessage(m_sRepositoryPath, sCommitId, sMessage);
}

//-------------------------------------------------------------------------------------------------

void CRepository::rebaseOnBranch(const QString& sName)
{
    m_pCommands->rebaseOnBranch(m_sRepositoryPath, sName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::mergeBranch(const QString& sName)
{
    m_pCommands->mergeBranch(m_sRepositoryPath, sName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::deleteBranch(const QString& sName)
{
    m_pCommands->deleteBranch(m_sRepositoryPath, sName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::createTagOnCommit(const QString& sCommitId, const QString& sTagName, const QString& sMessage)
{
    m_pCommands->createTagOnCommit(m_sRepositoryPath, sCommitId, sTagName, sMessage);
}

//-------------------------------------------------------------------------------------------------

void CRepository::commitDiffPrevious(const QString& sCommitId)
{
    m_pCommands->commitDiffPrevious(m_sRepositoryPath, sCommitId);
}

//-------------------------------------------------------------------------------------------------

void CRepository::blame(QString sFileFullName)
{
    m_pFileBlameModel->setFullSourceName(sFileFullName);
    m_pFileBlameModel->setRelativeSourceName(relativeFileName(sFileFullName));

    m_pCommands->blame(m_sRepositoryPath, sFileFullName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::cleanUp()
{
    m_pCommands->cleanUp(m_sRepositoryPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::setFileFilter(const QString& sText)
{
    m_pFlatFileModelProxy->setNameFilter(sText);
}

//-------------------------------------------------------------------------------------------------

void CRepository::setFileSortField(CEnums::ESortField eField)
{
    m_pFlatFileModelProxy->setSortField(eField);
}

//-------------------------------------------------------------------------------------------------

void CRepository::setFileSortDirection(bool bDirection)
{
    m_pFlatFileModelProxy->setSortDirection(bDirection);
}

//-------------------------------------------------------------------------------------------------

void CRepository::setWorkingTreeDiffFilter(const QString& sText)
{
    m_pStagedFileDiffModelProxy->setTextFilter(sText);
    m_pStagedFileDiffModelProxy->filterChanged();
    m_pUnstagedFileDiffModelProxy->setTextFilter(sText);
    m_pUnstagedFileDiffModelProxy->filterChanged();
}

/************************************************************************/
/*!
    \fn void CRepository::setCommitDiffFilter(const QString& sText)
    Sets the text filter of the commit diff view.
*/
void CRepository::setCommitDiffFilter(const QString& sText)
{
    m_pFileDiffModelProxy->setTextFilter(sText);
    m_pFileDiffModelProxy->filterChanged();
}

//-------------------------------------------------------------------------------------------------

void CRepository::setFileBlameFilter(const QString& sText)
{
    m_pFileBlameModelProxy->setTextFilter(sText);
    m_pFileBlameModelProxy->filterChanged();
}

//-------------------------------------------------------------------------------------------------

void CRepository::setBranchLogFilter(const QString& sText)
{
    m_pBranchLogModelProxy->setTextFilter(sText);
    m_pBranchLogModelProxy->filterChanged();
}

//-------------------------------------------------------------------------------------------------

CEnums::ERepositoryType CRepository::getRepositoryTypeFromFolder(const QString& sPath)
{
    // Check for Git and Gerrit repository
    if (QDir(QString("%1/.git").arg(sPath)).exists())
    {
        QString sCommitMsgFile = QString("%1/.git/hooks/commit-msg").arg(sPath);
        QString sCommitMsgContents = CUtils::getTextFileContents(sCommitMsgFile);

        if (sCommitMsgContents.contains("Gerrit") && sCommitMsgContents.contains("add_ChangeId()"))
            return CEnums::Gerrit;

        return CEnums::Git;
    }

    // Check for SVN repository
    if (QDir(QString("%1/.svn").arg(sPath)).exists())
    {
        return CEnums::SVN;
    }

    // Check for Mercurial repository
    if (QDir(QString("%1/.hg").arg(sPath)).exists())
    {
        return CEnums::HG;
    }

    // Check for CVS repository
    // TODO

    // Don't know about this repository
    return CEnums::UnknownRepositoryType;
}

//-------------------------------------------------------------------------------------------------

CEnums::ERepositoryType CRepository::getRepositoryTypeFromURL(const QString& sRepositoryURL)
{
    if (sRepositoryURL.contains(".git"))
        return CEnums::Git;

    if (sRepositoryURL.contains("gerrit"))
        return CEnums::Gerrit;

    if (sRepositoryURL.contains("/svn"))
        return CEnums::SVN;

    if (sRepositoryURL.contains("/hg"))
        return CEnums::HG;

    return CEnums::UnknownRepositoryType;
}

//-------------------------------------------------------------------------------------------------

CCommands* CRepository::getCommandsForRepositoryType(CController* pController, CEnums::ERepositoryType eType)
{
    switch (eType)
    {

    case CEnums::Git:
        return new CGitCommands(pController);

    case CEnums::Gerrit:
        return new CGerritCommands(pController);

    case CEnums::SVN:
        return new CSvnCommands(pController);

    case CEnums::HG:
        return new CHgCommands(pController);

    default:
        return new CCommands(pController);

    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::getBranches(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->branches(sPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::getBranchHeadCommits(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->branchHeadCommits(sPath, m_pRemoteBranchModel->branchNames());
    m_pCommands->branchHeadCommits(sPath, m_pLocalBranchModel->branchNames());
}

//-------------------------------------------------------------------------------------------------

void CRepository::getBranchesCommitCountAheadBehind(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->branchCommitCountAheadBehind(sPath, m_pRemoteBranchModel->branchNames());
    m_pCommands->branchCommitCountAheadBehind(sPath, m_pLocalBranchModel->branchNames());
}

//-------------------------------------------------------------------------------------------------

void CRepository::getTags(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->tags(sPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::getTagCommits(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    m_pCommands->tagCommits(sPath, m_pTagModel->branchNames());
}

//-------------------------------------------------------------------------------------------------

void CRepository::getGraph(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    // Loaded rows stay visible while the new graph loads: CGraphModel::setLines()
    // reconciles the content without rebuilding when nothing changed
    m_pCommands->graph(sPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::getBranchLog(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    // Loaded rows stay visible while the new log loads: CLogModel::setLines()
    // reconciles the content without rebuilding when nothing changed
    m_pCommands->branchLog(sPath);
}

//-------------------------------------------------------------------------------------------------

void CRepository::getRefLog(QString sPath)
{
    if (sPath.isEmpty())
        sPath = m_sRepositoryPath;

    // Loaded rows stay visible while the new log loads: CLogModel::setLines()
    // reconciles the content without rebuilding when nothing changed
    m_pCommands->refLog(sPath);
}

//-------------------------------------------------------------------------------------------------

QString CRepository::relativeFileName(const QString& sFullName)
{
    QDir dRoot(m_sRepositoryPath);
    return dRoot.relativeFilePath(sFullName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::onCurrentFileFullName(QString sFileFullName)
{
    if (sFileFullName == m_pFileLogModel->fullSourceName())
        return;

    // Loaded rows stay visible while the new log loads: CLogModel::setLines()
    // reconciles the content without rebuilding when nothing changed
    m_pFileLogModel->setFullSourceName(sFileFullName);
    m_pFileLogModel->setRelativeSourceName(sFileFullName.isEmpty() ? QString("") : relativeFileName(sFileFullName));

    if (sFileFullName.isEmpty())
    {
        m_pFileLogModel->clear();
        m_pFileLogModel->invalidate();

        m_pFileDiffModel->setFullSourceName("");
        m_pFileDiffModel->setRelativeSourceName("");
        m_pFileDiffModel->setLines(QList<CDiffLine*>());
        m_pStagedFileDiffModel->setFullSourceName("");
        m_pStagedFileDiffModel->setRelativeSourceName("");
        m_pStagedFileDiffModel->setLines(QList<CDiffLine*>());
        m_pUnstagedFileDiffModel->setFullSourceName("");
        m_pUnstagedFileDiffModel->setRelativeSourceName("");
        m_pUnstagedFileDiffModel->setLines(QList<CDiffLine*>());
        setStagedFileDiffString("");
        setUnstagedFileDiffString("");

        return;
    }

    m_pFileDiffModel->setFullSourceName(sFileFullName);
    m_pFileDiffModel->setRelativeSourceName(relativeFileName(sFileFullName));
    m_pStagedFileDiffModel->setFullSourceName(sFileFullName);
    m_pStagedFileDiffModel->setRelativeSourceName(relativeFileName(sFileFullName));
    m_pUnstagedFileDiffModel->setFullSourceName(sFileFullName);
    m_pUnstagedFileDiffModel->setRelativeSourceName(relativeFileName(sFileFullName));

    requestWorkingTreeDiffs(sFileFullName);
    m_pCommands->fileLog(m_sRepositoryPath, sFileFullName);
}

//-------------------------------------------------------------------------------------------------

void CRepository::onDiffCommitIdChanged()
{
    QString sDiffFromCommitId = m_sDiffFromCommitId;
    QString sDiffToCommitId = m_sDiffToCommitId;

    if (not m_sDiffFromCommitId.isEmpty() && not m_sDiffToCommitId.isEmpty())
    {
        m_pCommands->twoCommitDiff(m_sRepositoryPath, m_sDiffFromCommitId, m_sDiffToCommitId);

        setDiffFromCommitId("");
        setDiffToCommitId("");
    }

    m_pBranchLogModel->commitChanged(sDiffFromCommitId);
    m_pBranchLogModel->commitChanged(sDiffToCommitId);
    m_pGraphModel->commitChanged(sDiffFromCommitId);
    m_pGraphModel->commitChanged(sDiffToCommitId);
}

//-------------------------------------------------------------------------------------------------

void CRepository::onShouldRefreshFileStatus()
{
    checkRepositoryStatus();
    checkChangedFileStatus();
}

//-------------------------------------------------------------------------------------------------

void CRepository::onRequestBranchLogData(int iStartIndex, int iCount)
{
    m_pCommands->branchLog(m_sRepositoryPath, iStartIndex, iCount);
}

//-------------------------------------------------------------------------------------------------

void CRepository::onRequestFileLogData(int iStartIndex, int iCount)
{
    if (not m_pFileLogModel->fullSourceName().isEmpty())
    {
        m_pCommands->fileLog(m_sRepositoryPath, m_pFileLogModel->fullSourceName(), iStartIndex, iCount);
    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::onRequestGraphData(int iStartIndex, int iCount)
{
    m_pCommands->graph(m_sRepositoryPath, iStartIndex, iCount);
}
