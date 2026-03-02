
// Qt
#include <QDebug>
#include <QRegularExpression>
#include <QDir>

// Application
#include "CHgCommands.h"

/*!
    \class CHgCommands
    \inmodule CuteGit
    \section1 General
    This is the repository control interface for Mercurial.

    \code

    /  \.-"""-./  \
    \    -   -    /
     |   o   o   |
     \  .-'''-.  /
      '-\__Y__/-'
         `---`

    \endcode
*/

//-------------------------------------------------------------------------------------------------
// Constant numbers

static int iLogFormatValueCount = 4;

//-------------------------------------------------------------------------------------------------
// Command strings

static const char* sCommandAdd                = "hg add \"%1\"";
static const char* sCommandAmend              = "hg commit --amend --date now";
static const char* sCommandBranches           = "hg branches --template \"{branch} &&& {node}\\n\"";
static const char* sCommandBranchLog          = "hg log -f --template \"{node} &&& {desc} &&& {author} &&& {date}\\n\"";
static const char* sCommandCommit             = "hg commit -m \"%1\"";
static const char* sCommandCreateBranch       = "hg branch \"%1\"";
static const char* sCommandCurrentBranch      = "hg identify -b";
static const char* sCommandFileLog            = "hg log -f --template \"{node} &&& {desc} &&& {author} &&& {date}\\n\" \"%1\"";
// static const char* sCommandGraph  = "hg log -G";
static const char* sCommandPull               = "hg pull";
static const char* sCommandPush               = "hg push --new-branch";
static const char* sCommandRevert             = "hg revert \"%1\"";
static const char* sCommandStatus             = "hg status";
static const char* sCommandStatusForFile      = "hg status \"%1\"";
static const char* sCommandResetOnCommit      = "hg update \"%1\"";

//-------------------------------------------------------------------------------------------------
// Regular expressions

static const char* sStatusRegExp = "([a-zA-Z?!])\\s(.*)";

//-------------------------------------------------------------------------------------------------
// Other strings

static const char* sLogFormatSplitter   = "&&&";

//-------------------------------------------------------------------------------------------------
// Status characters

const QString sStatusClean       = "C";
const QString sStatusAdded       = "A";
const QString sStatusModified    = "M";
const QString sStatusDeleted     = "R";
const QString sStatusUntracked   = "?";
const QString sStatusMissing     = "!";
const QString sStatusIgnored     = "I";
const QString sStatusOriginOfAdd = " ";

//-------------------------------------------------------------------------------------------------

CHgCommands::CHgCommands(CController* pController)
    : CCommands(pController)
{
    connect(this, &CCommands::execFinished, this, &CHgCommands::onExecFinished);
}

//-------------------------------------------------------------------------------------------------

CHgCommands::~CHgCommands()
{
}

//-------------------------------------------------------------------------------------------------

bool CHgCommands::can(CEnums::ECapability eWhat) const
{
    switch (eWhat)
    {
    case CEnums::Clone:
    case CEnums::ShowGlobalGraph:
    case CEnums::ShowBranchLog:
    case CEnums::ShowFileLog:
    case CEnums::Commit:
    case CEnums::Amend:
    case CEnums::Pull:
    case CEnums::Push:
    case CEnums::BranchFromCommit:
        return true;

    default:
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::allFileStatus(const QString& sPath)
{
    CCleanFileLister* pLister = new CCleanFileLister();

    pLister->m_eCommand = CEnums::eAllFileStatus;
    pLister->m_sRootPath = sPath;
    pLister->setAutoDelete(true);

    connect(pLister, &CCleanFileLister::newOutputListOfCRepoFile, this, &CHgCommands::onNewOutputListOfCRepoFile);

    m_tPool.start(pLister);
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::changedFileStatus(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eChangedFileStatus, sPath, sCommandStatus));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::branches(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eBranches, sPath, sCommandBranches));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::branchHeadCommits(const QString& sPath, QStringList lBranches)
{
    Q_UNUSED(sPath);
    Q_UNUSED(lBranches);
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::branchLog(const QString& sPath, int iFrom, int iCount)
{
    Q_UNUSED(iCount);
    int iPotentialCount = 0;
    QString sCommand = QString(sCommandBranchLog);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eBranchLog, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::fileLog(const QString& sPath, const QString& sFullName, int iFrom, int iCount)
{
    Q_UNUSED(iCount);
    int iPotentialCount = 0;
    QString sCommand = QString(sCommandFileLog).arg(sFullName);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eFileLog, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::toggleStaged(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandStatusForFile).arg(sFullName);
    QString sLine = execNow(sPath, sCommand);

    CRepoFile* pFile = repoFileForLine(sPath, sLine);

    if (pFile != nullptr)
    {
        stageFile(sPath, sFullName, not pFile->staged());
        delete pFile;
    }
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::stageFile(const QString& sPath, const QString& sFullName, bool bStage)
{
    QString sCommand = QString(bStage ? sCommandAdd : sCommandRevert).arg(sFullName);
    exec(new CProcessCommand(CEnums::eStageFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::commit(const QString& sPath, const QString& sMessage)
{
    emit newOutputString(CEnums::eNotification, tr("Commiting..."));
    QString sCommand = QString(sCommandCommit).arg(sMessage);
    exec(new CProcessCommand(CEnums::eCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::amend(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Amending..."));
    QString sCommand = QString(sCommandAmend);
    exec(new CProcessCommand(CEnums::eAmend, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::push(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Pushing..."));
    QString sCommand = QString(sCommandPush);
    exec(new CProcessCommand(CEnums::ePush, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::pull(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Pulling..."));
    QString sCommand = QString(sCommandPull);
    exec(new CProcessCommand(CEnums::ePull, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::createBranchOnCommit(const QString& sPath, const QString& sCommitId, const QString& sBranchName)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Creating branch %1...")).arg(sBranchName));
    QString sCommand, sOutput;
    sCommand = QString(sCommandResetOnCommit).arg(sCommitId);
    sOutput = execNow(sPath, sCommand);
    sCommand = QString(sCommandCreateBranch).arg(sBranchName);
    exec(new CProcessCommand(CEnums::eCreateBranchOnCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

CRepoFile* CHgCommands::repoFileForLine(const QString &sPath, QString sLine)
{
    QRegularExpression tRegExp(sStatusRegExp);

#ifdef HAVE_QSTRING_BACK
    if (not sLine.isEmpty() && sLine.back() == '/')
        sLine.chop(1);
#endif

    QRegularExpressionMatch match = tRegExp.match(sLine);
    if (match.hasMatch())
    {
        QString sStatus = match.captured(1).trimmed();
        QString sRelativeName = match.captured(2).trimmed();
        QString sFullName = sPath + PATH_SEP + sRelativeName;
        QString sFileName = QFileInfo(sFullName).fileName();
        bool bStaged = true;

        CEnums::ERepoFileStatus eStatus = CEnums::eClean;

        if (not sStatus.isEmpty())
        {
            if (sStatus == sStatusClean)
                eStatus = CEnums::eClean;
            else if (sStatus == sStatusAdded)
                eStatus = CEnums::eAdded;
            else if (sStatus == sStatusModified)
                eStatus = CEnums::eModified;
            else if (sStatus == sStatusDeleted)
                eStatus = CEnums::eDeleted;
            else if (sStatus == sStatusIgnored)
                eStatus = CEnums::eIgnored;
            else if (sStatus == sStatusMissing)
                eStatus = CEnums::eMissing;
            else if (sStatus == sStatusUntracked)
            {
                eStatus = CEnums::eUntracked;
                bStaged = false;
            }
            else if (sStatus == sStatusOriginOfAdd)
            {
            }
        }

        CRepoFile* pFile = new CRepoFile();
        pFile->setFullName(sFullName);
        pFile->setFileName(sFileName);
        pFile->setRelativeName(sRelativeName);
        pFile->setStatus(eStatus);
        pFile->setStaged(bStaged);

        return pFile;
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::handleBranchOutput(const CProcessResult& tResult)
{
    // Create CBranchs with the returned string of the process

    QList<CBranch*> lReturnValue;
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lLines)
    {
        QStringList sValues = sLine.split(sLogFormatSplitter);

        if (sValues.count() == 2)
        {
            QString sName = sValues[0];
            QString sCommitId = sValues[1];

            CBranch* pNewBranch = new CBranch();
            pNewBranch->setName(sName);
            pNewBranch->setType(CEnums::RemoteBranchLabel);
            pNewBranch->setCommitId(sCommitId);

            lReturnValue << pNewBranch;
        }
    }

    QString sName = execNow(tResult.m_sPath, sCommandCurrentBranch).trimmed();
    emit newOutputString(CEnums::eCurrentBranch, sName);

    emit newOutputListOfCBranch(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::handleFileStatusOutput(const CProcessResult& tResult)
{
    // Create CRepoFiles with the returned string of the process

    CRepoFileList lReturnValue;
    QStringList lStrings = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lStrings)
    {
        CRepoFile* pFile = repoFileForLine(tResult.m_sPath, sLine);

        if (pFile != nullptr)
            lReturnValue.addItem(pFile->fullName(), pFile);
    }

    emit newOutputListOfCRepoFile(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::handleLogOutput(const CProcessResult& tResult)
{
    // Create a CLogLineCollection with the returned string of the process

    CLogLineCollection lReturnValue;
    QStringList lStrings = tResult.m_sValue.split(NEW_LINE);

    for (QString sLine : lStrings)
    {
        QStringList sValues = sLine.split(sLogFormatSplitter);

        if (sValues.count() == iLogFormatValueCount)
        {
            CLogLine* pLine = new CLogLine();

            qint64 iData = qint64(sValues[3].trimmed().toDouble());

            pLine->setCommitId(sValues[0].trimmed());
            pLine->setMessage(sValues[1].trimmed());
            pLine->setAuthor(sValues[2].trimmed());
            pLine->setDate(QDateTime::fromSecsSinceEpoch(iData));
            pLine->setMessageIsComplete(true);

            lReturnValue.add(pLine);
        }
    }

    QStringList lUserValues = tResult.m_sUserData.split(",");
    int iPotentialCount = lUserValues[0].toInt();
    int iStartIndex = lUserValues[1].toInt();

    if (iPotentialCount == 0)
        iPotentialCount = lReturnValue.lines().count();

    lReturnValue.setPotentialCount(iPotentialCount);
    lReturnValue.setStartIndex(iStartIndex);

    emit newOutputCLogLineCollection(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::onExecFinished(const CProcessResult& tResult)
{
    Q_UNUSED(tResult.m_sUserData);

    switch (tResult.m_eCommand)
    {

    case CEnums::eStageFile:
    case CEnums::eStageAll:
    case CEnums::eRevertFile:
    case CEnums::eCommit:
    case CEnums::eAmend:
    case CEnums::ePush:
    case CEnums::ePull:
    case CEnums::eCreateBranchOnCommit:
    {
        // Throw the returned string of the process
        emit newOutputString(tResult.m_eCommand, tResult.m_sValue);
        break;
    }

    case CEnums::eBranches:
    {
        handleBranchOutput(tResult);
        break;
    }

    case CEnums::eAllFileStatus:
    case CEnums::eChangedFileStatus:
    {
        handleFileStatusOutput(tResult);
        break;
    }

    case CEnums::eFileLog:
    case CEnums::eBranchLog:
    {
        handleLogOutput(tResult);
        break;
    }

    default:
    {
        break;
    }

    }
}

//-------------------------------------------------------------------------------------------------

void CHgCommands::onNewOutputListOfCRepoFile(CEnums::EProcessCommand eCommand, CRepoFileList lNewRepoFiles)
{
    emit newOutputListOfCRepoFile(eCommand, lNewRepoFiles);
}
