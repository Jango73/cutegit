
#pragma once

// Qt
#include <QObject>

// Application
#include "CCommands.h"

//-------------------------------------------------------------------------------------------------

class CHgCommands : public CCommands
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Enumerators
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Constructor
    CHgCommands(CController* pController);

    //! Destructor
    virtual ~CHgCommands() override;

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    virtual bool can(CEnums::ECapability eWhat) const override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    virtual void allFileStatus(const QString& sPath) override;

    //!
    virtual void changedFileStatus(const QString& sPath) override;

    //!
    virtual void branches(const QString& sPath) override;

    //!
    virtual void branchHeadCommits(const QString& sPath, QStringList lBranches) override;

    //!
    virtual void branchLog(const QString& sPath, int iFrom = 0, int iCount = LOG_COUNT_DEFAULT) override;

    //!
    virtual void fileLog(const QString& sPath, const QString& sFullName, int iFrom = 0, int iCount = LOG_COUNT_DEFAULT) override;

    //!
    virtual void toggleStaged(const QString& sPath, const QString& sFullName) override;

    //!
    virtual void stageFile(const QString& sPath, const QString& sFullName, bool bStage) override;

    //!
    virtual void commit(const QString& sPath, const QString& sMessage) override;

    //!
    virtual void pollFingerprint(const QString& sPath) override;

    //!
    virtual void amend(const QString& sPath) override;

    //!
    virtual void push(const QString& sPath) override;

    //!
    virtual void forcePush(const QString& sPath) override;

    //!
    virtual void pull(const QString& sPath) override;

    //!
    virtual void createBranchOnCommit(const QString& sPath, const QString& sCommitId, const QString& sBranchName) override;

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

    //-------------------------------------------------------------------------------------------------
    // Protected slots
    //-------------------------------------------------------------------------------------------------

protected slots:

    //!
    void onExecFinished(const CProcessResult& tResult);

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

protected:

    //! First half of the poll fingerprint, while the second half is running
    QString m_sPollStatusPart;
};
