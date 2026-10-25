
#pragma once

// Qt
#include <QObject>

// Application
#include "CCommands.h"

//-------------------------------------------------------------------------------------------------

class CSvnCommands : public CCommands
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Constructor
    CSvnCommands(CController* pController);

    //! Destructor
    virtual ~CSvnCommands() override;

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    virtual bool can(CEnums::ECapability eWhat) const override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    virtual void cloneRepository(const QString& sRepositoryURL, const QString& sRepositoryPath) override;

    //!
    virtual void repositoryStatus(const QString& sPath) override;

    //!
    virtual void allFileStatus(const QString& sPath) override;

    //!
    virtual void changedFileStatus(const QString& sPath) override;

    //!
    virtual void ignoredFileStatus(const QString& sPath) override;

    //!
    virtual void pollFingerprint(const QString& sPath) override;

    //!
    virtual void branches(const QString& sPath) override;

    //!
    virtual void graph(const QString& sPath, int iFrom = 0, int iCount = GRAPH_COUNT_DEFAULT) override;

    //!
    virtual void fileLog(const QString& sPath, const QString& sFullName, int iFrom = 0, int iCount = LOG_COUNT_DEFAULT) override;

    //!
    virtual void deleteFile(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void toggleStaged(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void stageFile(const QString& sPath, const QString& sFullName, bool bStage) override;

    //!
    virtual void stageAll(const QString& sPath, bool bStage) override;

    //!
    virtual void revertFile(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void undeleteFile(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void commit(const QString& sPath, const QString& sMessage) override;

    //!
    virtual void pull(const QString& sPath) override;

    //!
    virtual void unstagedFileDiff(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void blame(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void commitDiffPrevious(const QString& sPath, const QString& sCommitId) override;

    //!
    virtual void twoCommitDiff(const QString& sPath, const QString& sFromCommitId, const QString& sToCommitId) override;

    //!
    virtual void setCurrentBranch(const QString& sPath, const QString& sBranch) override;

    //!
    virtual void createBranchOnCommit(const QString& sPath, const QString& sCommitId, const QString& sBranchName) override;

    //!
    virtual void createTagOnCommit(const QString& sPath, const QString& sCommitId, const QString& sTagName, const QString& sMessage) override;

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    CRepoFile* repoFileForLine(const QString& sPath, QString sLine);

    //!
    void handleBranchOutput(const CProcessResult& tResult);

    //!
    void handleFileStatusOutput(const CProcessResult& tResult);

    //!
    void handleTrackedFilesOutput(const CProcessResult& tResult);

    //!
    void handleLogOutput(const CProcessResult& tResult);

    //!
    void handleGraphOutput(const CProcessResult& tResult);

    //!
    void handleDiffOutput(const CProcessResult& tResult);

    //!
    void handleBlameOutput(const CProcessResult& tResult);

    //-------------------------------------------------------------------------------------------------
    // Protected slots
    //-------------------------------------------------------------------------------------------------

protected slots:

    //!
    void onExecFinished(const CProcessResult& tResult);

    //-------------------------------------------------------------------------------------------------
    // Protected constants
    //-------------------------------------------------------------------------------------------------

protected:

    // Command strings

    static const QString sCommandAdd;
    static const QString sCommandAddForce;
    static const QString sCommandBlame;
    static const QString sCommandCheckout;
    static const QString sCommandCommit;
    static const QString sCommandCopyBranch;
    static const QString sCommandCopyTag;
    static const QString sCommandDelete;
    static const QString sCommandDiffFile;
    static const QString sCommandDiffRevision;
    static const QString sCommandDiffTwoRevisions;
    static const QString sCommandFileLog;
    static const QString sCommandGraphLog;
    static const QString sCommandInfo;
    static const QString sCommandInfoForFile;
    static const QString sCommandListBranches;
    static const QString sCommandRevert;
    static const QString sCommandStatus;
    static const QString sCommandStatusForFile;
    static const QString sCommandStatusNoIgnore;
    static const QString sCommandTrackedFiles;
    static const QString sCommandSwitchBranch;
    static const QString sCommandSwitchTrunk;
    static const QString sCommandUpdate;

    // Regular expressions

    static const QString sBlameLineRegExp;

    // Other strings

    static const QString sTrunkName;

    // Status characters

    static const QString sStatusAdded;
    static const QString sStatusConflicted;
    static const QString sStatusDeleted;
    static const QString sStatusIgnored;
    static const QString sStatusExternal;
    static const QString sStatusModified;
    static const QString sStatusMissing;
    static const QString sStatusObstructed;
    static const QString sStatusReplaced;
    static const QString sStatusUntracked;
};
