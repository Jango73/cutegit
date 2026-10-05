
#pragma once

// Qt
#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QThread>
#include <QMutex>
#include <QRecursiveMutex>
#include <QThreadPool>
#include <QRunnable>
#include <QProcess>

// Application
#include "../CEnums.h"
#include "../CBranch.h"
#include "../CRepoFile.h"
#include "../CLogLineCollection.h"
#include "../CDiffLine.h"
#include "../CGraphLine.h"

//-------------------------------------------------------------------------------------------------

class CProcessCommand : public QObject
{
    Q_OBJECT

public:

    CProcessCommand(
            CEnums::EProcessCommand eCommand,
            QString sWorkPath,
            QString sCommand,
            bool bAllowStack = false,
            QMap<QString, QString> mEnvironment = QMap<QString, QString>(),
            QString sUserData = "",
            CEnums::EProcessCommand eEndSignal = CEnums::eNothing
            )
        : m_eCommand(eCommand)
        , m_eEndSignal(eEndSignal)
        , m_bAllowStack(bAllowStack)
        , m_sWorkPath(sWorkPath)
        , m_sCommand(sCommand)
        , m_mEnvironment(mEnvironment)
        , m_sUserData(sUserData)
    {
    }

    CEnums::EProcessCommand m_eCommand;
    CEnums::EProcessCommand m_eEndSignal;
    bool                    m_bAllowStack;
    QString                 m_sWorkPath;
    QString                 m_sCommand;
    QMap<QString, QString>  m_mEnvironment;
    QString                 m_sUserData;
};

//-------------------------------------------------------------------------------------------------

class CProcessResult : public QObject
{
    Q_OBJECT

public:

    //! Default constructor
    CProcessResult(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    //! Constructor with parameters
    CProcessResult(
            QString sPath,
            CEnums::EProcessCommand eCommand,
            QString sValue,
            QString sUserData)
        : m_sPath(sPath)
        , m_eCommand(eCommand)
        , m_sValue(sValue)
        , m_sUserData(sUserData)
    {
    }

    //! Copy constructor
    CProcessResult(const CProcessResult& target, QObject* parent = nullptr)
        : QObject(parent)
        , m_sPath(target.m_sPath)
        , m_eCommand(target.m_eCommand)
        , m_sValue(target.m_sValue)
        , m_sUserData(target.m_sUserData)
    {
    }

    QString                 m_sPath;
    CEnums::EProcessCommand m_eCommand;
    QString                 m_sValue;
    QString                 m_sUserData;
};

Q_DECLARE_METATYPE(CProcessResult)

//-------------------------------------------------------------------------------------------------

class CExecution : public QThread
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Constructor
    CExecution();

    //! Destructor
    virtual ~CExecution() override;

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    void exec(CProcessCommand* pCommand);

    //!
    QString execNow(QString sWorkPath,
                    QString sCommand,
                    QMap<QString, QString> mEnvironment = QMap<QString, QString>()
                    );

    //!
    QString execNowLiveFeed(
            CEnums::EProcessCommand eCommand,
            QString sWorkPath,
            QString sCommand,
            QMap<QString, QString> mEnvironment = QMap<QString, QString>()
            );

    //-------------------------------------------------------------------------------------------------
    // Private control methods
    //-------------------------------------------------------------------------------------------------

private:

    //!
    virtual void run() override;

    //! Starts a process from a command line, split like Qt5 did
    static void startProcess(QProcess& tProcess, const QString& sCommand);

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    //!
    void execFinished(const CProcessResult& tResult);

    //!
    void newOutputString(CEnums::EProcessCommand eCommand, QString sValue);

    //!
    void newOutputKeyValue(CEnums::EProcessCommand eCommand, QString sKey, QString sValue);

    //!
    void newOutputStringList(CEnums::EProcessCommand eCommand, QStringList lValue);

    //!
    void newOutputListOfCBranch(CEnums::EProcessCommand eCommand, QList<CBranch*> lNewBranches);

    //!
    void newOutputListOfCRepoFile(CEnums::EProcessCommand eCommand, CRepoFileList lNewRepoFiles);

    //!
    void newOutputCLogLineCollection(CEnums::EProcessCommand eCommand, CLogLineCollection lNewLines);

    //!
    void newOutputListOfCDiffLine(CEnums::EProcessCommand eCommand, QList<CDiffLine*> lNewLines);

    //!
    void newOutputListOfCGraphLine(CEnums::EProcessCommand eCommand, QList<CGraphLine*> lNewLines);

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

protected:

    QThreadPool                 m_tPool;

private:

    bool                        m_bStop;
    QRecursiveMutex             m_mMutex;
    CEnums::EProcessCommand     m_eLastShownCommand;
    QList<CProcessCommand*>     m_lCommandStack;
};

//-------------------------------------------------------------------------------------------------

class CCleanFileLister : public QObject, public QRunnable
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    virtual void run() override;

    //!
    void getAllFiles(CRepoFileList& lFileList, const QString& sRootPath, const QString& sCurrentPath);

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    //!
    void newOutputListOfCRepoFile(CEnums::EProcessCommand eCommand, CRepoFileList lNewRepoFiles);

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

public:

    CEnums::EProcessCommand     m_eCommand;
    QString                     m_sRootPath;
};
