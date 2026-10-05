
// Qt
#include <QDebug>
#include <QCoreApplication>
#include <QProcess>
#include <QMutexLocker>
#include <QDir>

// Application
#include "CExecution.h"

/*!
    \class CExecution
    \inmodule CuteGit
    \section1 General
    This class allows execution of processes.
*/

//-------------------------------------------------------------------------------------------------

CExecution::CExecution()
    : m_tPool(this)
    , m_bStop(false)
    , m_mMutex()
    , m_eLastShownCommand(CEnums::eNothing)
{
    start();
}

//-------------------------------------------------------------------------------------------------

CExecution::~CExecution()
{
    m_bStop = true;
    wait();
}

//-------------------------------------------------------------------------------------------------

// This method does NOT run in the main thread
// Watch out for data integrity protection
void CExecution::run()
{
    while (not m_bStop)
    {
        bool bStackEmpty = true;
        CProcessCommand* pCommand = nullptr;

        // Open a scope for the mutex locker
        {
            // Lock access to the command stack
            QMutexLocker locker(&m_mMutex);

            if (m_lCommandStack.count() > 0)
            {
                bStackEmpty = false;
                pCommand = m_lCommandStack[0];
                m_lCommandStack.removeAt(0);
            }
        }

        if (pCommand != nullptr)
        {
            if (m_eLastShownCommand != pCommand->m_eCommand)
            {
                m_eLastShownCommand = pCommand->m_eCommand;

                emit execFinished(CProcessResult(pCommand->m_sWorkPath, CEnums::eIssuedCommand, pCommand->m_sCommand, ""));
            }

            if (pCommand->m_eEndSignal == CEnums::eNothing)
            {
                QString sOutput = execNow(
                            pCommand->m_sWorkPath,
                            pCommand->m_sCommand,
                            pCommand->m_mEnvironment
                            );

                emit execFinished(CProcessResult(pCommand->m_sWorkPath, pCommand->m_eCommand, sOutput, pCommand->m_sUserData));
            }
            else
            {
                QString sOutput = execNowLiveFeed(
                            pCommand->m_eCommand,
                            pCommand->m_sWorkPath,
                            pCommand->m_sCommand,
                            pCommand->m_mEnvironment
                            );

                emit execFinished(CProcessResult(pCommand->m_sWorkPath, pCommand->m_eEndSignal, sOutput, pCommand->m_sUserData));
            }

            delete pCommand;
        }

        msleep(bStackEmpty ? 100 : 2);
    }
}

//-------------------------------------------------------------------------------------------------

void CExecution::exec(CProcessCommand* pCommand)
{
    // Lock access to the command stack
    QMutexLocker locker(&m_mMutex);

    // Leave only one command of a given type in the stack
    // if not allowed to stack the type
    if (not pCommand->m_bAllowStack)
    {
        for (int index = 0; index < m_lCommandStack.count(); index++)
        {
            if (m_lCommandStack[index]->m_eCommand == pCommand->m_eCommand)
            {
                delete m_lCommandStack[index];
                m_lCommandStack.removeAt(index);
                index--;
            }
        }
    }

    m_lCommandStack << pCommand;
}

//-------------------------------------------------------------------------------------------------

void CExecution::startProcess(QProcess& tProcess, const QString& sCommand)
{
    QStringList lArguments = QProcess::splitCommand(sCommand);

    if (lArguments.isEmpty())
        return;

    QString sProgram = lArguments.takeFirst();
    tProcess.start(sProgram, lArguments);
}

//-------------------------------------------------------------------------------------------------

QString CExecution::execNow(QString sWorkPath, QString sCommand, QMap<QString, QString> mEnvironment)
{
    QProcess process;

    if (not sWorkPath.isEmpty())
        process.setWorkingDirectory(sWorkPath);

    if (not mEnvironment.isEmpty())
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

        for (QString sKey : mEnvironment.keys())
            env.insert(sKey, mEnvironment[sKey]);

        process.setProcessEnvironment(env);
    }

    startProcess(process, sCommand);
    process.waitForFinished();

    QString sOutput = process.readAllStandardOutput();
    sOutput += process.readAllStandardError();

    return sOutput;
}

//-------------------------------------------------------------------------------------------------

QString CExecution::execNowLiveFeed(CEnums::EProcessCommand eCommand, QString sWorkPath, QString sCommand, QMap<QString, QString> mEnvironment)
{
    QProcess process;

    if (not sWorkPath.isEmpty())
        process.setWorkingDirectory(sWorkPath);

    if (not mEnvironment.isEmpty())
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

        for (QString sKey : mEnvironment.keys())
            env.insert(sKey, mEnvironment[sKey]);

        process.setProcessEnvironment(env);
    }

    startProcess(process, sCommand);
    msleep(1000);

    while (process.state() == QProcess::Running)
    {
        QCoreApplication::processEvents();

        QString sOutput = process.readAllStandardOutput();
        sOutput += process.readAllStandardError();

        if (not sOutput.isEmpty())
        {
            emit newOutputString(eCommand, sOutput);
        }

        if (m_bStop || QThread::currentThread()->isInterruptionRequested())
            break;

        msleep(20);
    }

    process.waitForFinished();
    QString sOutput = process.readAllStandardOutput();
    sOutput += process.readAllStandardError();

    return sOutput;
}

//-------------------------------------------------------------------------------------------------

void CCleanFileLister::run()
{
    CRepoFileList lFileList;
    getAllFiles(lFileList, m_sRootPath, m_sRootPath);

    emit newOutputListOfCRepoFile(m_eCommand, lFileList);
}

//-------------------------------------------------------------------------------------------------

void CCleanFileLister::getAllFiles(CRepoFileList& lFileList, const QString& sRootPath, const QString& sCurrentPath)
{
    QStringList slNameFilter;
    slNameFilter << "*";

    QDir dRoot(sRootPath);
    QDir dDirectory(sCurrentPath);

    dDirectory.setFilter(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files);
    QStringList lFiles = dDirectory.entryList(slNameFilter);

    for (QString sFile : lFiles)
    {
        QString sFullName = QString("%1/%2").arg(sCurrentPath).arg(sFile);
        QFileInfo info(sFullName);

        CRepoFile* pNewFile = new CRepoFile();

        pNewFile->setStatus(CEnums::eClean);
        pNewFile->setFullName(sFullName);
        pNewFile->setRelativeName(dRoot.relativeFilePath(sFullName));
        pNewFile->setFileName(info.fileName());

        lFileList.addItem(pNewFile->fullName(), pNewFile);
    }

    dDirectory.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList lDirectories = dDirectory.entryList();

    for (QString sNewDirectory : lDirectories)
    {
        QString sFullName = QString("%1/%2").arg(sCurrentPath).arg(sNewDirectory);

        getAllFiles(lFileList, sRootPath, sFullName);
    }
}
