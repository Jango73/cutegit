
// Qt
#include <QFile>
#include <QModelIndex>
#include <QStringList>
#include <algorithm>

// Application
#include "CRepository.h"
#include "CController.h"
#include "Strings.h"

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutput(QString sOutput, bool bSeparation)
{
    Q_UNUSED(bSeparation)

    QStringList lNewList = sOutput.split(NEW_LINE);

    for (QString sLine : lNewList)
    {
        sLine = sLine.trimmed();

        if (not sLine.isEmpty())
        {
            m_pCommandOutputModel->insertRows(m_pCommandOutputModel->rowCount(), 1);
            QModelIndex qIndex = m_pCommandOutputModel->index(m_pCommandOutputModel->rowCount() - 1);
            m_pCommandOutputModel->setData(qIndex, sLine);
        }
    }

    int iCount = m_pCommandOutputModel->rowCount();

    if (iCount > m_iMaxCommandOutputLines)
        m_pCommandOutputModel->removeRows(0, iCount - m_iMaxCommandOutputLines);

    emit commandOutputModelChanged();
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn void CRepository::setStatusText(const QString& sText)
    Forwards a text to the status bar, through the statusTextRequested signal.
*/
void CRepository::setStatusText(const QString& sText)
{
    emit statusTextRequested(sText);
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn QString failureSummary(const QString& sOutput)
    Returns the most meaningful error line of a failed process output, for the status bar.
    Prefers the remote hook message, then the rejected reference line, then the first
    fatal, error or abort line. Returns an empty string when no error line is found.
*/
static QString failureSummary(const QString& sOutput)
{
    QString sRejectedLine;
    QString sErrorLine;

    for (QString sLine : sOutput.split(NEW_LINE))
    {
        sLine = sLine.trimmed();

        if (sLine.isEmpty())
            continue;

        if (sLine.startsWith("remote:"))
        {
            QString sRemoteLine = sLine.mid(QString("remote:").length()).trimmed();

            if (not sRemoteLine.isEmpty())
                return sRemoteLine;
        }
        else if (sLine.startsWith("!"))
        {
            if (sRejectedLine.isEmpty())
                sRejectedLine = sLine.mid(1).trimmed();
        }
        else if (sLine.startsWith("fatal:") || sLine.startsWith("error:") || sLine.startsWith("abort:"))
        {
            if (sErrorLine.isEmpty())
                sErrorLine = sLine;
        }
    }

    if (not sRejectedLine.isEmpty())
        return sRejectedLine;

    return sErrorLine;
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn void CRepository::reportOperationStatus(CEnums::EProcessCommand eCommand, int iExitCode, const QString& sOutput)
    Shows the finished or failed status text of a commit, amend, push, pull or fetch operation.
    A zero exit code means success, anything else means failure.
    On failure the status bar shows the process error summary when one is found, the generic failed text otherwise.
*/
void CRepository::reportOperationStatus(CEnums::EProcessCommand eCommand, int iExitCode, const QString& sOutput)
{
    bool bSucceeded = (iExitCode == 0);
    QString sFinishedText;
    QString sFailedText;

    switch (eCommand)
    {
    case CEnums::eCommit:
        sFinishedText = Strings::s_sCommitFinished;
        sFailedText = Strings::s_sCommitFailed;
        break;

    case CEnums::eAmend:
        sFinishedText = Strings::s_sAmendFinished;
        sFailedText = Strings::s_sAmendFailed;
        break;

    case CEnums::ePush:
    case CEnums::eForcePush:
    case CEnums::ePushAsWIP:
        sFinishedText = Strings::s_sPushFinished;
        sFailedText = Strings::s_sPushFailed;
        break;

    case CEnums::ePull:
        sFinishedText = Strings::s_sPullFinished;
        sFailedText = Strings::s_sPullFailed;
        break;

    case CEnums::eFetch:
        sFinishedText = Strings::s_sFetchFinished;
        sFailedText = Strings::s_sFetchFailed;
        break;

    default:
        break;
    }

    if (sFinishedText.isEmpty())
        return;

    if (bSucceeded)
    {
        setStatusText(sFinishedText);
    }
    else
    {
        QString sSummary = failureSummary(sOutput);
        setStatusText(sSummary.isEmpty() ? sFailedText : sSummary);
    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputString(CEnums::EProcessCommand eCommand, QString sOutput, QString sUserData, int iExitCode)
{
    switch (eCommand)
    {

    case CEnums::eRepositoryStatus:
    {
        if (sOutput == CRepoFile::sRepositoryStatusClean)
            setRepositoryStatus(CEnums::NoMerge);
        else if (sOutput == CRepoFile::sRepositoryStatusMerge)
            setRepositoryStatus(CEnums::Merge);
        else if (sOutput == CRepoFile::sRepositoryStatusRebase)
            setRepositoryStatus(CEnums::Rebase);
        else if (sOutput == CRepoFile::sRepositoryStatusInteractiveRebase)
            setRepositoryStatus(CEnums::InteractiveRebase);
        break;
    }

    case CEnums::eUnstagedFileDiff:
    case CEnums::eStagedFileDiff:
    {
        // Drop stale diffs tagged for a previously selected file
        setWorkingTreeDiffString(eCommand, sOutput, sUserData);
        break;
    }

    case CEnums::eTwoCommitDiff:
    {
        setFileDiffString(sOutput);
        break;
    }

        // Following commands need only output
    case CEnums::eIssuedCommand:
    case CEnums::eCloneRepository:
    case CEnums::eNotification:
    case CEnums::eApplyPatch:
    case CEnums::eDeleteFile:
    case CEnums::eUndeleteFile:
    case CEnums::eStageFile:
    case CEnums::eStageAll:
    case CEnums::eRevertFile:
    {
        onNewOutput(sOutput, false);
        break;
    }

        // Following commands need output and file status check
    case CEnums::eStashSave:
    case CEnums::eStashPop:
    case CEnums::ePatchApply:
    {
        onNewOutput(sOutput, false);

        checkChangedFileStatus();
        break;
    }

        // Following commands need output and tag refresh
    case CEnums::eCreateTagOnCommit:
    {
        onNewOutput(sOutput, false);

        getTags();
        break;
    }

        // Following commands need output, general refresh and operation status
    case CEnums::eCommit:
    case CEnums::eAmend:
    case CEnums::ePush:
    case CEnums::eForcePush:
    case CEnums::ePushAsWIP:
    case CEnums::ePull:
    case CEnums::eFetch:
    {
        onNewOutput(sOutput, false);

        refresh();

        reportOperationStatus(eCommand, iExitCode, sOutput);
        break;
    }

        // Following commands need output and general refresh
    case CEnums::eSetCurrentBranch:
    case CEnums::eResetToCommit:
    case CEnums::eRebaseOnCommit:
    case CEnums::eSquashCommit:
    case CEnums::eCreateBranchOnCommit:
    case CEnums::eRebaseOnBranch:
    case CEnums::eMergeBranch:
    case CEnums::eDeleteBranch:
    case CEnums::eChangeCommitMessage:
    case CEnums::eContinueRebase:
    case CEnums::eAbortRebase:
    case CEnums::eCleanUp:
    {
        onNewOutput(sOutput, false);

        refresh();
        break;
    }

    case CEnums::ePollFingerprint:
    {
        m_bPollPending = false;

        // A poll result that arrives while own commands run is stale, drop it
        if (m_pCommands != nullptr && m_pCommands->isBusy())
            break;

        // The first result sets the baseline silently
        if (m_sPollFingerprint.isEmpty())
        {
            setPollFingerprint(sOutput);
            break;
        }

        if (sOutput != m_sPollFingerprint)
        {
            setPollFingerprint(sOutput);

            if (m_pController != nullptr && m_pController->currentRepository() == this)
                refresh();
            else
                setNeedsRefresh(true);
        }
        break;
    }

    case CEnums::eCurrentBranch:
    {
        m_sCurrentBranch = sOutput;
        emit currentBranchChanged();
        break;
    }

    default:
    {
        break;
    }

    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::refreshCommitLabels()
{
    m_pBranchLogModel->refreshLabels();
    m_pFileLogModel->refreshLabels();
    m_pGraphModel->refreshLabels();
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputKeyValue(CEnums::EProcessCommand eCommand, QString sKey, QString sValue)
{
    switch (eCommand)
    {

    case CEnums::eBranchHeadCommit:
    {
        m_pRemoteBranchModel->setBranchHeadCommit(sKey, sValue);
        m_pLocalBranchModel->setBranchHeadCommit(sKey, sValue);
        refreshCommitLabels();
        break;
    }

    case CEnums::eTagCommit:
    {
        m_pTagModel->setBranchHeadCommit(sKey, sValue);
        refreshCommitLabels();
        break;
    }

    case CEnums::eBranchCommitCountAhead:
    {
        if (sKey == m_sCurrentBranch)
        {
            setCommitCountAhead(sValue.toInt());
            setHasPushableCommits(m_iCommitCountAhead > 0);
        }
        break;
    }

    case CEnums::eBranchCommitCountBehind:
    {
        if (sKey == m_sCurrentBranch)
        {
            setCommitCountBehind(sValue.toInt());
            setHasPullableCommits(m_iCommitCountBehind > 0);
        }
        break;
    }

    default:
    {
        break;
    }

    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputStringList(CEnums::EProcessCommand eCommand, QStringList lValue)
{
    Q_UNUSED(eCommand)
    Q_UNUSED(lValue)

    // This may be reused some day
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputListOfCBranch(CEnums::EProcessCommand eCommand, QList<CBranch*> lNewBranches)
{
    switch (eCommand)
    {

    case CEnums::eBranches:
    {
        m_pRemoteBranchModel->setBranchList(lNewBranches, CEnums::RemoteBranchLabel);
        m_pLocalBranchModel->setBranchList(lNewBranches, CEnums::LocalBranchLabel);
        refreshCommitLabels();
        getBranchHeadCommits();
        getBranchesCommitCountAheadBehind();
        break;
    }

    case CEnums::eTags:
    {
        m_pTagModel->setBranchList(lNewBranches, CEnums::TagLabel);
        refreshCommitLabels();
        getTagCommits();
        break;
    }

    default:
    {
        break;
    }

    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputListOfCRepoFile(CEnums::EProcessCommand eCommand, CRepoFileList lNewFiles)
{
    switch (eCommand)
    {

    case CEnums::eAllFileStatus:
    case CEnums::eChangedFileStatus:
    case CEnums::eIgnoredFileStatus:
    {
        bool bHasModifiedFiles = false;
        bool bHasCommitableFiles = false;

        // The clean inventory only discovers files: it carries no status
        // information and must never downgrade a known file, otherwise the
        // resync below wipes the displayed diff source and line staging
        // silently stops working until the file is selected again
        bool bInventoryOnly = (eCommand == CEnums::eAllFileStatus);

        // The ignored batch replaces its own rows: it owns the eIgnored
        // entries and must never reset, flag or resync the other files
        bool bIgnoredOnly = (eCommand == CEnums::eIgnoredFileStatus);

        // A fresh ignored listing supersedes the previous one: drop the
        // owned rows first so un-ignored files disappear with it
        if (bIgnoredOnly)
        {
            for (int iIgnoredIndex = 0; iIgnoredIndex < m_lRepoFiles.count(); iIgnoredIndex++)
            {
                if (m_lRepoFiles[iIgnoredIndex]->status() == CEnums::eIgnored)
                {
                    QString sIgnoredKey = m_lRepoFiles[iIgnoredIndex]->fullName();
                    delete m_lRepoFiles[iIgnoredIndex];
                    m_lRepoFiles.removeItem(sIgnoredKey);
                    iIgnoredIndex--;
                }
            }
        }

        // Browse every file in lNewFiles and add/update. Consumed entries
        // stay alive until the presence checks below are done: deleting
        // them here would leave dangling pointers inside lNewFiles.
        QList<CRepoFile*> lOrphanFiles;

        for (CRepoFile* pNewFile : lNewFiles)
        {
            QString sNewKey = pNewFile->fullName();

            if (bInventoryOnly && m_lRepoFiles.containsKey(sNewKey))
            {
                lOrphanFiles << pNewFile;
                continue;
            }

            if (pNewFile->status() != CEnums::eIgnored && pNewFile->status() != CEnums::eClean)
            {
                bHasModifiedFiles = true;

                if (pNewFile->staged())
                    bHasCommitableFiles = true;
            }

            // Update any existing item using sNewKey
            if (m_lRepoFiles.containsKey(sNewKey))
            {
                CRepoFile* pExistingFile = m_lRepoFiles.itemByKey(sNewKey);
                pExistingFile->setStatus(pNewFile->status());
                pExistingFile->setStaged(pNewFile->staged());
                pExistingFile->setChanging(false);

                lOrphanFiles << pNewFile;
            }
            else
            {
                // Add new item
                m_lRepoFiles.addItem(sNewKey, pNewFile);
            }
        }

        // Absence from a real status batch means clean; absence from the
        // clean inventory means nothing, the existence check below handles
        // deleted files. Ignored rows belong to the on-demand batch: the
        // changed batch never carries them and must leave them alone
        if (not bInventoryOnly && not bIgnoredOnly)
        {
            // For every file not existing in lNewFiles, mark 'not staged' and 'clean' if required
            for (CRepoFile* pExistingFile : m_lRepoFiles)
            {
                QString sExistingKey = pExistingFile->fullName();

                if (not lNewFiles.containsKey(sExistingKey))
                {
                    pExistingFile->setStaged(false);

                    // Absence from a real status batch means the file carries
                    // no change anymore: it turned clean, whatever its
                    // previous state (tracked, untracked or missing). Without
                    // this, an externally committed untracked file keeps its
                    // stale status and stays visible after the poll refresh.
                    if (pExistingFile->status() != CEnums::eClean && pExistingFile->status() != CEnums::eIgnored)
                    {
                        pExistingFile->setStatus(CEnums::eClean);
                    }
                }
            }
        }

        // Presence checks are done: consumed entries can now be destroyed
        // without leaving dangling pointers behind
        qDeleteAll(lOrphanFiles);

        // For every file in current list, check if it still exists
        for (int iExistingFileIndex = 0; iExistingFileIndex < m_lRepoFiles.count(); iExistingFileIndex++)
        {
            CRepoFile* pExistingFile = m_lRepoFiles[iExistingFileIndex];
            QString sExistingKey = pExistingFile->fullName();

            if (pExistingFile->status() != CEnums::eDeleted && pExistingFile->status() != CEnums::eMissing)
            {
                if (not QFile(pExistingFile->fullName()).exists())
                {
                    delete pExistingFile;
                    m_lRepoFiles.removeItem(sExistingKey);
                    iExistingFileIndex--;
                }
            }
        }

        // Update some flags: only real status batches own them, the clean
        // inventory would clear them on every pass and the ignored batch
        // carries no dirtiness information
        if (not bInventoryOnly && not bIgnoredOnly)
        {
            setHasModifiedFiles(bHasModifiedFiles);
            setHasCommitableFiles(bHasCommitableFiles);
        }

        // Sort the files
        std::sort(m_lRepoFiles.begin(), m_lRepoFiles.end(), [] (CRepoFile* left, CRepoFile* right) {
            return left->fullName() < right->fullName();
        });

        // Update the models
        m_pFlatFileModel->handleRepoFilesChanged();
        m_pFlatFileModelProxy->filterChanged();

        // Check if file log must be reset: only real status batches carry
        // dirtiness information, the clean inventory must never wipe the
        // displayed diff source and the ignored batch replaces its own rows
        if (not bInventoryOnly && not bIgnoredOnly)
        {
            if (m_lRepoFiles.count() == 0)
            {
                onCurrentFileFullName("");
            }
            else
            {
                // Resynchronize the diff with the file list: after a commit the selected
                // file turns clean and proxy rows shift without QML currentIndexChanged,
                // so without this the diff view keeps showing a stale diff.
                // The requested commit diff is independent and stays untouched.
                QString sCurrentDiffSource = m_pUnstagedFileDiffModel->fullSourceName();

                if (not sCurrentDiffSource.isEmpty())
                {
                    CRepoFile* pCurrentFile = fileByFullName(sCurrentDiffSource);

                    if (pCurrentFile == nullptr || pCurrentFile->status() == CEnums::eClean)
                    {
                        onCurrentFileFullName("");
                    }
                    else
                    {
                        requestWorkingTreeDiffs(sCurrentDiffSource);
                    }
                }
            }
        }

        break;
    }

    default:
    {
        break;
    }

    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputCLogLineCollection(CEnums::EProcessCommand eCommand, CLogLineCollection lNewLines)
{
    switch (eCommand)
    {

    case CEnums::eBranchLog:
    {
        m_pBranchLogModel->setLines(lNewLines);
        break;
    }

    case CEnums::eFileLog:
    {
        m_pFileLogModel->setLines(lNewLines);
        break;
    }

    case CEnums::eRefLog:
    {
        m_pRefLogModel->setLines(lNewLines);
        break;
    }

    default:
    {
        break;
    }

    }

    updateCommitDetails();
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputListOfCDiffLine(CEnums::EProcessCommand eCommand, QList<CDiffLine*> lNewLines, QString sUserData)
{
    switch (eCommand)
    {

    case CEnums::eUnstagedFileDiff:
    case CEnums::eStagedFileDiff:
    {
        // Drop stale diffs tagged for a previously selected file
        setWorkingTreeDiffLines(eCommand, lNewLines, sUserData);
        break;
    }

    case CEnums::eTwoCommitDiff:
    {
        m_pFileDiffModel->setLines(lNewLines);
        updateCommitDetails();
        break;
    }

    case CEnums::eBlame:
    {
        m_pFileBlameModel->setLines(lNewLines);
        break;
    }

    default:
    {
        break;
    }

    }
}

//-------------------------------------------------------------------------------------------------

void CRepository::onNewOutputCGraphLineCollection(CEnums::EProcessCommand eCommand, CGraphLineCollection lNewLines)
{
    switch (eCommand)
    {

    case CEnums::eGraph:
    {
        m_pGraphModel->setLines(lNewLines);
        updateCommitDetails();
        break;
    }

    default:
    {
        break;
    }

    }
}
