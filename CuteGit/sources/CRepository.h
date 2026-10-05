
#pragma once

// Qt
#include <QStringListModel>
#include <QHash>

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

    //! Creates a list of CRepoFile from the repo at sPath
    Q_INVOKABLE void checkFileStatus(const QString& sFileFullName);

    //!
    Q_INVOKABLE void refresh();

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

    //! Saves the file diff as a patch
    Q_INVOKABLE void saveFileDiffAsPatch(const QString& sFullName);

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

    //! Sets the diff filter
    Q_INVOKABLE void setFileDiffFilter(const QString& sText);

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

    //!
    QString relativeFileName(const QString& sFullName);

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

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
    void onNewOutputString(CEnums::EProcessCommand eCommand, QString sValue);

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
    void onNewOutputListOfCDiffLine(CEnums::EProcessCommand eCommand, QList<CDiffLine*> lNewLines);

    //! Call when an issued command throws a list of CGraphLine objects
    void onNewOutputListOfCGraphLine(CEnums::EProcessCommand eCommand, QList<CGraphLine*> lNewLines);

    //!
    void onDiffCommitIdChanged();

    //!
    void onShouldRefreshFileStatus();

    //! Indirectly called by models when they need more data for branch log (lazy loading)
    void onRequestBranchLogData(int iStartIndex, int iCount);

    //! Indirectly called by models when they need more data for file log (lazy loading)
    void onRequestFileLogData(int iStartIndex, int iCount);
};
