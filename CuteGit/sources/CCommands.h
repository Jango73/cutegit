
#pragma once

// Qt
#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QThread>
#include <QMutex>

// Application
#include "CRepoFile.h"
#include "CGraphLine.h"

//-------------------------------------------------------------------------------------------------

class CProcessCommand : public QObject
{
    Q_OBJECT

public:

    enum EProcessCommand
    {
        eNotification,
        eAllFileStatus,
        eRepositoryStatus,
        eBranches,
        eBranchLog,
        eCurrentBranch,
        eFileLog,
        eStageFile,
        eStageAll,
        eRevertFile,
        eCommit,
        eAmend,
        ePush,
        ePull,
        eUnstagedFileDiff,
        eSetCurrentBranch
    };

    Q_ENUMS(EProcessCommand)

    CProcessCommand(EProcessCommand eCommand, QString sWorkPath, QString sCommand)
        : m_eCommand(eCommand)
        , m_sWorkPath(sWorkPath)
        , m_sCommand(sCommand)
    {
    }

    EProcessCommand m_eCommand;
    QString m_sWorkPath;
    QString m_sCommand;
};

//-------------------------------------------------------------------------------------------------

class CCommands : public QThread
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Constructor
    CCommands();

    //! Destructor
    virtual ~CCommands() override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    virtual void allFileStatus(const QString& sPath);

    //!
    virtual void repositoryStatus(const QString& sPath);

    //!
    virtual void branches(const QString& sPath);

    //!
    virtual void branchLog(const QString& sPath, const QDateTime& from, const QDateTime& to);

    //!
    virtual void fileLog(const QString& sPath, const QString& sFullName);

    //!
    virtual void stageFile(const QString& sPath, const QString& sFullName, bool bStage);

    //!
    virtual void stageAll(const QString& sPath, bool bStage);

    //!
    virtual void revertFile(const QString& sPath, const QString& sFullName);

    //!
    virtual void commit(const QString& sPath, const QString& sMessage);

    //!
    virtual void amend(const QString& sPath);

    //!
    virtual void push(const QString& sPath);

    //!
    virtual void pull(const QString& sPath);

    //!
    virtual void unstagedFileDiff(const QString& sPath, const QString& sFullName);

    //!
    virtual void setCurrentBranch(const QString& sPath, const QString& sBranch);

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    void exec(CProcessCommand* pCommand);

    //!
    QString execNow(QString m_sWorkPath, QString m_sCommand);

    //-------------------------------------------------------------------------------------------------
    // Private control methods
    //-------------------------------------------------------------------------------------------------

private:

    //!
    virtual void run() override;

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    //!
    void execFinished(QString sPath, CProcessCommand::EProcessCommand eCommand, QString sValue);

    //!
    void newOutputString(CProcessCommand::EProcessCommand eCommand, QString sValue);

    //!
    void newOutputStringList(CProcessCommand::EProcessCommand eCommand, QStringList lValue);

    //!
    void newOutputListOfCRepoFile(CProcessCommand::EProcessCommand eCommand, QList<CRepoFile*> vNewRepoFiles);

    //!
    void newOutputListOfCGraphLine(CProcessCommand::EProcessCommand eCommand, QList<CGraphLine*> vNewGraphLines);

private:

    bool                    m_bStop;
    QMutex                  m_mMutex;
    QList<CProcessCommand*> m_lCommands;
};
