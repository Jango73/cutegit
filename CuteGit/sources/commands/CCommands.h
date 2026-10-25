
#pragma once

// Qt
#include <QObject>

// Application
#include "../CEnums.h"
#include "CExecution.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CController;

//-------------------------------------------------------------------------------------------------

class CCommands : public CExecution
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Constructor
    CCommands(CController* pController);

    //! Destructor
    virtual ~CCommands() override;

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    virtual bool can(CEnums::ECapability eWhat) const;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Clones a repository
    virtual void cloneRepository(const QString& sRepositoryURL, const QString& sRepositoryPath);

    //! Checks the status of the repo at sPath
    virtual void repositoryStatus(const QString& sPath);

    //! Creates a list of CRepoFile from the repo at sPath
    virtual void allFileStatus(const QString& sPath);

    //! Creates a list of CRepoFile from the repo at sPath
    virtual void changedFileStatus(const QString& sPath);

    //! Creates a list of ignored CRepoFile from the repo at sPath, on demand only
    virtual void ignoredFileStatus(const QString& sPath);

    //! Creates a list of CRepoFile from the repo at sPath
    virtual void fileStatus(const QString& sPath, const QString& sFullName);

    //! Enqueues a cheap sentinel command used to detect changes made behind our back
    //! The result is thrown as ePollFingerprint and must stay silent (no output logging)
    virtual void pollFingerprint(const QString& sPath);

    //! Creates a list of branches of the repo at sPath
    virtual void branches(const QString& sPath);

    //! Creates a list of head commits for branches of the repo at sPath
    virtual void branchHeadCommits(const QString& sPath, QStringList lBranches);

    //! Creates a list of commit ahead counts for branches of the repo at sPath
    virtual void branchCommitCountAheadBehind(const QString& sPath, QStringList lBranches);

    //! Creates a list of tags of the repo at sPath
    virtual void tags(const QString& sPath);

    //! Creates a list of head commits for tags of the repo at sPath
    virtual void tagCommits(const QString& sPath, QStringList lTags);

    //! Creates a list of CGraphLine from the repo at sPath, paginated like the log
    virtual void graph(const QString& sPath, int iFrom = 0, int iCount = GRAPH_COUNT_DEFAULT);

    //! Creates a list of CLogLine from the repo at sPath, for the current branch
    virtual void branchLog(const QString& sPath, int iFrom = 0, int iCount = LOG_COUNT_DEFAULT);

    //! Creates a list of CLogLine from the repo at sPath, for sFullName
    virtual void fileLog(const QString& sPath, const QString& sFullName, int iFrom = 0, int iCount = LOG_COUNT_DEFAULT);

    //! Creates a list of CLogLine from the repo at sPath, for the ref log
    virtual void refLog(const QString& sPath, int iFrom = 0, int iCount = REF_LOG_COUNT_DEFAULT);

    //! Deletes a file from the repo at sPath
    virtual void deleteFile(const QString& sPath, const QString& sFullName);

    //! Toggles the 'staged' state of sFullName in the repo at sPath
    virtual void toggleStaged(const QString& sPath, const QString& sFullName);

    //! Stages or unstages sFullName in the repo at sPath
    virtual void stageFile(const QString& sPath, const QString& sFullName, bool bStage);

    //! Stages or unstages all files in the repo at sPath
    virtual void stageAll(const QString& sPath, bool bStage);

    //! Reverts all modifications of sFullName in the repo at sPath
    //! Use with caution
    virtual void revertFile(const QString& sPath, const QString& sFullName);

    //! Reverts delete state of sFullName in the repo at sPath
    virtual void undeleteFile(const QString& sPath, const QString& sFullName);

    //! Commits all staged files in the repo at sPath, using sMessage
    virtual void commit(const QString& sPath, const QString& sMessage);

    //! Amends all staged files in the repo at sPath
    virtual void amend(const QString& sPath);

    //!
    virtual void push(const QString& sPath);

    //! Pushes local commits to remote, rewriting remote history when it diverged
    virtual void forcePush(const QString& sPath);

    //!
    virtual void pushAsWIP(const QString& sPath);

    //!
    virtual void pull(const QString& sPath);

    //!
    virtual void fetch(const QString& sPath);

    //!
    virtual void stashSave(const QString& sPath);

    //!
    virtual void stashPop(const QString& sPath);

    //!
    virtual void patchApply(const QString& sPath, const QString& sFullName);

    //!
    virtual void unstagedFileDiff(const QString& sPath, const QString& sFullName);

    //! Shows staged changes of sFullName in the repo at sPath (index versus HEAD)
    virtual void stagedFileDiff(const QString& sPath, const QString& sFullName);

    //! Applies sPatchFile in the repo at sPath, optionally on the index only, optionally reversed
    //! Runs synchronously and returns the process output, empty on success
    virtual QString applyPatchNow(const QString& sPath, const QString& sPatchFile, bool bCached, bool bReverse);

    //!
    virtual void blame(const QString& sPath, const QString& sFullName);

    //!
    virtual void commitDiffPrevious(const QString& sPath, const QString& sCommitId);

    //!
    virtual void twoCommitDiff(const QString& sPath, const QString& sFromCommitId, const QString& sToCommitId);

    //!
    virtual void setCurrentBranch(const QString& sPath, const QString& sBranch);

    //!
    virtual void commitReset(const QString& sPath, const QString& sCommitId);

    //!
    virtual void commitRebase(const QString& sPath, const QString& sCommitId);

    //!
    virtual void commitSquash(const QString& sPath, const QString& sCommitId);

    //!
    virtual void changeCommitMessage(const QString& sPath, const QString& sCommitId, const QString& sMessage);

    //!
    virtual void continueRebase(const QString& sPath);

    //!
    virtual void abortRebase(const QString& sPath);

    //!
    virtual void createBranchOnCommit(const QString& sPath, const QString& sCommitId, const QString& sBranchName);

    //!
    virtual void rebaseOnBranch(const QString& sPath, const QString& sBranchName);

    //!
    virtual void mergeBranch(const QString& sPath, const QString& sBranchName);

    //!
    virtual void deleteBranch(const QString& sPath, const QString& sBranchName);

    //!
    virtual void createTagOnCommit(const QString& sPath, const QString& sCommitId, const QString& sTagName, const QString& sMessage);

    //!
    virtual void cleanUp(const QString& sPath);

    //!
    virtual void updateSubmodules(const QString& sPath);

    //! Checks whether submodule checkouts match the recorded commits
    virtual void submoduleStatus(const QString& sPath);

    //!
    virtual void editSequenceFile(const QString& sFileName);

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

protected:

    //! Builds clean CRepoFile entries for VCS-tracked relative paths, single
    //! place for the file inventory seed shared by all backends
    static CRepoFileList cleanFilesForRelativePaths(const QString& sPath, const QStringList& lRelativeNames);

    CController*    m_pController;
};
