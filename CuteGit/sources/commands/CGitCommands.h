
#pragma once

// Qt
#include <QObject>

// Application
#include "CCommands.h"

//-------------------------------------------------------------------------------------------------

class CGitCommands : public CCommands
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Enumerators
    //-------------------------------------------------------------------------------------------------

    enum ERebaseType
    {
        eRTReword,
        eRTEdit,
        eRTSquash
    };

    enum ERebaseStep
    {
        eRSChangeCommitEditSequence,
        eRSChangeCommitEditMessage,
        eRSSquashCommitEditSequence,
        eRSSquashCommitEditMessage
    };

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Constructor
    CGitCommands(CController* pController);

    //! Destructor
    virtual ~CGitCommands() override;

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
    virtual void fileStatus(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void pollFingerprint(const QString& sPath) override;

    //!
    virtual void branches(const QString& sPath) override;

    //!
    virtual void branchHeadCommits(const QString& sPath, QStringList lBranches) override;

    //!
    virtual void branchCommitCountAheadBehind(const QString& sPath, QStringList lBranches) override;

    //!
    virtual void tags(const QString& sPath) override;

    //!
    virtual void tagCommits(const QString& sPath, QStringList lTags) override;

    //!
    virtual void graph(const QString& sPath, int iFrom = 0, int iCount = GRAPH_COUNT_DEFAULT) override;

    //!
    virtual void branchLog(const QString& sPath, int iFrom = 0, int iCount = LOG_COUNT_DEFAULT) override;

    //!
    virtual void fileLog(const QString& sPath, const QString& sFullName, int iFrom = 0, int iCount = LOG_COUNT_DEFAULT) override;

    //!
    virtual void refLog(const QString& sPath, int iFrom = 0, int iCount = REF_LOG_COUNT_DEFAULT) override;

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
    virtual void amend(const QString& sPath) override;

    //!
    virtual void push(const QString& sPath) override;

    //!
    virtual void pull(const QString& sPath) override;

    //!
    virtual void fetch(const QString& sPath) override;

    //!
    virtual void stashSave(const QString& sPath) override;

    //!
    virtual void stashPop(const QString& sPath) override;

    //!
    virtual void patchApply(const QString& sPath, const QString& sFullName) override;

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
    virtual void commitReset(const QString& sPath, const QString& sCommitId) override;

    //!
    virtual void commitRebase(const QString& sPath, const QString& sCommitId) override;

    //!
    virtual void commitSquash(const QString& sPath, const QString& sCommitId) override;

    //!
    virtual void changeCommitMessage(const QString& sPath, const QString& sCommitId, const QString& sMessage) override;

    //!
    virtual void continueRebase(const QString& sPath) override;

    //!
    virtual void abortRebase(const QString& sPath) override;

    //!
    virtual void createBranchOnCommit(const QString& sPath, const QString& sCommitId, const QString& sBranchName) override;

    //!
    virtual void rebaseOnBranch(const QString& sPath, const QString& sBranchName) override;

    //!
    virtual void mergeBranch(const QString& sPath, const QString& sBranchName) override;

    //!
    virtual void deleteBranch(const QString& sPath, const QString& sBranchName) override;

    //!
    virtual void createTagOnCommit(const QString& sPath, const QString& sCommitId, const QString& sTagName, const QString& sMessage) override;

    //!
    virtual void cleanUp(const QString& sPath) override;

    //!
    virtual void editSequenceFile(const QString& sFileName) override;

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    CRepoFile* repoFileForLine(const QString& sPath, QString sLine, bool bSimple);

    //!
    void getFullCommitMessage(QString sPath, const QString &sCommitId);

    //!
    void handleCommitCountOutput(const CProcessResult& tResult);

    //!
    void handleDiffOutput(const CProcessResult& tResult);

    //!
    void handleBlameOutput(const CProcessResult& tResult);

    //!
    void handleBranchOutput(const CProcessResult& tResult);

    //!
    void handleTagOutput(const CProcessResult& tResult);

    //!
    void handleFileStatusOutput(const CProcessResult& tResult);

    //!
    void handleLogOutput(const CProcessResult& tResult);

    //!
    void handleRefLogOutput(const CProcessResult& tResult);

    //!
    void handleGraphOutput(const CProcessResult& tResult);

    //-------------------------------------------------------------------------------------------------
    // Protected slots
    //-------------------------------------------------------------------------------------------------

protected slots:

    //!
    void onExecFinished(const CProcessResult& tResult);

    //!
    void onNewOutputListOfCRepoFile(CEnums::EProcessCommand eCommand, CRepoFileList lNewRepoFiles);

    //-------------------------------------------------------------------------------------------------
    // Protected constants
    //-------------------------------------------------------------------------------------------------

protected:

    // Constant numbers

    static const int iLogFormatValueCount;
    static const int iGraphFormatValueCount;
    static const int iRefLogFormatValueCount;

    // Command strings

    static const QString sCommandAbortMerge;
    static const QString sCommandAbortRebase;
    static const QString sCommandAmend;
    static const QString sCommandBlame;
    static const QString sCommandBranchAhead;
    static const QString sCommandBranches;
    static const QString sCommandBranchFromCommit;
    static const QString sCommandBranchLog;
    static const QString sCommandBranchLogCount;
    static const QString sCommandClone;
    static const QString sCommandCommit;
    static const QString sCommandCommitDiffPrevious;
    static const QString sCommandCommitMessage;
    static const QString sCommandContinueMerge;
    static const QString sCommandContinueRebase;
    static const QString sCommandCreateTagOnCommit;
    static const QString sCommandCurrentBranch;
    static const QString sCommandDeleteBranch;
    static const QString sCommandDeleteFile;
    static const QString sCommandFetch;
    static const QString sCommandFetchAndPrune;
    static const QString sCommandFileLog;
    static const QString sCommandFileLogCount;
    static const QString sCommandFileStatus;
    static const QString sCommandGetRebaseApplyPath;
    static const QString sCommandGetRebaseMergePath;
    static const QString sCommandGraph;
    static const QString sCommandGraphCount;
    static const QString sCommandHeadCommit;
    static const QString sCommandListOtherFiles;
    static const QString sCommandMergeBranch;
    static const QString sCommandPatchApply;
    static const QString sCommandPollFingerprint;
    static const QString sCommandPull;
    static const QString sCommandPush;
    static const QString sCommandRebaseOnBranch;
    static const QString sCommandRebaseOnCommit;
    static const QString sCommandRefLog;
    static const QString sCommandResetOnCommit;
    static const QString sCommandRevert;
    static const QString sCommandSetCurrentBranch;
    static const QString sCommandSquashCommit;
    static const QString sCommandStage;
    static const QString sCommandStageAll;
    static const QString sCommandStashPop;
    static const QString sCommandStashSave;
    static const QString sCommandStatus;
    static const QString sCommandTags;
    static const QString sCommandTagCommit;
    static const QString sCommandTwoCommitDiff;
    static const QString sCommandUnstage;
    static const QString sCommandUnstageAll;
    static const QString sCommandUnstagedDiff;

    // Regular expressions

    static const QString sStatusBranchRegExp;
    static const QString sStatusRegExp;
    static const QString sPickCommitRegExp;
    static const QString sDiffLineRegExp;
    static const QString sBlameLineRegExp;

    // Other strings

    static const QString sComment;
    static const QString sLogFormatSplitter;
    static const QString sRemoteBranchPrefix;
    static const QString sRemoteBranchUselessPrefix;
    static const QString sSequenceEditorToken;
    static const QString sTextEditorToken;
    static const QString sRebaseEditCommit;
    static const QString sRebaseRewordCommit;
    static const QString sRebaseSquashCommit;

    static const QString sStatusClean;
    static const QString sStatusAdded;
    static const QString sStatusModified;
    static const QString sStatusRenamed;
    static const QString sStatusDeleted;
    static const QString sStatusUnmerged;
    static const QString sStatusUntracked;
    static const QString sStatusIgnored;

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

protected:

    ERebaseType     m_eRebaseType;
    ERebaseStep     m_eRebaseStep;
    QString         m_sCommitId;
    QString         m_sCommitMessage;
};
