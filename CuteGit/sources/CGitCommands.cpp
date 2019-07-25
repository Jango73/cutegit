
// Qt
#include <QDebug>
#include <QRegExp>
#include <QDir>
#include <QCoreApplication>

// Application
#include "CGitCommands.h"

//-------------------------------------------------------------------------------------------------

static int iLogFormatValueCount = 4;
static const char* sLogFormatSplitter = "|";

static const char* sCommandStatus = "git status --ignored --porcelain";
static const char* sCommandStatusForFile = "git status --ignored --porcelain \"%1\"";
static const char* sCommandBranches = "git branch -a";
// static const char* sCommandGraph = "git log --graph --pretty=format:\"%h | %s | %an | %ai\" --after=\"%1\" --before=\"%2\"";
static const char* sCommandBranchLog = "git log --pretty=format:\"%h | %s | %an | %aI\" --max-count=20";
static const char* sCommandFileLog = "git log --pretty=format:\"%h | %s | %an | %aI\" --max-count=20 \"%1\"";
static const char* sCommandStage = "git add -f \"%1\"";
static const char* sCommandUnstage = "git reset HEAD \"%1\"";
static const char* sCommandStageAll = "git add -u";
static const char* sCommandUnstageAll = "git reset .";
static const char* sCommandRevert = "git checkout \"%1\"";
static const char* sCommandCommit = "git commit -m \"%1\"";
static const char* sCommandAmend = "git commit --amend --reset-author --no-edit";
static const char* sCommandPush = "git push";
static const char* sCommandPull = "git pull";
static const char* sCommandUnstagedDiff = "git diff --no-color --ignore-all-space \"%1\"";
static const char* sCommandSetCurrentBranch = "git checkout \"%1\"";
// static const char* sCommandInteractiveRebase = "git rebase --interactive";
static const char* sCommandResetOnCommit = "git reset %1";
static const char* sCommandRebaseOnCommit = "git rebase --interactive %1~1";
static const char* sCommandContinueRebase = "git rebase --continue";
static const char* sCommandAbortRebase = "git rebase --abort";

static const char* sCommandGetRebaseApplyPath = "git rev-parse --git-path rebase-apply";
static const char* sCommandGetRebaseMergePath = "git rev-parse --git-path rebase-merge";

static const char* sStatusRegExp = "([a-zA-Z?!\\s])([a-zA-Z?!\\s])\\s(.*)";
static const char* sPickCommitRegExp = "(pick)\\s+([a-zA-Z0-9]+)\\s+(.*)";

static const char* sRemoteBranchPrefix = "remotes/origin/";

static const char* sSequenceEditorToken = "GIT_SEQUENCE_EDITOR";
static const char* sTextEditorToken = "GIT_EDITOR";

static const char* sRebaseEditCommit = "edit %1 %2";
static const char* sRebaseRewordCommit = "reword %1 %2";

static const char* sComment = "#";

const QString sStatusAdded = "A";
const QString sStatusModified = "M";
const QString sStatusRenamed = "R";
const QString sStatusDeleted = "D";
const QString sStatusUntracked = "?";
const QString sStatusIgnored = "!";

//-------------------------------------------------------------------------------------------------

CGitCommands::CGitCommands()
{
    connect(this, &CCommands::execFinished, this, &CGitCommands::onExecFinished);
}

//-------------------------------------------------------------------------------------------------

CGitCommands::~CGitCommands()
{
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::allFileStatus(const QString& sPath)
{
    exec(new CProcessCommand(CProcessCommand::eAllFileStatus, sPath, sCommandStatus));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::repositoryStatus(const QString& sPath)
{
    QString sRebaseApplyPath = QString("%1/%2").arg(sPath).arg(execNow(sPath, sCommandGetRebaseApplyPath).trimmed());
    QString sRebaseMergePath = QString("%1/%2").arg(sPath).arg(execNow(sPath, sCommandGetRebaseMergePath).trimmed());

    if (QDir(sRebaseMergePath).exists())
    {
        emit newOutputString(
                    CProcessCommand::eRepositoryStatus,
                    CRepoFile::sRepositoryStatusInteractiveRebase
                    );
    }
    else if (QDir(sRebaseApplyPath).exists())
    {
        emit newOutputString(
                    CProcessCommand::eRepositoryStatus,
                    CRepoFile::sRepositoryStatusRebase
                    );
    }
    else
    {
        emit newOutputString(
                    CProcessCommand::eRepositoryStatus,
                    CRepoFile::sRepositoryStatusClean
                    );
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::branches(const QString& sPath)
{
    exec(new CProcessCommand(CProcessCommand::eBranches, sPath, sCommandBranches));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::branchLog(const QString& sPath, const QDateTime& from, const QDateTime& to)
{
    QString sFrom = from.toString(Qt::ISODate);
    QString sTo = to.toString(Qt::ISODate);
    QString sCommand = QString(sCommandBranchLog); // .arg(sFrom).arg(sTo);
    exec(new CProcessCommand(CProcessCommand::eBranchLog, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::fileLog(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandFileLog).arg(sFullName);
    exec(new CProcessCommand(CProcessCommand::eFileLog, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::toggleStaged(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandStatusForFile).arg(sFullName);
    QString sLine = execNow(sPath, sCommand);

    CRepoFile* pFile = repoFileForLine(sPath, sLine);

    if (pFile != nullptr)
    {
        stageFile(sPath, sFullName, not pFile->staged());
        pFile->deleteLater();
    }
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::stageFile(const QString& sPath, const QString& sFullName, bool bStage)
{
    QString sCommand = QString(bStage ? sCommandStage : sCommandUnstage).arg(sFullName);
    exec(new CProcessCommand(CProcessCommand::eStageFile, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::stageAll(const QString& sPath, bool bStage)
{
    QString sCommand = QString(bStage ? sCommandStageAll : sCommandUnstageAll);
    exec(new CProcessCommand(CProcessCommand::eStageAll, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::revertFile(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandRevert).arg(sFullName);
    exec(new CProcessCommand(CProcessCommand::eRevertFile, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commit(const QString& sPath, const QString& sMessage)
{
    QString sCommand = QString(sCommandCommit).arg(sMessage);
    exec(new CProcessCommand(CProcessCommand::eCommit, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::amend(const QString& sPath)
{
    QString sCommand = QString(sCommandAmend);
    exec(new CProcessCommand(CProcessCommand::eAmend, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::push(const QString& sPath)
{
    emit newOutputString(CProcessCommand::eNotification, tr("Pushing..."));
    QString sCommand = QString(sCommandPush);
    exec(new CProcessCommand(CProcessCommand::ePush, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::pull(const QString& sPath)
{
    emit newOutputString(CProcessCommand::eNotification, tr("Pulling..."));
    QString sCommand = QString(sCommandPull);
    exec(new CProcessCommand(CProcessCommand::ePull, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::unstagedFileDiff(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandUnstagedDiff).arg(sFullName);
    exec(new CProcessCommand(CProcessCommand::eUnstagedFileDiff, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::setCurrentBranch(const QString& sPath, const QString& sBranch)
{
    QString sFinalName = sBranch;
    sFinalName.replace(sRemoteBranchPrefix, "");
    exec(new CProcessCommand(CProcessCommand::eSetCurrentBranch, sPath, QString(sCommandSetCurrentBranch).arg(sFinalName)));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commitReset(const QString& sPath, const QString& sCommitId)
{
    emit newOutputString(CProcessCommand::eNotification, tr("Doing reset..."));
    QString sCommand = QString(sCommandResetOnCommit).arg(sCommitId);
    exec(new CProcessCommand(CProcessCommand::eCommitReset, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::commitRebase(const QString& sPath, const QString& sCommitId)
{
    emit newOutputString(CProcessCommand::eNotification, tr("Doing rebase..."));

    m_eRebaseType = eRTEdit;
    m_eRebaseStep = eRSChangeCommitEditSequence;
    m_sCommitId = sCommitId;
    m_sCommitMessage = "";

    QMap<QString, QString> mEnvironment;
    mEnvironment[sSequenceEditorToken] = QCoreApplication::applicationFilePath();

    QString sCommand = QString(sCommandRebaseOnCommit).arg(sCommitId);

    exec(new CProcessCommand(CProcessCommand::eCommitRebase, sPath, sCommand, mEnvironment));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::changeCommitMessage(const QString& sPath, const QString& sCommitId, const QString& sMessage)
{
    emit newOutputString(CProcessCommand::eNotification, tr("Doing rebase and commit message changes..."));

    m_eRebaseType = eRTReword;
    m_eRebaseStep = eRSChangeCommitEditSequence;
    m_sCommitId = sCommitId;
    m_sCommitMessage = sMessage;

    QMap<QString, QString> mEnvironment;
    mEnvironment[sSequenceEditorToken] = QCoreApplication::applicationFilePath();
    mEnvironment[sTextEditorToken] = QCoreApplication::applicationFilePath();

    QString sCommand = QString(sCommandRebaseOnCommit).arg(sCommitId);

    exec(new CProcessCommand(CProcessCommand::eChangeCommitMessage, sPath, sCommand, mEnvironment));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::continueRebase(const QString& sPath)
{
    emit newOutputString(CProcessCommand::eNotification, tr("Continuing rebase..."));
    QString sCommand = QString(sCommandContinueRebase);
    exec(new CProcessCommand(CProcessCommand::eContinueRebase, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::abortRebase(const QString& sPath)
{
    emit newOutputString(CProcessCommand::eNotification, tr("Aborting rebase..."));
    QString sCommand = QString(sCommandAbortRebase);
    exec(new CProcessCommand(CProcessCommand::eAbortRebase, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CGitCommands::editSequenceFile(const QString& sFileName)
{
    QFile file(sFileName);

    if (file.open(QIODevice::ReadOnly))
    {
        // Read in the file provided by GIT
        QTextStream stream(&file);
        QString sInputText = stream.readAll();
        QStringList lOutputLines;
        file.close();

        // Get a list of lines
        QStringList sLines = sInputText.split("\n");

        switch (m_eRebaseStep)
        {
        // In case we're modifyng the sequence file
        // Most often .git/rebase-merge/git-rebase-todo
        case eRSChangeCommitEditSequence:
        {
            // Setup a "pick nnnn aaaa" reg exp
            QRegExp tRegExp(sPickCommitRegExp);

            for (QString sLine : sLines)
            {
                // Process only non-comment lines
                if (not sLine.startsWith(sComment))
                {
                    if (tRegExp.indexIn(sLine) != -1)
                    {
                        QString sWord = tRegExp.cap(1).trimmed();
                        QString sCommitID = tRegExp.cap(2).trimmed();
                        QString sMessage = tRegExp.cap(3).trimmed();

                        if (sCommitID == m_sCommitId)
                        {
                            switch (m_eRebaseType)
                            {

                            case eRTEdit:
                            {
                                lOutputLines << QString(sRebaseEditCommit).arg(sCommitID).arg(sMessage);
                                break;
                            }

                            case eRTReword:
                            {
                                lOutputLines << QString(sRebaseRewordCommit).arg(sCommitID).arg(sMessage);
                                break;
                            }

                            }
                        }
                        else
                        {
                            lOutputLines << sLine;
                        }
                    }
                }
            }

            m_eRebaseStep = eRSChangeCommitEditMessage;
            break;
        }

        case eRSChangeCommitEditMessage:
        {
            lOutputLines = m_sCommitMessage.split("\n");
            break;
        }
        }

        // Rebuild output text from lines
        QString sOutputText;

        for (QString sLine : lOutputLines)
        {
            sOutputText += sLine;
            sOutputText += "\n";
        }

        // Rewrite sequence file
        if (file.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&file);
            stream << sOutputText;
            file.close();
        }
    }
}

//-------------------------------------------------------------------------------------------------

CRepoFile* CGitCommands::repoFileForLine(const QString &sPath, QString sLine)
{
    QRegExp tRegExp(sStatusRegExp);

    if (sLine.isEmpty() == false && sLine.back() == '/')
        sLine.chop(1);

    if (tRegExp.indexIn(sLine) != -1)
    {
        QString sStaged = tRegExp.cap(1).trimmed();
        QString sUnstaged = tRegExp.cap(2).trimmed();
        QString sRelativeName = tRegExp.cap(3).split("->").last().trimmed();
        QString sFullName = sPath + "/" + sRelativeName;
        QString sFileName = QFileInfo(sFullName).fileName();
        bool bStaged = false;

        CRepoFile::ERepoFileStatus eStatus = CRepoFile::eClean;

        if (sStaged.isEmpty() == false)
        {
            bStaged = true;

            if (sStaged == sStatusAdded)
                eStatus = CRepoFile::eAdded;
            else if (sStaged == sStatusModified)
                eStatus = CRepoFile::eModified;
            else if (sStaged == sStatusRenamed)
                eStatus = CRepoFile::eRenamed;
            else if (sStaged == sStatusDeleted)
                eStatus = CRepoFile::eDeleted;
            else if (sStaged == sStatusIgnored)
                eStatus = CRepoFile::eIgnored;
            else if (sStaged == sStatusUntracked)
            {
                bStaged = false;
                eStatus = CRepoFile::eUntracked;
            }
        }

        if (sUnstaged.isEmpty() == false)
        {
            if (sUnstaged == sStatusAdded)
            {
                bStaged = false;
                eStatus = CRepoFile::eAdded;
            }
            else if (sUnstaged == sStatusModified)
            {
                bStaged = false;
                eStatus = CRepoFile::eModified;
            }
            else if (sUnstaged == sStatusRenamed)
            {
                bStaged = false;
                eStatus = CRepoFile::eRenamed;
            }
            else if (sUnstaged == sStatusDeleted)
            {
                bStaged = false;
                eStatus = CRepoFile::eDeleted;
            }
            else if (sUnstaged == sStatusIgnored)
                eStatus = CRepoFile::eIgnored;
            else if (sUnstaged == sStatusUntracked)
            {
                bStaged = false;
                eStatus = CRepoFile::eUntracked;
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

void CGitCommands::onExecFinished(QString sPath, CProcessCommand::EProcessCommand eCommand, QString sValue)
{
    switch (eCommand)
    {

    case CProcessCommand::eNotification:
    case CProcessCommand::eCurrentBranch:
    case CProcessCommand::eRepositoryStatus:
        break;

    case CProcessCommand::eStageFile:
    case CProcessCommand::eStageAll:
    case CProcessCommand::eRevertFile:
    case CProcessCommand::eCommit:
    case CProcessCommand::eAmend:
    case CProcessCommand::ePush:
    case CProcessCommand::ePull:
    case CProcessCommand::eSetCurrentBranch:
    case CProcessCommand::eCommitReset:
    case CProcessCommand::eCommitRebase:
    case CProcessCommand::eChangeCommitMessage:
    case CProcessCommand::eContinueRebase:
    case CProcessCommand::eAbortRebase:
    {
        emit newOutputString(eCommand, sValue);
        break;
    }

    case CProcessCommand::eUnstagedFileDiff:
    {
        QList<CDiffLine*> lReturnValue;
        QStringList lLines = sValue.split("\n");
        bool bAtLeastOneLineNotEmpty = false;

        for (QString sLine : lLines)
        {
            QString sTrimmedLine = sLine.trimmed();

            if (not sTrimmedLine.isEmpty())
            {
                bAtLeastOneLineNotEmpty = true;

                CDiffLine* pDiffLine = new CDiffLine();
                pDiffLine->setText(sLine);

                if (sLine.startsWith("+"))
                    pDiffLine->setOperation(CDiffLine::Add);
                if (sLine.startsWith("-"))
                    pDiffLine->setOperation(CDiffLine::Delete);

                lReturnValue << pDiffLine;
            }
        }

        if (bAtLeastOneLineNotEmpty == false)
        {
            qDeleteAll(lReturnValue);
            lReturnValue.clear();
        }

        emit newOutputListOfCDiffLine(eCommand, lReturnValue);
        break;
    }

    case CProcessCommand::eBranches:
    {
        QStringList lLines = sValue.split("\n");
        QStringList lReturnValue;

        for (QString sLine : lLines)
        {
            sLine = sLine.split("->").first();

            if (sLine.startsWith("*"))
            {
                sLine.remove(0, 2);
                lReturnValue << sLine;

                emit newOutputString(CProcessCommand::eCurrentBranch, sLine);
            }
            else
            {
                sLine.remove(0, 2);
                lReturnValue << sLine;
            }
        }

        emit newOutputStringList(eCommand, lReturnValue);
        break;
    }

    case CProcessCommand::eAllFileStatus:
    {
        QList<CRepoFile*> vReturnValue;
        QStringList lStrings = sValue.split("\n");

        for (QString sLine : lStrings)
        {
            CRepoFile* pFile = repoFileForLine(sPath, sLine);

            if (pFile != nullptr)
                vReturnValue << pFile;
        }

        emit newOutputListOfCRepoFile(eCommand, vReturnValue);
        break;
    }

    case CProcessCommand::eFileLog:
    case CProcessCommand::eBranchLog:
    {
        QList<CLogLine*> lReturnValue;
        QStringList lStrings = sValue.split("\n");

        for (QString sLine : lStrings)
        {
            QStringList sValues = sLine.split(sLogFormatSplitter);

            if (sValues.count() == iLogFormatValueCount)
            {
                CLogLine* pLine = new CLogLine();

                pLine->setCommitId(sValues[0].trimmed());
                pLine->setMessage(sValues[1].trimmed());
                pLine->setAuthor(sValues[2].trimmed());
                pLine->setDate(QDateTime::fromString(sValues[3].trimmed(), Qt::ISODate));

                lReturnValue << pLine;
            }
        }

        emit newOutputListOfCLogLine(eCommand, lReturnValue);
        break;
    }

    }
}
