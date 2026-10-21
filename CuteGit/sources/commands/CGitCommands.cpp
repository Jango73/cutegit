
// Qt
#include <QDebug>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QDir>

// Application
#include "CGitCommands.h"
#include "../CUtils.h"
#include "../CController.h"
#include "../Strings.h"
#include "../CDiffLine.h"
#include "../CDiffConstants.h"

/*!
    \class CGitCommands
    \inmodule CuteGit
    \section1 General
    This is the repository control interface for Git.

    \code

     _._     _,-'""`-._
    (,-.`._,'(       |\`-/|
        `-.-' \ )-`( , o o)
              `-    \`_`"'-

    \endcode
*/

//-------------------------------------------------------------------------------------------------
// Constants

const int CGitCommands::iLogFieldCount                   = 5;
const int CGitCommands::iGraphFieldCount                 = 8;
const int CGitCommands::iRefLogFormatValueCount         = 3;

const QString CGitCommands::sCommandAbortMerge          = "git merge --abort";
const QString CGitCommands::sCommandAbortRebase         = "git rebase --abort";
const QString CGitCommands::sCommandAmend               = "git commit --amend --reset-author --no-edit";
const QString CGitCommands::sCommandBlame               = "git annotate \"%1\"";
const QString CGitCommands::sCommandBranchAheadBehind = "git for-each-ref --format=\"%(refname:short) %(upstream:track)\" refs/heads";
const QString CGitCommands::sCommandBranches            = "git branch -a";
const QString CGitCommands::sCommandBranchFromCommit    = "git checkout -b \"%1\" \"%2\"";
const QString CGitCommands::sCommandBranchLog           = "git log --pretty=format:\"%x1e%h &&& %s &&& %an &&& %aI &&& %B\" --skip=%1 --max-count=%2";
const QString CGitCommands::sCommandBranchLogCount      = "git rev-list --count HEAD";
const QString CGitCommands::sCommandClone               = "git clone --progress \"%1\"";
const QString CGitCommands::sCommandCommit              = "git commit -m \"%1\"";
const QString CGitCommands::sCommandCommitDiffPrevious  = "git diff %1~1 %1";
const QString CGitCommands::sCommandContinueMerge       = "git merge --continue";
const QString CGitCommands::sCommandContinueRebase      = "git rebase --continue";
const QString CGitCommands::sCommandCreateTagOnCommit   = "git tag -m \"%1\" \"%2\" \"%3\"";
const QString CGitCommands::sCommandCurrentBranch       = "git rev-parse --abbrev-ref HEAD";
const QString CGitCommands::sCommandDeleteBranch        = "git branch --delete \"%1\"";
const QString CGitCommands::sCommandDeleteFile          = "git rm \"%1\"";
const QString CGitCommands::sCommandFetch               = "git fetch";
const QString CGitCommands::sCommandFetchAndPrune       = "git fetch -p";
const QString CGitCommands::sCommandFileLog              = "git log --pretty=format:\"%x1e%h &&& %s &&& %an &&& %aI &&& %B\" --skip=%1 --max-count=%2 \"%3\"";
const QString CGitCommands::sCommandFileLogCount        = "git rev-list --count HEAD \"%1\"";
const QString CGitCommands::sCommandFileStatus          = "git status --porcelain --ignored --untracked-files=all \"%1\"";
const QString CGitCommands::sCommandGetRebaseApplyPath  = "git rev-parse --git-path rebase-apply";
const QString CGitCommands::sCommandGetRebaseMergePath  = "git rev-parse --git-path rebase-merge";
const QString CGitCommands::sCommandGraph               = "git log --topo-order --all --decorate --pretty=format:\"%x1e&&& %H &&& %P &&& %D &&& %s &&& %an &&& %aI &&& %B\" --skip=%1 --max-count=%2";
const QString CGitCommands::sCommandGraphCount         = "git rev-list --count --all";
const QString CGitCommands::sCommandHeadCommits         = "git for-each-ref --format=\"%(refname:short) %(objectname:short)\" refs/heads refs/remotes";
const QString CGitCommands::sCommandIgnoredFiles        = "git ls-files --others --ignored --exclude-standard";
const QString CGitCommands::sCommandMergeBranch         = "git merge \"%1\"";
const QString CGitCommands::sCommandPatchApply          = "git apply \"%1\"";
const QString CGitCommands::sCommandPollFingerprint      = "git status --porcelain=v1 -b --ignored=no --untracked-files=normal";
const QString CGitCommands::sCommandPollRefs              = "git for-each-ref --format=\"%(objectname:short) %(refname:short)\"";
const QString CGitCommands::sCommandPull                = "git pull";
const QString CGitCommands::sCommandPush                = "git push %1 %2";
const QString CGitCommands::sCommandPushForce           = "git push --force-with-lease %1 %2";
const QString CGitCommands::sCommandRebaseOnBranch      = "git rebase \"%1\"";
const QString CGitCommands::sCommandRebaseOnCommit      = "git rebase --interactive %1~1";
const QString CGitCommands::sCommandRefLog              = "git reflog show --pretty=format:\"%h &&& %gD &&& %gs\" --skip=%1 --max-count=%2";
const QString CGitCommands::sCommandResetOnCommit       = "git reset %1";
const QString CGitCommands::sCommandRevert              = "git checkout \"%1\"";
const QString CGitCommands::sCommandSetCurrentBranch    = "git checkout \"%1\"";
const QString CGitCommands::sCommandSquashCommit        = "git rebase --interactive %1~2";
const QString CGitCommands::sCommandStage               = "git add -f \"%1\"";
const QString CGitCommands::sCommandStageAll            = "git add -u";
const QString CGitCommands::sCommandStashPop            = "git stash pop";
const QString CGitCommands::sCommandStashSave           = "git stash save";
const QString CGitCommands::sCommandStatus              = "git status --porcelain --untracked-files=all";
const QString CGitCommands::sCommandTags                = "git tag";
const QString CGitCommands::sCommandTagCommits           = "git for-each-ref --format=\"%(refname:short) %(*objectname:short) %(objectname:short)\" refs/tags";
const QString CGitCommands::sCommandTrackedFiles        = "git ls-files --cached";
const QString CGitCommands::sCommandTwoCommitDiff       = "git diff --no-color --ignore-all-space \"%1\" \"%2\"";
const QString CGitCommands::sCommandUnstage             = "git reset HEAD \"%1\"";
const QString CGitCommands::sCommandUnstageAll          = "git reset .";
// File diffs carry 3 context lines (-U3) so hunks stay applicable after
// partial staging drops unselected lines. Patch application tolerates
// zero-context hunks (--unidiff-zero) for the same reason: kept hunks
// hold only selected changes plus kept context.
const QString CGitCommands::sCommandUnstagedDiff        = "git diff --no-color -U3 -- \"%1\"";
const QString CGitCommands::sCommandUnstagedDiffUntracked = "git diff --no-color -U3 --no-index -- \"/dev/null\" \"%1\"";
const QString CGitCommands::sCommandIsTracked            = "git ls-files --error-unmatch -- \"%1\"";
const QString CGitCommands::sCommandStagedDiff          = "git diff --cached --no-color -U3 -- \"%1\"";
const QString CGitCommands::sCommandApplyPatchCached   = "git apply --cached --unidiff-zero \"%1\"";
const QString CGitCommands::sCommandApplyPatchCachedReverse = "git apply --cached --reverse --unidiff-zero \"%1\"";
const QString CGitCommands::sCommandApplyPatch         = "git apply --unidiff-zero \"%1\"";
const QString CGitCommands::sCommandApplyPatchReverse  = "git apply --reverse --unidiff-zero \"%1\"";

const QString CGitCommands::sStatusBranchRegExp         = "";
const QString CGitCommands::sStatusRegExp               = "([a-zA-Z?!\\s])([a-zA-Z?!\\s])\\s(.*)";
const QString CGitCommands::sPickCommitRegExp           = "(pick)\\s+([a-zA-Z0-9]+)\\s+(.*)";
const QString CGitCommands::sDiffLineRegExp             = "diff\\s+(.*)\\s+a/(.*)\\s+b/(.*)";
const QString CGitCommands::sDiffHunkRegExp             = CDiffConstants::sHunkHeaderPattern;
const QString CGitCommands::sBlameLineRegExp            = "([0-9a-fA-F]+)\\s+\\(\\s+([a-zA-Z0-9_-]+)\\s+([0-9:+-]+\\s+[0-9:+-]+\\s+[0-9:+-]+)\\s+([0-9]+)\\)(.*)";

const QString CGitCommands::sComment                    = "#";
const QString CGitCommands::sLogFormatSplitter          = "&&&";
const QChar CGitCommands::sRecordSeparator(0x1E);
const QString CGitCommands::sRemoteBranchPrefix         = "origin";
const QString CGitCommands::sRemoteBranchUselessPrefix  = "remotes/";
const QString CGitCommands::sSequenceEditorToken        = "GIT_SEQUENCE_EDITOR";
const QString CGitCommands::sTextEditorToken            = "GIT_EDITOR";
const QString CGitCommands::sRebaseEditCommit           = "edit %1 %2";
const QString CGitCommands::sRebaseRewordCommit         = "reword %1 %2";
const QString CGitCommands::sRebaseSquashCommit         = "squash %1 %2";

const QString CGitCommands::sStatusClean                = " ";
const QString CGitCommands::sStatusAdded                = "A";
const QString CGitCommands::sStatusModified             = "M";
const QString CGitCommands::sStatusRenamed              = "R";
const QString CGitCommands::sStatusDeleted              = "D";
const QString CGitCommands::sStatusUnmerged             = "U";
const QString CGitCommands::sStatusUntracked            = "?";
const QString CGitCommands::sStatusIgnored              = "!";

//-------------------------------------------------------------------------------------------------

CGitCommands::CGitCommands(CController* pController)
    : CCommands(pController)
    , m_eRebaseType(eRTReword)
    , m_eRebaseStep(eRSChangeCommitEditSequence)
{
    connect(this, &CCommands::execFinished, this, &CGitCommands::onExecFinished);
}

//-------------------------------------------------------------------------------------------------

CGitCommands::~CGitCommands()
{
}

//-------------------------------------------------------------------------------------------------

bool CGitCommands::can(CEnums::ECapability eWhat) const
{
    switch (eWhat)
    {
    case CEnums::PushAsWIP:
        return false;

    default:
        return true;
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::cloneRepository(const QString& sRepositoryURL, const QString& sRepositoryPath)
{
    QString sCommand = QString(sCommandClone).arg(sRepositoryURL);
    exec(new CProcessCommand(
             CEnums::eCloneRepository,
             sRepositoryPath,
             sCommand,
             false,
             QMap<QString, QString>(),
             "",
             CEnums::eCloneRepositoryFinished)
         );
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::repositoryStatus(const QString& sPath)
{
    QString sRebaseApplyPath = QString("%1/%2").arg(sPath).arg(execNow(sPath, sCommandGetRebaseApplyPath).trimmed());
    QString sRebaseMergePath = QString("%1/%2").arg(sPath).arg(execNow(sPath, sCommandGetRebaseMergePath).trimmed());

    if (QDir(sRebaseMergePath).exists())
    {
        emit newOutputString(
                    CEnums::eRepositoryStatus,
                    CRepoFile::sRepositoryStatusInteractiveRebase
                    );
    }
    else if (QDir(sRebaseApplyPath).exists())
    {
        emit newOutputString(
                    CEnums::eRepositoryStatus,
                    CRepoFile::sRepositoryStatusRebase
                    );
    }
    else
    {
        emit newOutputString(
                    CEnums::eRepositoryStatus,
                    CRepoFile::sRepositoryStatusClean
                    );
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::allFileStatus(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eAllFileStatus, sPath, sCommandTrackedFiles));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::changedFileStatus(const QString& sPath)
{
    // Untracked files arrive one by one through --untracked-files=all, the
    // former ls-files batch only duplicated them. Ignored files stay out:
    // 17k node_modules rows must not cross the models on every refresh,
    // they are fetched on demand through ignoredFileStatus
    exec(new CProcessCommand(CEnums::eChangedFileStatus, sPath, sCommandStatus));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::ignoredFileStatus(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eIgnoredFileStatus, sPath, sCommandIgnoredFiles));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::fileStatus(const QString& sPath, const QString& sFullName)
{
    QString sFileStatusCommand = QString(sCommandFileStatus).arg(sFullName);
    exec(new CProcessCommand(CEnums::eChangedFileStatus, sPath, sFileStatusCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::pollFingerprint(const QString& sPath)
{
    m_sPollStatusPart.clear();
    exec(new CProcessCommand(CEnums::ePollFingerprint, sPath, sCommandPollFingerprint, true, QMap<QString, QString>(), "status"));
    exec(new CProcessCommand(CEnums::ePollFingerprint, sPath, sCommandPollRefs, true, QMap<QString, QString>(), "refs"));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::branches(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eBranches, sPath, sCommandBranches));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::branchHeadCommits(const QString& sPath, QStringList lBranches)
{
    Q_UNUSED(lBranches);

    // One batched read covers every branch, no per-item process is needed
    exec(new CProcessCommand(CEnums::eBranchHeadCommit, sPath, sCommandHeadCommits, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::branchCommitCountAheadBehind(const QString& sPath, QStringList lBranches)
{
    Q_UNUSED(lBranches);

    // One batched read covers every local branch with its upstream tracking state
    exec(new CProcessCommand(CEnums::eBranchCommitCountAhead, sPath, sCommandBranchAheadBehind, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::tags(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eTags, sPath, sCommandTags));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::tagCommits(const QString& sPath, QStringList lTags)
{
    Q_UNUSED(lTags);

    // One batched read covers every tag, peeled to its commit when annotated
    exec(new CProcessCommand(CEnums::eTagCommit, sPath, sCommandTagCommits, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::graph(const QString& sPath, int iFrom, int iCount)
{
    int iPotentialCount = execNow(sPath, sCommandGraphCount).trimmed().toInt();
    QString sCommand = QString(sCommandGraph).arg(iFrom).arg(iCount);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eGraph, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::branchLog(const QString& sPath, int iFrom, int iCount)
{
    int iPotentialCount = execNow(sPath, sCommandBranchLogCount).trimmed().toInt();
    QString sCommand = QString(sCommandBranchLog).arg(iFrom).arg(iCount);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eBranchLog, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::fileLog(const QString& sPath, const QString& sFullName, int iFrom, int iCount)
{
    // Getting the count of log entries is very slow so log is limited to LOG_COUNT_DEFAULT for now
    // QString sCountCommand = QString(sCommandFileLogCount).arg(sFullName);
    // int iPotentialCount = execNow(sPath, sCountCommand).trimmed().toInt();
    int iPotentialCount = 0;
    QString sCommand = QString(sCommandFileLog).arg(iFrom).arg(iCount).arg(sFullName);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eFileLog, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::refLog(const QString& sPath, int iFrom, int iCount)
{
    int iPotentialCount = 0;
    QString sCommand = QString(sCommandRefLog).arg(iFrom).arg(iCount);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eRefLog, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::deleteFile(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandDeleteFile).arg(sFullName);
    exec(new CProcessCommand(CEnums::eDeleteFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::toggleStaged(const QString& sPath, const QString& sFullName)
{
    QString sFileStatusCommand = QString(sCommandFileStatus).arg(sFullName);
    QString sLine = execNow(sPath, sFileStatusCommand);

    CRepoFile* pFile = repoFileForLine(sPath, sLine, false);

    if (pFile != nullptr)
    {
        stageFile(sPath, sFullName, not pFile->staged());
        delete pFile;
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::stageFile(const QString& sPath, const QString& sFullName, bool bStage)
{
    QString sCommand = QString(bStage ? sCommandStage : sCommandUnstage).arg(sFullName);
    exec(new CProcessCommand(CEnums::eStageFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::stageAll(const QString& sPath, bool bStage)
{
    emit newOutputString(CEnums::eNotification, bStage ? tr("Staging all files...") : tr("Unstaging all files..."));
    QString sCommand = QString(bStage ? sCommandStageAll : sCommandUnstageAll);
    exec(new CProcessCommand(CEnums::eStageAll, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::revertFile(const QString& sPath, const QString& sFullName)
{
    emit newOutputString(CEnums::eNotification, tr("Reverting..."));
    QString sCommand = QString(sCommandRevert).arg(sFullName);
    exec(new CProcessCommand(CEnums::eRevertFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::undeleteFile(const QString& sPath, const QString& sFullName)
{
    QString sCommand;
    emit newOutputString(CEnums::eNotification, tr("Undeleting..."));
    sCommand = QString(sCommandUnstage).arg(sFullName);
    exec(new CProcessCommand(CEnums::eNotification, sPath, sCommand, true));
    sCommand = QString(sCommandRevert).arg(sFullName);
    exec(new CProcessCommand(CEnums::eUndeleteFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commit(const QString& sPath, const QString& sMessage)
{
    emit newOutputString(CEnums::eNotification, tr("Commiting..."));
    QString sCommand = QString(sCommandCommit).arg(sMessage);
    exec(new CProcessCommand(CEnums::eCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::amend(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Amending..."));
    QString sCommand = QString(sCommandAmend);
    exec(new CProcessCommand(CEnums::eAmend, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::push(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Pushing..."));
    QString sCurrentBranch = execNow(sPath, sCommandCurrentBranch).trimmed();
    if (not sCurrentBranch.isEmpty())
    {
        QString sCommand = QString(sCommandPush).arg(sRemoteBranchPrefix).arg(sCurrentBranch);
        exec(new CProcessCommand(CEnums::ePush, sPath, sCommand, true));
    }
    else
    {
        emit newOutputString(CEnums::eNotification, Strings::s_sUnableToGetBranch);
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::forcePush(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Force pushing..."));
    QString sCurrentBranch = execNow(sPath, sCommandCurrentBranch).trimmed();
    if (not sCurrentBranch.isEmpty())
    {
        QString sCommand = QString(sCommandPushForce).arg(sRemoteBranchPrefix).arg(sCurrentBranch);
        exec(new CProcessCommand(CEnums::eForcePush, sPath, sCommand, true));
    }
    else
    {
        emit newOutputString(CEnums::eNotification, Strings::s_sUnableToGetBranch);
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::pull(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Pulling..."));
    QString sCommand = QString(sCommandPull);
    exec(new CProcessCommand(CEnums::ePull, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::fetch(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Fetching..."));
    QString sCommand = QString(sCommandFetch);
    exec(new CProcessCommand(CEnums::eFetch, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::stashSave(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Saving stash..."));
    exec(new CProcessCommand(CEnums::eStashSave, sPath, sCommandStashSave, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::stashPop(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Restoring stash..."));
    exec(new CProcessCommand(CEnums::eStashPop, sPath, sCommandStashPop, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::patchApply(const QString& sPath, const QString& sFullName)
{
    emit newOutputString(CEnums::eNotification, tr("Applying patch..."));
    QString sCommand = QString(sCommandPatchApply).arg(sFullName);
    exec(new CProcessCommand(CEnums::ePatchApply, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::unstagedFileDiff(const QString& sPath, const QString& sFullName)
{
    // Untracked files are invisible to plain git diff: compare the working
    // tree file against /dev/null so its content shows up as added lines.
    // A relative path keeps the diff headers (a/<relative> b/<relative>)
    // consistent with tracked files, so patch building and staging keep
    // working on the parsed lines
    QString sCommand;
    int iExitCode = 0;

    execNow(sPath, QString(sCommandIsTracked).arg(sFullName), QMap<QString, QString>(), &iExitCode);

    if (iExitCode == 0)
    {
        sCommand = QString(sCommandUnstagedDiff).arg(sFullName);
    }
    else
    {
        QString sRelativeName = QDir(sPath).relativeFilePath(sFullName);
        sCommand = QString(sCommandUnstagedDiffUntracked).arg(sRelativeName);
    }

    exec(new CProcessCommand(CEnums::eUnstagedFileDiff, sPath, sCommand, false, QMap<QString, QString>(), sFullName));
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CGitCommands::stagedFileDiff
    Shows staged changes (index versus HEAD) for a single file.
*/
void CGitCommands::stagedFileDiff(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandStagedDiff).arg(sFullName);
    exec(new CProcessCommand(CEnums::eStagedFileDiff, sPath, sCommand, false, QMap<QString, QString>(), sFullName));
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CGitCommands::applyPatchNow
    Applies a patch file synchronously, optionally on the index only, optionally reversed.
    Returns the process output, empty on success.
*/
QString CGitCommands::applyPatchNow(const QString& sPath, const QString& sPatchFile, bool bCached, bool bReverse)
{
    QString sCommand;

    if (bCached)
        sCommand = QString(bReverse ? sCommandApplyPatchCachedReverse : sCommandApplyPatchCached).arg(sPatchFile);
    else
        sCommand = QString(bReverse ? sCommandApplyPatchReverse : sCommandApplyPatch).arg(sPatchFile);

    return execNow(sPath, sCommand);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::blame(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandBlame).arg(sFullName);
    exec(new CProcessCommand(CEnums::eBlame, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commitDiffPrevious(const QString& sPath, const QString& sCommitId)
{
    QString sCommand = QString(sCommandCommitDiffPrevious).arg(sCommitId);
    exec(new CProcessCommand(CEnums::eTwoCommitDiff, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::twoCommitDiff(const QString& sPath, const QString& sFromCommitId, const QString& sToCommitId)
{
    QString sCommand = QString(sCommandTwoCommitDiff).arg(sFromCommitId).arg(sToCommitId);
    exec(new CProcessCommand(CEnums::eTwoCommitDiff, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::setCurrentBranch(const QString& sPath, const QString& sBranch)
{
    QString sFinalName = sBranch;
    sFinalName.replace(QString("%1/").arg(sRemoteBranchPrefix), "");
    emit newOutputString(CEnums::eNotification, QString(tr("Switching to %1...")).arg(sFinalName));
    QString sCommand = QString(sCommandSetCurrentBranch).arg(sFinalName);
    exec(new CProcessCommand(CEnums::eSetCurrentBranch, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commitReset(const QString& sPath, const QString& sCommitId)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Doing reset on %1...")).arg(sCommitId));

    QString sCommand = QString(sCommandResetOnCommit).arg(sCommitId);
    exec(new CProcessCommand(CEnums::eResetToCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commitRebase(const QString& sPath, const QString& sCommitId)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Doing rebase on %1...")).arg(sCommitId));

    m_eRebaseType = eRTEdit;
    m_eRebaseStep = eRSChangeCommitEditSequence;
    m_sCommitId = sCommitId;
    m_sCommitMessage.clear();

    QMap<QString, QString> mEnvironment;
    mEnvironment[sSequenceEditorToken] = m_pController->stubFileName();
    mEnvironment[sTextEditorToken] = m_pController->stubFileName();

    QString sCommand = QString(sCommandRebaseOnCommit).arg(sCommitId);

    exec(new CProcessCommand(CEnums::eRebaseOnCommit, sPath, sCommand, true, mEnvironment));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commitSquash(const QString& sPath, const QString& sCommitId)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Squashing %1...")).arg(sCommitId));

    m_eRebaseType = eRTSquash;
    m_eRebaseStep = eRSSquashCommitEditSequence;
    m_sCommitId = sCommitId;
    m_sCommitMessage.clear();

    QMap<QString, QString> mEnvironment;
    mEnvironment[sSequenceEditorToken] = m_pController->stubFileName();
    mEnvironment[sTextEditorToken] = m_pController->stubFileName();

    QString sCommand = QString(sCommandSquashCommit).arg(sCommitId);

    exec(new CProcessCommand(CEnums::eSquashCommit, sPath, sCommand, true, mEnvironment));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::changeCommitMessage(const QString& sPath, const QString& sCommitId, const QString& sMessage)
{
    emit newOutputString(CEnums::eNotification, tr("Changing commit message..."));

    m_eRebaseType = eRTReword;
    m_eRebaseStep = eRSChangeCommitEditSequence;
    m_sCommitId = sCommitId;
    m_sCommitMessage = sMessage;

    QMap<QString, QString> mEnvironment;
    mEnvironment[sSequenceEditorToken] = m_pController->stubFileName();
    mEnvironment[sTextEditorToken] = m_pController->stubFileName();

    QString sCommand = QString(sCommandRebaseOnCommit).arg(sCommitId);

    exec(new CProcessCommand(CEnums::eChangeCommitMessage, sPath, sCommand, true, mEnvironment));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::continueRebase(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Continuing rebase..."));
    QString sCommand = QString(sCommandContinueRebase);
    exec(new CProcessCommand(CEnums::eContinueRebase, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::abortRebase(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Aborting rebase..."));
    QString sCommand = QString(sCommandAbortRebase);
    exec(new CProcessCommand(CEnums::eAbortRebase, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::createBranchOnCommit(const QString& sPath, const QString& sCommitId, const QString& sBranchName)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Creating branch %1...")).arg(sBranchName));
    QString sCommand = QString(sCommandBranchFromCommit).arg(sBranchName).arg(sCommitId);
    exec(new CProcessCommand(CEnums::eCreateBranchOnCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::rebaseOnBranch(const QString& sPath, const QString& sBranchName)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Rebasing on branch %1...")).arg(sBranchName));
    QString sCommand = QString(sCommandRebaseOnBranch).arg(sBranchName);
    exec(new CProcessCommand(CEnums::eRebaseOnBranch, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::mergeBranch(const QString& sPath, const QString& sBranchName)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Merging branch %1...")).arg(sBranchName));
    QString sCommand = QString(sCommandMergeBranch).arg(sBranchName);
    exec(new CProcessCommand(CEnums::eMergeBranch, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::deleteBranch(const QString& sPath, const QString& sBranchName)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Deleting branch %1...")).arg(sBranchName));
    QString sCommand = QString(sCommandDeleteBranch).arg(sBranchName);
    exec(new CProcessCommand(CEnums::eDeleteBranch, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::createTagOnCommit(const QString& sPath, const QString& sCommitId, const QString& sTagName, const QString& sMessage)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Creating tag %1...")).arg(sTagName));
    QString sCommand = QString(sCommandCreateTagOnCommit).arg(sMessage).arg(sTagName).arg(sCommitId);
    exec(new CProcessCommand(CEnums::eCreateTagOnCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::cleanUp(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Cleaning up..."));
    QString sCommand = QString(sCommandFetchAndPrune);
    exec(new CProcessCommand(CEnums::eCleanUp, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::editSequenceFile(const QString& sFileName)
{
    // Read in the file provided by GIT
    QString sInputText = CUtils::getTextFileContents(sFileName);

    if (not sInputText.isEmpty())
    {
        QStringList lOutputLines;

        // Get a list of lines
        QStringList lLines = sInputText.split(NEW_LINE);

        switch (m_eRebaseStep)
        {
        // In case we're modifying the sequence file
        // Most often .git/rebase-merge/git-rebase-todo
        case eRSChangeCommitEditSequence:
        case eRSSquashCommitEditSequence:
        {
            // Setup a "pick nnnn aaaa" reg exp
            QRegularExpression tRegExp(sPickCommitRegExp);

            for (QString sLine : lLines)
            {
                // Process only non-comment lines
                if (not sLine.startsWith(sComment))
                {
                    QRegularExpressionMatch match = tRegExp.match(sLine);
                    if (match.hasMatch())
                    {
                        QString sWord = match.captured(1).trimmed();
                        QString sCommitID = match.captured(2).trimmed();
                        QString sMessage = match.captured(3).trimmed();

                        if (sCommitID == m_sCommitId)
                        {
                            switch (m_eRebaseType)
                            {

                            case eRTEdit:
                            {
                                lOutputLines << QString(sRebaseEditCommit).arg(sCommitID).arg(sMessage);
                                break;
                            }

                            case eRTReword:
                            {
                                lOutputLines << QString(sRebaseRewordCommit).arg(sCommitID).arg(sMessage);
                                m_eRebaseStep = eRSChangeCommitEditMessage;
                                break;
                            }

                            case eRTSquash:
                            {
                                lOutputLines << QString(sRebaseSquashCommit).arg(sCommitID).arg(sMessage);
                                m_eRebaseStep = eRSSquashCommitEditMessage;
                                break;
                            }

                            }
                        }
                        else
                        {
                            lOutputLines << sLine;
                        }
                    }
                }
            }

            break;
        }

        case eRSChangeCommitEditMessage:
        {
            lOutputLines = m_sCommitMessage.split(NEW_LINE);
            break;
        }

        case eRSSquashCommitEditMessage:
        {
            lOutputLines = lLines;
            break;
        }
        }

        // Rebuild output text from lines
        QString sOutputText;

        for (QString sLine : lOutputLines)
        {
            sOutputText += sLine;
            sOutputText += NEW_LINE;
        }

        QFile file(sFileName);

        // Rewrite sequence file
        if (file.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&file);
            stream << sOutputText;
            file.close();
        }
    }
}

//-------------------------------------------------------------------------------------------------

CRepoFile* CGitCommands::repoFileForLine(const QString &sPath, QString sLine, bool bSimple)
{
    if (not bSimple)
    {
        QRegularExpression tRegExp(sStatusRegExp);

    #ifdef HAVE_QSTRING_BACK
        if (not sLine.isEmpty() && sLine.back() == '/')
            sLine.chop(1);
    #endif

        // TODO : Handle quoted file names

        QRegularExpressionMatch match = tRegExp.match(sLine);
        if (match.hasMatch())
        {
            QString sStaged = match.captured(1).trimmed();
            QString sUnstaged = match.captured(2).trimmed();
            QString sRelativeName = match.captured(3).split("->").last().trimmed();
            QString sFullName = sPath + PATH_SEP + sRelativeName;
            QString sFileName = QFileInfo(sFullName).fileName();
            bool bStaged = false;

            CEnums::ERepoFileStatus eStatus = CEnums::eClean;

            if (not sStaged.isEmpty())
            {
                bStaged = true;

                if (sStaged == sStatusAdded)
                    eStatus = CEnums::eAdded;
                else if (sStaged == sStatusModified)
                    eStatus = CEnums::eModified;
                else if (sStaged == sStatusRenamed)
                    eStatus = CEnums::eRenamed;
                else if (sStaged == sStatusDeleted)
                    eStatus = CEnums::eDeleted;
                else if (sStaged == sStatusIgnored)
                {
                    bStaged = false;
                    eStatus = CEnums::eIgnored;
                }
                else if (sStaged == sStatusUntracked)
                {
                    bStaged = false;
                    eStatus = CEnums::eUntracked;
                }
            }

            if (not sUnstaged.isEmpty())
            {
                if (sUnstaged == sStatusAdded)
                {
                    bStaged = false;
                    eStatus = CEnums::eAdded;
                }
                else if (sUnstaged == sStatusModified)
                {
                    bStaged = false;
                    eStatus = CEnums::eModified;
                }
                else if (sUnstaged == sStatusRenamed)
                {
                    bStaged = false;
                    eStatus = CEnums::eRenamed;
                }
                else if (sUnstaged == sStatusDeleted)
                {
                    bStaged = false;
                    eStatus = CEnums::eDeleted;
                }
                else if (sUnstaged == sStatusIgnored)
                {
                    eStatus = CEnums::eIgnored;
                }
                else if (sUnstaged == sStatusUntracked)
                {
                    bStaged = false;
                    eStatus = CEnums::eUntracked;
                }
            }

            CRepoFile* pFile = new CRepoFile();
            pFile->setFullName(sFullName);
            pFile->setFileName(sFileName);
            pFile->setRelativeName(sRelativeName);
            pFile->setStatus(eStatus);
            pFile->setStaged(bStaged);

            return pFile;
        }
    }
    else
    {
        // Bare relative path from the ignored listing: no status columns
        // to parse, the entry is ignored by construction
        QString sRelativeName = sLine.trimmed();

        if (sRelativeName.isEmpty())
            return nullptr;

        QString sFullName = sPath + PATH_SEP + sRelativeName;
        QString sFileName = QFileInfo(sFullName).fileName();

        CRepoFile* pFile = new CRepoFile();
        pFile->setFullName(sFullName);
        pFile->setFileName(sFileName);
        pFile->setRelativeName(sRelativeName);
        pFile->setStatus(CEnums::eIgnored);

        return pFile;
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleBranchHeadCommitOutput(const CProcessResult& tResult)
{
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lLines)
    {
        sLine = sLine.trimmed();

        if (sLine.isEmpty())
            continue;

        // The commit id is the last token, branch names may contain spaces
        int iSeparator = sLine.lastIndexOf(' ');

        if (iSeparator <= 0)
            continue;

        emit newOutputKeyValue(tResult.m_eCommand, sLine.left(iSeparator), sLine.mid(iSeparator + 1));
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleBranchAheadBehindOutput(const CProcessResult& tResult)
{
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lLines)
    {
        sLine = sLine.trimmed();

        if (sLine.isEmpty())
            continue;

        // Tracking states read "[ahead 1]", "[behind 2]", "[ahead 1, behind 2]",
        // "[gone]" or empty when in sync or without upstream
        QString sBranch = sLine;
        QString sTrack;

        if (sLine.endsWith(']'))
        {
            int iTrack = sLine.lastIndexOf(" [");

            if (iTrack > 0)
            {
                sBranch = sLine.left(iTrack);
                sTrack = sLine.mid(iTrack + 1);
            }
        }

        int iAhead = 0;
        int iBehind = 0;

        if (sTrack.startsWith('[') && sTrack.endsWith(']'))
        {
            QStringList lParts = sTrack.mid(1, sTrack.length() - 2).split(',');

            for (QString sPart : lParts)
            {
                sPart = sPart.trimmed();

                if (sPart.startsWith("ahead "))
                    iAhead = sPart.mid(6).toInt();
                else if (sPart.startsWith("behind "))
                    iBehind = sPart.mid(7).toInt();
            }
        }

        emit newOutputKeyValue(CEnums::eBranchCommitCountAhead, sBranch, QString::number(iAhead));
        emit newOutputKeyValue(CEnums::eBranchCommitCountBehind, sBranch, QString::number(iBehind));
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleTagCommitOutput(const CProcessResult& tResult)
{
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lLines)
    {
        sLine = sLine.trimmed();

        if (sLine.isEmpty())
            continue;

        // Lines read "name peeled object", the peeled id stays empty for
        // lightweight tags and names may contain spaces
        QStringList lParts = sLine.split(' ', Qt::KeepEmptyParts);

        if (lParts.count() < 3)
            continue;

        QString sObject = lParts.takeLast();
        QString sPeeled = lParts.takeLast();
        QString sTag = lParts.join(' ');

        if (sTag.isEmpty() || sObject.isEmpty())
            continue;

        emit newOutputKeyValue(tResult.m_eCommand, sTag, sPeeled.isEmpty() ? sObject : sPeeled);
    }
}

//-------------------------------------------------------------------------------------------------

/***************************************************************************/

/*! Builds a fully configured diff line, so callers set every field in one call */
static CDiffLine* makeDiffLine(CEnums::EDiffOperation eOperation, const QString& sText, const QString& sFileName, int iHunkId, int iOldLineNumber, int iNewLineNumber)
{
    CDiffLine* pReturnValue = new CDiffLine();

    pReturnValue->setOperation(eOperation);
    pReturnValue->setText(sText);
    pReturnValue->setFileName(sFileName);
    pReturnValue->setHunkId(iHunkId);
    pReturnValue->setOldLineNumber(iOldLineNumber);
    pReturnValue->setNewLineNumber(iNewLineNumber);

    return pReturnValue;
}

/***************************************************************************/

void CGitCommands::handleDiffOutput(const CProcessResult& tResult)
{
    // Create CDiffLine objects with the returned string of the process
    // Hunk headers are kept with operation DiffHunk so callers can rebuild
    // applicable patches for partial staging

    QList<CDiffLine*> lReturnValue;
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);
    bool bAtLeastOneLineOK = false;

    const int iInvalidHunkId = CDiffConstants::iInvalidHunkId;
    const int iNoLineNumber = CDiffConstants::iNoLineNumber;
    QString sCurrentFileName;
    int iHunkId = iInvalidHunkId;
    int iOldLineNumber = iNoLineNumber;
    int iNewLineNumber = iNoLineNumber;

    QRegularExpression tFileRegExp(sDiffLineRegExp);
    QRegularExpression tHunkRegExp(sDiffHunkRegExp);

    for (QString sLine : lLines)
    {
        // Keep context lines holding an empty file line (" "): only the
        // trailing split artifact (truly empty) is skipped
        if (not sLine.isEmpty())
        {
            bAtLeastOneLineOK = true;

            if (sLine.startsWith(CDiffConstants::sDiffIndexPrefix) || sLine.startsWith(CDiffConstants::sDiffAddedHeaderPrefix) || sLine.startsWith(CDiffConstants::sDiffDeletedHeaderPrefix)
                    || sLine.startsWith(CDiffConstants::sDiffNewFilePrefix) || sLine.startsWith(CDiffConstants::sDiffDeletedFilePrefix)
                    || sLine.startsWith(CDiffConstants::sDiffOldModePrefix) || sLine.startsWith(CDiffConstants::sDiffNewModePrefix)
                    || sLine.startsWith(CDiffConstants::sDiffSimilarityPrefix) || sLine.startsWith(CDiffConstants::sDiffDissimilarityPrefix)
                    || sLine.startsWith(CDiffConstants::sDiffRenameFromPrefix) || sLine.startsWith(CDiffConstants::sDiffRenameToPrefix)
                    || sLine.startsWith(CDiffConstants::sDiffCopyFromPrefix) || sLine.startsWith(CDiffConstants::sDiffCopyToPrefix))
            {
                continue;
            }

            CDiffLine* pDiffLine = nullptr;

            if (sLine.startsWith(CDiffConstants::sDiffFilePrefix))
            {
                QString sNewText = sLine;

                QRegularExpressionMatch match = tFileRegExp.match(sLine);
                if (match.hasMatch())
                {
                    sNewText = match.captured(CDiffConstants::iDiffFilePathCapture).trimmed();
                }

                sCurrentFileName = sNewText;

                // Keep hunk identifiers unique across files: patch grouping
                // by hunk alone stays correct on multi-file diffs
                pDiffLine = makeDiffLine(CEnums::DiffFileName, sNewText, sCurrentFileName, iInvalidHunkId, iNoLineNumber, iNoLineNumber);
            }
            else if (sLine.startsWith(CDiffConstants::sDiffHunkPrefix))
            {
                QRegularExpressionMatch match = tHunkRegExp.match(sLine);
                if (match.hasMatch())
                {
                    iOldLineNumber = match.captured(CDiffConstants::iHunkOldStartCapture).toInt();
                    iNewLineNumber = match.captured(CDiffConstants::iHunkNewStartCapture).toInt();
                }

                iHunkId++;

                pDiffLine = makeDiffLine(CEnums::DiffHunk, sLine, sCurrentFileName, iHunkId, iNoLineNumber, iNoLineNumber);
            }
            else if (sLine.startsWith(CDiffConstants::sDiffAddedPrefix) && not sLine.startsWith(CDiffConstants::sDiffAddedHeaderPrefix))
            {
                pDiffLine = makeDiffLine(CEnums::DiffAdd, sLine, sCurrentFileName, iHunkId, iNoLineNumber, iNewLineNumber);
                iNewLineNumber++;
            }
            else if (sLine.startsWith(CDiffConstants::sDiffDeletedPrefix) && not sLine.startsWith(CDiffConstants::sDiffDeletedHeaderPrefix))
            {
                pDiffLine = makeDiffLine(CEnums::DiffDelete, sLine, sCurrentFileName, iHunkId, iOldLineNumber, iNoLineNumber);
                iOldLineNumber++;
            }
            else if (sLine.startsWith(CDiffConstants::sDiffNoNewlinePrefix))
            {
                pDiffLine = makeDiffLine(CEnums::None, sLine, sCurrentFileName, iHunkId, iNoLineNumber, iNoLineNumber);
            }
            else
            {
                pDiffLine = makeDiffLine(CEnums::None, sLine, sCurrentFileName, iHunkId, iOldLineNumber, iNewLineNumber);
                iOldLineNumber++;
                iNewLineNumber++;
            }

            lReturnValue << pDiffLine;
        }
    }

    if (not bAtLeastOneLineOK)
    {
        qDeleteAll(lReturnValue);
        lReturnValue.clear();
    }

    emit newOutputString(tResult.m_eCommand, tResult.m_sValue, tResult.m_sUserData);
    emit newOutputListOfCDiffLine(tResult.m_eCommand, lReturnValue, tResult.m_sUserData);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleBlameOutput(const CProcessResult& tResult)
{
    QList<CDiffLine*> lReturnValue;
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lLines)
    {
        CDiffLine* pDiffLine = new CDiffLine();

        QRegularExpression tRegExp(sBlameLineRegExp);

        QRegularExpressionMatch match = tRegExp.match(sLine);
        if (match.hasMatch())
        {
            QString sCommitID = match.captured(1).trimmed();
            QString sAuthor = match.captured(2).trimmed();
            QString sDate = match.captured(3).trimmed();
            QString sLine = match.captured(4).trimmed();
            QString sText = match.captured(5);

            QString sFinalText = QString("%1: %2 %3 | %4")
                               .arg(sLine.toInt(), 5, 10, QChar('0'))
                               .arg(sAuthor, -20)
                               .arg(sDate)
                               .arg(sText);

            pDiffLine->setText(sFinalText);

            lReturnValue << pDiffLine;
        }
    }

    emit newOutputListOfCDiffLine(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleBranchOutput(const CProcessResult& tResult)
{
    // Create CBranchs with the returned string of the process

    QList<CBranch*> lReturnValue;
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lLines)
    {
        if (sLine.contains("->"))
            continue;

        if (not sLine.isEmpty())
        {
            CBranch* pNewBranch = new CBranch();
            bool bIsCurrent = false;

            if (sLine.startsWith("*"))
                bIsCurrent = true;

            pNewBranch->setType(sLine.contains(sRemoteBranchUselessPrefix) ? CEnums::RemoteBranchLabel : CEnums::LocalBranchLabel);

            sLine.remove(0, 2);
            sLine.replace(sRemoteBranchUselessPrefix, "");
            pNewBranch->setName(sLine);

            if (bIsCurrent)
            {
                // Tell the world about the current branch
                emit newOutputString(CEnums::eCurrentBranch, sLine);
            }

            lReturnValue << pNewBranch;
        }
    }

    emit newOutputListOfCBranch(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleTagOutput(const CProcessResult& tResult)
{
    QList<CBranch*> lReturnValue;
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lLines)
    {
        sLine = sLine.trimmed();

        if (not sLine.isEmpty())
        {
            CBranch* pNewBranch = new CBranch();
            pNewBranch->setType(CEnums::TagLabel);
            pNewBranch->setName(sLine);
            lReturnValue << pNewBranch;
        }
    }

    emit newOutputListOfCBranch(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleFileStatusOutput(const CProcessResult& tResult)
{
    // Create CRepoFiles with the returned string of the process

    CRepoFileList lReturnValue;
    QStringList lStrings = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lStrings)
    {
        CRepoFile* pFile = repoFileForLine(tResult.m_sPath, sLine, tResult.m_eCommand == CEnums::eIgnoredFileStatus);

        if (pFile != nullptr)
            lReturnValue.addItem(pFile->fullName(), pFile);
    }

    emit newOutputListOfCRepoFile(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleTrackedFilesOutput(const CProcessResult& tResult)
{
    // The tracked listing carries bare relative paths: every entry seeds
    // the inventory as clean, dirtiness arrives through the status batches
    CRepoFileList lReturnValue = cleanFilesForRelativePaths(tResult.m_sPath, tResult.m_sValue.split(NEW_LINE));

    emit newOutputListOfCRepoFile(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn static QStringList splitRecordFields(const QString& sRecord, const QString& sSplitter, int iFieldCount)
    Splits a record into fixed fields, the last field keeps the remainder
    so bodies containing the splitter stay intact. Returns an empty list
    when the record holds fewer fields.
*/
static QStringList splitRecordFields(const QString& sRecord, const QString& sSplitter, int iFieldCount)
{
    QStringList sFields;
    QString sRemainder = sRecord;

    for (int iField = 0; iField < iFieldCount - 1; iField++)
    {
        int iSeparator = sRemainder.indexOf(sSplitter);

        if (iSeparator < 0)
            return QStringList();

        sFields << sRemainder.left(iSeparator);
        sRemainder = sRemainder.mid(iSeparator + sSplitter.length());
    }

    sFields << sRemainder;

    return sFields;
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleLogOutput(const CProcessResult& tResult)
{
    // Create a CLogLineCollection with the returned string of the process
    // Records start with the record separator: the raw body (%B) is last and
    // spans lines, so records split on the separator instead of newlines

    CLogLineCollection lReturnValue;
    QStringList lStrings = tResult.m_sValue.split(sRecordSeparator);

    for (QString sLine : lStrings)
    {
        if (sLine.trimmed().isEmpty())
            continue;

        QStringList sValues = splitRecordFields(sLine, sLogFormatSplitter, iLogFieldCount);

        if (sValues.isEmpty())
            continue;

        {
            CLogLine* pLine = new CLogLine();

            pLine->setCommitId(sValues[0].trimmed());
            pLine->setMessage(sValues[4]);
            pLine->setMessageIsComplete(true);
            pLine->setAuthor(sValues[2].trimmed());
            pLine->setDate(QDateTime::fromString(sValues[3].trimmed(), Qt::ISODate));

            lReturnValue.add(pLine);
        }
    }

    QStringList lUserValues = tResult.m_sUserData.split(",");
    int iPotentialCount = lUserValues[0].toInt();
    int iStartIndex = lUserValues[1].toInt();

    if (iPotentialCount == 0)
        iPotentialCount = lReturnValue.lines().count();

    lReturnValue.setPotentialCount(iPotentialCount);
    lReturnValue.setStartIndex(iStartIndex);

    emit newOutputCLogLineCollection(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::handleRefLogOutput(const CProcessResult& tResult)
{
    // Create a CLogLineCollection with the returned string of the process

    CLogLineCollection lReturnValue;
    QStringList lStrings = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lStrings)
    {
        QStringList sValues = sLine.split(sLogFormatSplitter);

        if (sValues.count() == iRefLogFormatValueCount)
        {
            CLogLine* pLine = new CLogLine();

            pLine->setCommitId(sValues[0].trimmed());
            pLine->setAuthor(sValues[1].trimmed());
            pLine->setMessage(sValues[2].trimmed());
            pLine->setMessageIsComplete(true);

            lReturnValue.add(pLine);
        }
    }

    QStringList lUserValues = tResult.m_sUserData.split(",");
    int iPotentialCount = lUserValues[0].toInt();
    int iStartIndex = lUserValues[1].toInt();

    if (iPotentialCount == 0)
        iPotentialCount = lReturnValue.lines().count();

    lReturnValue.setPotentialCount(iPotentialCount);
    lReturnValue.setStartIndex(iStartIndex);

    emit newOutputCLogLineCollection(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

/*! Splits parent ids of a graph field, trimming tokens and skipping empties */
static QStringList splitParentIds(const QString& sValue)
{
    QStringList lReturnValue;

    for (QString sToken : sValue.split(" ", Qt::SkipEmptyParts))
    {
        sToken = sToken.trimmed();

        if (!sToken.isEmpty())
            lReturnValue << sToken;
    }

    return lReturnValue;
}

/***************************************************************************/

/*! Splits decorations of a graph field, trimming tokens and skipping empties */
static QStringList splitDecorations(const QString& sValue)
{
    QStringList lReturnValue;

    for (QString sToken : sValue.split(",", Qt::SkipEmptyParts))
    {
        sToken = sToken.trimmed();

        if (!sToken.isEmpty())
            lReturnValue << sToken;
    }

    return lReturnValue;
}

/***************************************************************************/

void CGitCommands::handleGraphOutput(const CProcessResult& tResult)
{
    // Create a CGraphLineCollection with the returned string of the process

    CGraphLineCollection lReturnValue;
    QStringList lStrings = tResult.m_sValue.split(sRecordSeparator);

    for (QString sLine : lStrings)
    {
        if (sLine.trimmed().isEmpty())
            continue;

        QStringList sValues = splitRecordFields(sLine, sLogFormatSplitter, iGraphFieldCount);

        if (sValues.isEmpty())
            continue;

        {
            QString sCommitId = sValues[1].trimmed();

            if (sCommitId.isEmpty())
                continue;

            CGraphLine* pLine = new CGraphLine();

            pLine->setCommitId(sCommitId);
            pLine->setParents(splitParentIds(sValues[2]));
            pLine->setDecorations(splitDecorations(sValues[3]));
            pLine->setMessage(sValues[7]);
            pLine->setMessageIsComplete(true);
            pLine->setAuthor(sValues[5].trimmed());
            pLine->setDate(QDateTime::fromString(sValues[6].trimmed(), Qt::ISODate));

            lReturnValue.add(pLine);
        }
    }

    QStringList lUserValues = tResult.m_sUserData.split(",");
    int iPotentialCount = lUserValues.count() > 0 ? lUserValues[0].toInt() : 0;
    int iStartIndex = lUserValues.count() > 1 ? lUserValues[1].toInt() : 0;

    if (iPotentialCount == 0)
        iPotentialCount = lReturnValue.lines().count();

    lReturnValue.setPotentialCount(iPotentialCount);
    lReturnValue.setStartIndex(iStartIndex);

    emit newOutputCGraphLineCollection(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::onExecFinished(const CProcessResult& tResult)
{
    switch (tResult.m_eCommand)
    {

    case CEnums::eNothing:
    case CEnums::eNotification:
    case CEnums::eCurrentBranch:
    case CEnums::eRepositoryStatus:
        break;

    case CEnums::eIssuedCommand:
    case CEnums::eCloneRepository:
    case CEnums::eCloneRepositoryFinished:
    case CEnums::eDeleteFile:
    case CEnums::eUndeleteFile:
    case CEnums::eStageFile:
    case CEnums::eStageAll:
    case CEnums::eRevertFile:
    case CEnums::eCommit:
    case CEnums::eAmend:
    case CEnums::ePush:
    case CEnums::eForcePush:
    case CEnums::ePushAsWIP:
    case CEnums::ePull:
    case CEnums::eFetch:
    case CEnums::eStashSave:
    case CEnums::eStashPop:
    case CEnums::eSetCurrentBranch:
    case CEnums::eResetToCommit:
    case CEnums::eRebaseOnCommit:
    case CEnums::eSquashCommit:
    case CEnums::eCreateBranchOnCommit:
    case CEnums::eRebaseOnBranch:
    case CEnums::eMergeBranch:
    case CEnums::eDeleteBranch:
    case CEnums::eCreateTagOnCommit:
    case CEnums::eChangeCommitMessage:
    case CEnums::eContinueRebase:
    case CEnums::eAbortRebase:
    case CEnums::ePatchCreate:
    case CEnums::ePatchApply:
    case CEnums::eApplyPatch:
    case CEnums::eCleanUp:
    {
        // Throw the returned string of the process
        emit newOutputString(tResult.m_eCommand, tResult.m_sValue, QString(), tResult.m_iExitCode);
        break;
    }

    case CEnums::eBranchHeadCommit:
    {
        handleBranchHeadCommitOutput(tResult);
        break;
    }

    case CEnums::ePollFingerprint:
    {
        if (tResult.m_sUserData == "status")
        {
            m_sPollStatusPart = tResult.m_sValue;
        }
        else if (tResult.m_sUserData == "refs")
        {
            // Throw the combined sentinel output, the repository compares it with the previous one
            emit newOutputString(tResult.m_eCommand, m_sPollStatusPart + tResult.m_sValue);
            m_sPollStatusPart.clear();
        }
        else
        {
            // Throw the raw sentinel output, the repository compares it with the previous one
            emit newOutputString(tResult.m_eCommand, tResult.m_sValue);
        }
        break;
    }

    case CEnums::eTagCommit:
    {
        handleTagCommitOutput(tResult);
        break;
    }

    case CEnums::eBranchCommitCountAhead:
    {
        handleBranchAheadBehindOutput(tResult);
        break;
    }

    case CEnums::eUnstagedFileDiff:
    case CEnums::eStagedFileDiff:
    case CEnums::eTwoCommitDiff:
    {
        handleDiffOutput(tResult);
        break;
    }

    case CEnums::eBlame:
    {
        handleBlameOutput(tResult);
        break;
    }

    case CEnums::eBranches:
    {
        handleBranchOutput(tResult);
        break;
    }

    case CEnums::eTags:
    {
        handleTagOutput(tResult);
        break;
    }

    case CEnums::eAllFileStatus:
    {
        handleTrackedFilesOutput(tResult);
        break;
    }

    case CEnums::eChangedFileStatus:
    case CEnums::eIgnoredFileStatus:
    {
        handleFileStatusOutput(tResult);
        break;
    }

    case CEnums::eFileLog:
    case CEnums::eBranchLog:
    {
        handleLogOutput(tResult);
        break;
    }

    case CEnums::eRefLog:
    {
        handleRefLogOutput(tResult);
        break;
    }

    case CEnums::eGraph:
    {
        handleGraphOutput(tResult);
        break;
    }

    }
}
