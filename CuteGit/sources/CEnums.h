
#pragma once

// Qt
#include <QObject>
#include <QtGlobal>

//-------------------------------------------------------------------------------------------------

// Character constants
#define PATH_SEP            "/"
#define NEW_LINE            "\n"
#define NEW_LINE_ESCAPED    "\\n"

// Qt features
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#else
#define HAVE_QSTRING_BACK
#endif

#define LOG_COUNT_DEFAULT       20
#define REF_LOG_COUNT_DEFAULT   50
#define GRAPH_COUNT_DEFAULT     50

//-------------------------------------------------------------------------------------------------

class CEnums : public QObject
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Enumerators exposed to QML
    //-------------------------------------------------------------------------------------------------

    // Capabilities of a versioning system
    enum ECapability
    {
        Clone,
        ShowGlobalGraph,
        ShowBranchLog,
        ShowFileLog,
        Stash,
        Commit,
        Amend,
        Push,
        ForcePush,
        PushAsWIP,
        Pull,
        Fetch,
        SquashCommit,
        ChangeCommitMessage,
        ResetToCommit,
        RebaseOnCommit,
        BranchFromCommit,
        TagOnCommit,
        MergeBranch,
        DeleteBranch,
        ContinueRebase,
        AbortRebase,
        CreatePatch,
        ApplyPatch,
        Blame,
        CleanUp,
        UpdateSubmodules,
        Stage,
        Unstage,
        SwitchBranch,
        RebaseOnBranch
    };

    // Type of versioning system
    enum ERepositoryType
    {
        UnknownRepositoryType,
        Git,
        Gerrit,
        CVS,
        SVN,
        HG,     // Mercurial
    };

    // Status of a repository
    enum ERepositoryStatus
    {
        NoMerge,
        Merge,
        Rebase,
        InteractiveRebase
    };

    // Label types
    enum ELabelType
    {
        UnknownLabel,
        LocalBranchLabel,
        RemoteBranchLabel,
        TagLabel,
        UpToDateLabel,
        UnsyncedLabel,
        DivergedLabel
    };

    // Diff operations
    enum EDiffOperation
    {
        None,
        DiffFileName,
        DiffHunk,
        DiffAdd,
        DiffDelete
    };

    // Sort field
    enum ESortField
    {
        SortFileName,
        SortFullName
    };

    Q_ENUMS(ECapability)
    Q_ENUMS(ERepositoryType)
    Q_ENUMS(ERepositoryStatus)
    Q_ENUMS(ELabelType)
    Q_ENUMS(EDiffOperation)
    Q_ENUMS(ESortField)

    //-------------------------------------------------------------------------------------------------
    // Internal enumerators
    //-------------------------------------------------------------------------------------------------

    // Command identifiers
    enum EProcessCommand
    {
        eNothing,
        eIssuedCommand,
        eNotification,
        eCloneRepository,
        eCloneRepositoryFinished,
        eAllFileStatus,
        eChangedFileStatus,
        eIgnoredFileStatus,
        eRepositoryStatus,
        eBranches,
        eBranchHeadCommit,
        eBranchCommitCountAhead,
        eBranchCommitCountBehind,
        eTags,
        eTagCommit,
        eCurrentBranch,
        eGraph,
        eBranchLog,
        eFileLog,
        eRefLog,
        eDeleteFile,
        eUndeleteFile,
        eCommitMessage,
        eStageFile,
        eStageAll,
        eRevertFile,
        eCommit,
        eAmend,
        ePush,
        eForcePush,
        ePushAsWIP,
        ePull,
        eFetch,
        eStashSave,
        eStashPop,
        eUnstagedFileDiff,
        eStagedFileDiff,
        eApplyPatch,
        eTwoCommitDiff,
        eSetCurrentBranch,
        eResetToCommit,
        eRebaseOnCommit,
        eSquashCommit,
        eChangeCommitMessage,
        eCreateBranchOnCommit,
        eRebaseOnBranch,
        eMergeBranch,
        eDeleteBranch,
        eCreateTagOnCommit,
        eContinueRebase,
        eAbortRebase,
        ePatchCreate,
        ePatchApply,
        eBlame,
        eCleanUp,
        eUpdateSubmodules,
        eSubmoduleStatus,
        ePollFingerprint
    };

    // Status of a file in repository
    enum ERepoFileStatus
    {
        eClean,
        eAdded,
        eModified,
        eRenamed,
        eDeleted,
        eMissing,
        eUntracked,
        eIgnored
    };
};
