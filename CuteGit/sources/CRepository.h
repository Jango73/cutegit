
#pragma once

// Qt
#include <QDateTime>
#include <QHash>
#include <QSet>
#include <QStringListModel>
#include <QVariant>

// qt-plus
#include "Macros.h"
#include "CXMLNode.h"

// Application
#include "CEnums.h"
#include "CLabel.h"
#include "commands/CCommands.h"
#include "CBranch.h"
#include "CRepoFile.h"
#include "CLogLineCollection.h"
#include "CDiffLine.h"
#include "CGraphLine.h"
#include "CFlatFileModel.h"
#include "CFlatFileModelProxy.h"
#include "CStagedFileModelProxy.h"
#include "CBranchModel.h"
#include "CLogModel.h"
#include "CLogModelProxy.h"
#include "CDiffModel.h"
#include "CDiffModelProxy.h"
#include "CGraphModel.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CController;

//-------------------------------------------------------------------------------------------------

class CRepository : public QObject
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    // Informations on the repo
    Q_FAST_PROPERTY_NO_SET_IMPL(CEnums::ERepositoryType, e, repositoryType, RepositoryType)
    Q_FAST_PROPERTY(CEnums::ERepositoryStatus, e, repositoryStatus, RepositoryStatus)
    Q_FAST_PROPERTY(QString, s, repositoryPath, RepositoryPath)
    Q_FAST_PROPERTY(QString, s, repositoryName, RepositoryName)

    // From and to commit Ids
    Q_FAST_PROPERTY(QString, s, diffFromCommitId, DiffFromCommitId)
    Q_FAST_PROPERTY(QString, s, diffToCommitId, DiffToCommitId)

    // Main controller (plain member: no Q_PROPERTY to avoid an inclusion cycle
    // with CController, which exposes the repository to QML)
    CController* controller() const { return m_pController; }

    // Versioning system interface
	Q_FAST_PROPERTY(CCommands*, p, commands, Commands)

    // Models for files
    Q_FAST_PROPERTY(CFlatFileModel*, p, flatFileModel, FlatFileModel)
    Q_FAST_PROPERTY(CFlatFileModelProxy*, p, flatFileModelProxy, FlatFileModelProxy)
    Q_FAST_PROPERTY(CStagedFileModelProxy*, p, stagedFileModelProxy, StagedFileModelProxy)

    // Models for branchs and tags
    Q_FAST_PROPERTY(CBranchModel*, p, remoteBranchModel, RemoteBranchModel)
    Q_FAST_PROPERTY(CBranchModel*, p, localBranchModel, LocalBranchModel)
    Q_FAST_PROPERTY(CBranchModel*, p, tagModel, TagModel)

    // Models for various logs
    Q_FAST_PROPERTY(CLogModel*, p, branchLogModel, BranchLogModel)
    Q_FAST_PROPERTY(CLogModelProxy*, p, branchLogModelProxy, BranchLogModelProxy)
    Q_FAST_PROPERTY(CLogModel*, p, fileLogModel, FileLogModel)
    Q_FAST_PROPERTY(CGraphModel*, p, graphModel, GraphModel)
    Q_FAST_PROPERTY(CLogModel*, p, refLogModel, RefLogModel)

    // Model for the diff view
    Q_FAST_PROPERTY(CDiffModel*, p, fileDiffModel, FileDiffModel)
    Q_FAST_PROPERTY(CDiffModelProxy*, p, fileDiffModelProxy, FileDiffModelProxy)
    Q_FAST_PROPERTY(QString, s, fileDiffString, FileDiffString)

    // Details of the commit shown in the commit diff view
    Q_FAST_PROPERTY(QString, s, commitDetailsId, CommitDetailsId)
    Q_FAST_PROPERTY(QString, s, commitDetailsAuthor, CommitDetailsAuthor)
    Q_FAST_PROPERTY(QDateTime, d, commitDetailsDate, CommitDetailsDate)
    Q_FAST_PROPERTY(QString, s, commitDetailsMessage, CommitDetailsMessage)
    Q_FAST_PROPERTY(int, i, commitDetailsAddedCount, CommitDetailsAddedCount)
    Q_FAST_PROPERTY(int, i, commitDetailsRemovedCount, CommitDetailsRemovedCount)

    // Models for staged versus unstaged working tree diffs
    Q_FAST_PROPERTY(CDiffModel*, p, stagedFileDiffModel, StagedFileDiffModel)
    Q_FAST_PROPERTY(CDiffModelProxy*, p, stagedFileDiffModelProxy, StagedFileDiffModelProxy)
    Q_FAST_PROPERTY(QString, s, stagedFileDiffString, StagedFileDiffString)
    Q_FAST_PROPERTY(CDiffModel*, p, unstagedFileDiffModel, UnstagedFileDiffModel)
    Q_FAST_PROPERTY(CDiffModelProxy*, p, unstagedFileDiffModelProxy, UnstagedFileDiffModelProxy)
    Q_FAST_PROPERTY(QString, s, unstagedFileDiffString, UnstagedFileDiffString)

    // Model for the blame view
    Q_FAST_PROPERTY(CDiffModel*, p, fileBlameModel, FileBlameModel)
    Q_FAST_PROPERTY(CDiffModelProxy*, p, fileBlameModelProxy, FileBlameModelProxy)

    // Output of commands
    Q_FAST_PROPERTY(QStringListModel*, p, commandOutputModel, CommandOutputModel)
    Q_FAST_PROPERTY(int, i, maxCommandOutputLines, MaxCommandOutputLines)

    // Name of the current branch
    Q_FAST_PROPERTY_NO_SET_IMPL(QString, s, currentBranch, CurrentBranch)

    // List of files in the repo
    Q_FAST_PROPERTY(CRepoFileList, l, repoFiles, RepoFiles)

    // Number of commits ahead and behind
    Q_FAST_PROPERTY(int, i, commitCountAhead, CommitCountAhead)
    Q_FAST_PROPERTY(int, i, commitCountBehind, CommitCountBehind)

    // Values that drive visibility of actions
    Q_FAST_PROPERTY(bool, b, hasModifiedFiles, HasModifiedFiles)
    Q_FAST_PROPERTY(bool, b, hasCommitableFiles, HasCommitableFiles)
    Q_FAST_PROPERTY(bool, b, hasPushableCommits, HasPushableCommits)
    Q_FAST_PROPERTY(bool, b, hasPullableCommits, HasPullableCommits)
    Q_FAST_PROPERTY(bool, b, hasStashableFiles, HasStashableFiles)

    // External change detection (driven by the controller polling)
    Q_FAST_PROPERTY(QString, s, pollFingerprint, PollFingerprint)
    Q_FAST_PROPERTY(bool, b, needsRefresh, NeedsRefresh)

    Q_PROPERTY(QString repositoryTypeString READ repositoryTypeString NOTIFY repositoryTypeStringChanged)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CRepository(const QString& sPath, CController* pController, QObject *parent = nullptr, CEnums::ERepositoryType eType = CEnums::UnknownRepositoryType);

    //! Destructor
    virtual ~CRepository() override;

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //!
    CRepoFile* fileByFullName(const QString& sFullName) const;

    //! True once the file inventory and the first full scan have been issued
    bool isLoaded() const;

    //!
    QList<CLabel*> labelsForCommit(const QString& sCommitId) const;

    //!
    QString repositoryTypeString() const;

    //-------------------------------------------------------------------------------------------------
    // Invokable control methods
    //-------------------------------------------------------------------------------------------------

    //! Clears the list of output lines
    Q_INVOKABLE void clearOutput();

	//! Copies a string to clipboard
    Q_INVOKABLE void copyText(const QString& sText);

    //! Opens a file for edit
    Q_INVOKABLE void openFiles(QStringList lFullNames);

    //!
    Q_INVOKABLE bool can(CEnums::ECapability eWhat);

    //! Checks the status of the repo at sPath
    Q_INVOKABLE void checkRepositoryStatus(QString sPath = "");

    //! Creates a list of CRepoFile from the repo at sPath
    Q_INVOKABLE void checkAllFileStatus(QString sPath = "");

    //! Creates a list of CRepoFile from the repo at sPath
    Q_INVOKABLE void checkChangedFileStatus(QString sPath = "");

    //! Fetches the ignored files on demand, the default refresh skips them
    Q_INVOKABLE void checkIgnoredFileStatus();

    //! Drops the ignored rows, the proxy hides them but the models own them
    Q_INVOKABLE void purgeIgnoredFiles();

    //! Shows ignored files, fetching them first when switched on
    void setShowIgnored(bool bValue);

    //! Creates a list of CRepoFile from the repo at sPath
    Q_INVOKABLE void checkFileStatus(const QString& sFileFullName);

    //!
    Q_INVOKABLE void refresh();

    //! Loads the branch log, unless it is already fresh (called by shown views)
    Q_INVOKABLE void refreshBranchLog();

    //! Loads the graph, unless it is already fresh (called by shown views)
    Q_INVOKABLE void refreshGraph();

    //! Loads the ref log, unless it is already fresh (called by shown views)
    Q_INVOKABLE void refreshRefLog();

    //! Enqueues a poll fingerprint command, unless a command is already in flight
    void pollExternalChanges();

    //! Toggles the 'staged' flag of files
    Q_INVOKABLE void toggleStaged(QStringList lFullNames);

    //! Deletes files
    Q_INVOKABLE void deleteFiles(QStringList lFullNames);

    //!
    Q_INVOKABLE void stageSelection(QStringList lFullNames);

    //!
    Q_INVOKABLE void unstageSelection(QStringList lFullNames);

    //! Stages all modified files
    Q_INVOKABLE void stageAll();

    //! Unstages all staged files
    Q_INVOKABLE void unstageAll();

    //! Does a revert on all selected files
    Q_INVOKABLE void revertSelection(QStringList lFullNames);

    //! Commits (or amends) all staged files
    Q_INVOKABLE void commit(const QString& sMessage, bool bAmend);

    //! Continues any ongoing rebase
    Q_INVOKABLE void continueRebase();

    //! Aborts any ongoing rebase
    Q_INVOKABLE void abortRebase();

    //! Pushes local commits to remote
    Q_INVOKABLE void push();

    //! Pushes local commits to remote, rewriting remote history when it diverged
    Q_INVOKABLE void forcePush();

    //! Pushes local commits to remote, as work in progress
    Q_INVOKABLE void pushAsWIP();

    //! Pulls remote commits
    Q_INVOKABLE void pull();

    //! Fetches changes from remote
    Q_INVOKABLE void fetch();

    //! Saves stash
    Q_INVOKABLE void stashSave();

    //! Pops stash
    Q_INVOKABLE void stashPop();

    //! Saves the working tree diff as a patch
    Q_INVOKABLE void saveWorkingTreeDiffAsPatch(const QString& sFullName);

    //! Saves the commit diff as a patch
    Q_INVOKABLE void saveCommitDiffAsPatch(const QString& sFullName);

    //! Applies a patch
    Q_INVOKABLE void patchApply(const QString& sFullName);

    //! Resets current branch to a commit
    Q_INVOKABLE void commitReset(const QString& sCommitId);

    //! Rebases current branch on a commit
    Q_INVOKABLE void commitRebase(const QString& sCommitId);

    //! Squashes a commit
    Q_INVOKABLE void commitSquash(const QString& sCommitId);

    //! Branches from a commit
    Q_INVOKABLE void commitBranchFrom(const QString& sCommitId, const QString& sBranchName);

    //! Changes the message of a commit
    Q_INVOKABLE void changeCommitMessage(const QString& sCommitId, const QString& sMessage);

    //! Rebases on a branch
    Q_INVOKABLE void rebaseOnBranch(const QString& sName);

    //! Merges a branch
    Q_INVOKABLE void mergeBranch(const QString& sName);

    //! Deletes a branch
    Q_INVOKABLE void deleteBranch(const QString& sName);

    //! Creates a tag
    Q_INVOKABLE void createTagOnCommit(const QString& sCommitId, const QString& sTagName, const QString& sMessage);

    //! View diff with previous commit
    Q_INVOKABLE void commitDiffPrevious(const QString& sCommitId);

    //! Refreshes the commit details from the loaded logs and commit diff
    Q_INVOKABLE void refreshCommitDetails();

    //! Shows info about last modifications in a file
    Q_INVOKABLE void blame(QString sFileFullName);

    //! Cleans up local stuff
    Q_INVOKABLE void cleanUp();

    //! Sets the file filter
    Q_INVOKABLE void setFileFilter(const QString& sText);

    //! Sets the file sort field
    Q_INVOKABLE void setFileSortField(CEnums::ESortField eField);

    //! Sets the file sort direction
    Q_INVOKABLE void setFileSortDirection(bool bDirection);

    //! Sets the working tree diff filter
    Q_INVOKABLE void setWorkingTreeDiffFilter(const QString& sText);

    //! Sets the commit diff filter
    Q_INVOKABLE void setCommitDiffFilter(const QString& sText);

    //! Stages selected unstaged diff lines, given as unstaged proxy rows
    Q_INVOKABLE void stageDiffLines(QVariantList lProxyRows);

    //! Unstages selected staged diff lines, given as staged proxy rows
    Q_INVOKABLE void unstageDiffLines(QVariantList lProxyRows);

    //! Stages every line of a hunk in the unstaged diff
    Q_INVOKABLE void stageDiffHunk(int iHunkId);

    //! Unstages every line of a hunk in the staged diff
    Q_INVOKABLE void unstageDiffHunk(int iHunkId);

    //! Sets the blame filter
    Q_INVOKABLE void setFileBlameFilter(const QString& sText);

    //! Sets the branch log filter
    Q_INVOKABLE void setBranchLogFilter(const QString& sText);

    //-------------------------------------------------------------------------------------------------
    // Static control methods
    //-------------------------------------------------------------------------------------------------

    //! Returns the type of repository from a folder
    static CEnums::ERepositoryType getRepositoryTypeFromFolder(const QString& sPath);

    //! Returns the type of repository from an URL
    static CEnums::ERepositoryType getRepositoryTypeFromURL(const QString& sRepositoryURL);

    //! Returns a command interface for a given repository type
    static CCommands* getCommandsForRepositoryType(CController* pController, CEnums::ERepositoryType eType);

    //-------------------------------------------------------------------------------------------------
    // Protected members
    //-------------------------------------------------------------------------------------------------

protected:

    //! Main controller
    CController* m_pController;

    //! True once the file inventory and the first full scan have been issued
    bool m_bFileInventoryDone;

    //! True when ignored files are shown, their batch runs on every refresh
    bool m_bShowIgnored;

    //! True while a poll fingerprint command is waiting for its result
    bool m_bPollPending;

    //! Log domains that missed a refresh, reloaded on demand by shown views
    QSet<QString> m_sStaleLogDomains;

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

    //!
    void getBranches(QString sPath = "");

    //!
    void getBranchHeadCommits(QString sPath = "");

    //!
    void getBranchesCommitCountAheadBehind(QString sPath = "");

    //!
    void getTags(QString sPath = "");

    //!
    void getTagCommits(QString sPath = "");

    //!
    void getGraph(QString sPath = "");

    //!
    void getBranchLog(QString sPath = "");

    //!
    void getRefLog(QString sPath = "");

    //! Loads one log domain when stale or never loaded, clears its stale mark
    void refreshLogDomain(const QString& sDomain);

    //! Rows already held by a log domain model, detects a never loaded domain
    int logDomainRowCount(const QString& sDomain) const;

    //!
    QString relativeFileName(const QString& sFullName);

    //! Requests staged and unstaged diffs for a file
    void requestWorkingTreeDiffs(const QString& sFileFullName);

    //! Returns the loaded log line showing a commit, nullptr when not loaded
    const CLogLine* findCommitLine(const QString& sCommitId) const;

    //! Refreshes the commit details from the loaded logs and commit diff
    void updateCommitDetails();

    //! Returns true when a working tree diff result still belongs to the displayed file
    bool workingTreeDiffMatchesCurrentFile(CDiffModel* pModel, const QString& sUserData) const;

    //! Builds a patch from selected diff rows and applies it on the index
    void applyDiffLines(CDiffModel* pModel, CDiffModelProxy* pProxy, const QVariantList& lProxyRows, bool bStage);

    //! Builds a patch from every visible change of a hunk and applies it on the index
    void applyDiffHunk(CDiffModel* pModel, CDiffModelProxy* pProxy, int iHunkId, bool bStage);

    //! Builds a patch from selected source rows and applies it on the index
    void applyDiffSourceRows(CDiffModel* pModel, const QSet<int>& sSelectedSourceRows, bool bStage);

    //! Returns the staged or unstaged diff model for a working tree diff command
    CDiffModel* workingTreeDiffModel(CEnums::EProcessCommand eCommand) const;

    //! Forwards a text to the status bar, through the statusTextRequested signal
    void setStatusText(const QString& sText);

    //! Shows the finished or failed status text of a commit, amend, push, pull or fetch operation
    //! On failure the status bar shows the process error summary when one is found, the generic failed text otherwise
    void reportOperationStatus(CEnums::EProcessCommand eCommand, int iExitCode, const QString& sOutput);

    //! Stores the raw output of a staged or unstaged diff
    void setWorkingTreeDiffString(CEnums::EProcessCommand eCommand, const QString& sOutput, const QString& sUserData);

    //! Stores parsed lines of a staged or unstaged diff
    void setWorkingTreeDiffLines(CEnums::EProcessCommand eCommand, QList<CDiffLine*> lNewLines, const QString& sUserData);

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    //! Emitted when a text should be shown in the status bar
    void statusTextRequested(const QString& sText);

    //! Emitted when log source data changed, shown views reload on receipt
    void logDataStaled();

    void repositoryTypeStringChanged();

    //-------------------------------------------------------------------------------------------------
    // Slots
    //-------------------------------------------------------------------------------------------------

protected slots:

    //! Called when a file has been highlighted
    void onCurrentFileFullName(QString sFileFullName);

    //!
    void onNewOutput(QString sOutput, bool bSeparation);

    //! Call when an issued command throws a string
    void onNewOutputString(CEnums::EProcessCommand eCommand, QString sValue, QString sUserData = QString(), int iExitCode = 0);

    //! Call when an issued command throws a key/value
    void onNewOutputKeyValue(CEnums::EProcessCommand eCommand, QString sKey, QString sValue);

    //! Call when an issued command throws a string list
    void onNewOutputStringList(CEnums::EProcessCommand eCommand, QStringList lValue);

    //! Call when an issued command throws a list of CBranch objects
    void onNewOutputListOfCBranch(CEnums::EProcessCommand eCommand, QList<CBranch*> lNewBranches);

    //! Call when an issued command throws a list of CRepoFile objects
    void onNewOutputListOfCRepoFile(CEnums::EProcessCommand eCommand, CRepoFileList lNewRepoFiles);

    //! Call when an issued command throws a list of CLogLineCollection objects
    void onNewOutputCLogLineCollection(CEnums::EProcessCommand eCommand, CLogLineCollection lNewLines);

    //! Call when an issued command throws a list of CDiffLine objects
    void onNewOutputListOfCDiffLine(CEnums::EProcessCommand eCommand, QList<CDiffLine*> lNewLines, QString sUserData = QString());

    //! Call when an issued command throws a list of CGraphLine objects
    void onNewOutputCGraphLineCollection(CEnums::EProcessCommand eCommand, CGraphLineCollection lNewLines);

    //!
    void onDiffCommitIdChanged();

    //!
    void onShouldRefreshFileStatus();

    //! Indirectly called by models when they need more data for branch log (lazy loading)
    void onRequestBranchLogData(int iStartIndex, int iCount);

    //! Indirectly called by models when they need more data for file log (lazy loading)
    void onRequestFileLogData(int iStartIndex, int iCount);

    //! Indirectly called by models when they need more data for graph (lazy loading)
    void onRequestGraphData(int iStartIndex, int iCount);
};
