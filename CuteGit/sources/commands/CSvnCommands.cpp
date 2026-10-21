
// Qt
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

// qt-plus
#include "CXMLNode.h"

// Application
#include "CSvnCommands.h"
#include "../CUtils.h"

/*!
    \class CSvnCommands
    \inmodule CuteGit
    \section1 General
    This is the repository control interface for Apache Subversion (SVN).

         .-"""-.__     ltb
        /      ' o'\
     ,-;  '.  :   _c
    :_."\._ ) ::-"
           ""m "m*/

//-------------------------------------------------------------------------------------------------
// Constants

const QString CSvnCommands::sCommandAdd                = "svn add \"%1\"";
const QString CSvnCommands::sCommandAddForce           = "svn add --force \"%1\"";
const QString CSvnCommands::sCommandBlame              = "svn blame \"%1\"";
const QString CSvnCommands::sCommandBranchLog          = "svn log --limit %1 --xml";
const QString CSvnCommands::sCommandCheckout           = "svn checkout \"%1\"";
const QString CSvnCommands::sCommandCommit             = "svn commit -m \"%1\"";
const QString CSvnCommands::sCommandCopyBranch         = "svn copy -r %1 \"^/trunk\" \"^/branches/%2\" -m \"%3\"";
const QString CSvnCommands::sCommandCopyTag            = "svn copy -r %1 \"^/trunk\" \"^/tags/%2\" -m \"%3\"";
const QString CSvnCommands::sCommandDelete             = "svn delete \"%1\"";
const QString CSvnCommands::sCommandDiffFile           = "svn diff \"%1\"";
const QString CSvnCommands::sCommandDiffRevision       = "svn diff -c %1";
const QString CSvnCommands::sCommandDiffTwoRevisions   = "svn diff -r %1:%2";
const QString CSvnCommands::sCommandFileLog            = "svn log --limit %1 --xml \"%2\"";
const QString CSvnCommands::sCommandGraphLog           = "svn log --limit %1 --xml";
const QString CSvnCommands::sCommandInfo               = "svn info";
const QString CSvnCommands::sCommandInfoForFile         = "svn info \"%1\"";
const QString CSvnCommands::sCommandListBranches       = "svn list \"^/branches\"";
const QString CSvnCommands::sCommandRevert             = "svn revert \"%1\"";
const QString CSvnCommands::sCommandStatus             = "svn status";
const QString CSvnCommands::sCommandStatusForFile      = "svn status \"%1\"";
const QString CSvnCommands::sCommandStatusNoIgnore     = "svn status --no-ignore";
const QString CSvnCommands::sCommandTrackedFiles       = "svn info -R --xml";
const QString CSvnCommands::sCommandSwitchBranch       = "svn switch \"^/branches/%1\"";
const QString CSvnCommands::sCommandSwitchTrunk        = "svn switch \"^/trunk\"";
const QString CSvnCommands::sCommandUpdate             = "svn update";

const QString CSvnCommands::sBlameLineRegExp           = "^\\s*(\\d+)\\s+(\\S+)\\s?(.*)$";

const QString CSvnCommands::sTrunkName                 = "trunk";

const QString CSvnCommands::sStatusAdded               = "A";
const QString CSvnCommands::sStatusConflicted          = "C";
const QString CSvnCommands::sStatusDeleted             = "D";
const QString CSvnCommands::sStatusIgnored             = "I";
const QString CSvnCommands::sStatusExternal            = "X";
const QString CSvnCommands::sStatusModified            = "M";
const QString CSvnCommands::sStatusMissing             = "!";
const QString CSvnCommands::sStatusObstructed          = "~";
const QString CSvnCommands::sStatusReplaced            = "R";
const QString CSvnCommands::sStatusUntracked           = "?";

//-------------------------------------------------------------------------------------------------

/*! Parses a Subversion XML log date, dropping fractional seconds when Qt cannot parse them */
static QDateTime parseSvnDate(const QString& sValue)
{
    QString sText = sValue.trimmed();
    QDateTime tDate = QDateTime::fromString(sText, Qt::ISODate);

    if (not tDate.isValid())
    {
        int iDot = sText.indexOf('.');

        if (iDot != -1)
        {
            QString sHead = sText.left(iDot);
            QString sTail = sText.mid(iDot + 1);
            int iZone = sTail.indexOf(QRegularExpression("[Z+-]"));
            QString sZone = (iZone != -1) ? sTail.mid(iZone) : "";
            tDate = QDateTime::fromString(sHead + sZone, Qt::ISODate);
        }
    }

    return tDate;
}

//-------------------------------------------------------------------------------------------------

/*! Extracts the current branch name from the output of 'svn info' */
static QString currentBranchFromInfo(const QString& sInfo)
{
    QString sRelativeURL;

    for (QString sLine : sInfo.split(NEW_LINE))
    {
        if (sLine.startsWith("Relative URL:"))
        {
            sRelativeURL = sLine.mid(QString("Relative URL:").length()).trimmed();
            break;
        }
    }

    if (not sRelativeURL.isEmpty())
    {
        if (sRelativeURL.endsWith("/trunk") || sRelativeURL == "^/trunk")
            return "trunk";

        int iBranches = sRelativeURL.indexOf("/branches/");

        if (iBranches != -1)
            return sRelativeURL.mid(iBranches + QString("/branches/").length()).split('/').first();

        int iTags = sRelativeURL.indexOf("/tags/");

        if (iTags != -1)
            return sRelativeURL.mid(iTags + QString("/tags/").length()).split('/').first();

        return sRelativeURL.split('/').last();
    }

    for (QString sLine : sInfo.split(NEW_LINE))
    {
        if (sLine.startsWith("URL:"))
        {
            QString sURL = sLine.mid(QString("URL:").length()).trimmed();

            if (sURL.endsWith("/trunk"))
                return "trunk";

            int iBranches = sURL.indexOf("/branches/");

            if (iBranches != -1)
                return sURL.mid(iBranches + QString("/branches/").length()).split('/').first();
        }
    }

    return "trunk";
}

//-------------------------------------------------------------------------------------------------

CSvnCommands::CSvnCommands(CController* pController)
    : CCommands(pController)
{
    connect(this, &CCommands::execFinished, this, &CSvnCommands::onExecFinished);
}

//-------------------------------------------------------------------------------------------------

CSvnCommands::~CSvnCommands()
{
}

//-------------------------------------------------------------------------------------------------

bool CSvnCommands::can(CEnums::ECapability eWhat) const
{
    switch (eWhat)
    {
    case CEnums::Clone:
    case CEnums::ShowGlobalGraph:
    case CEnums::ShowBranchLog:
    case CEnums::ShowFileLog:
    case CEnums::Commit:
    case CEnums::Pull:
    case CEnums::BranchFromCommit:
    case CEnums::TagOnCommit:
    case CEnums::Blame:
    case CEnums::Stage:
    case CEnums::SwitchBranch:
        return true;

    default:
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::cloneRepository(const QString& sRepositoryURL, const QString& sRepositoryPath)
{
    QString sCommand = QString(sCommandCheckout).arg(sRepositoryURL);
    exec(new CProcessCommand(
             CEnums::eCloneRepository,
             sRepositoryPath,
             sCommand,
             false,
             QMap<QString, QString>(),
             "",
             CEnums::eCloneRepositoryFinished)
         );
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::repositoryStatus(const QString& sPath)
{
    QString sStatus = execNow(sPath, sCommandStatus);
    bool bConflicted = false;

    for (QString sLine : sStatus.split(NEW_LINE))
    {
        if (not sLine.isEmpty() && QString(sLine[0]) == sStatusConflicted)
        {
            bConflicted = true;
            break;
        }
    }

    emit newOutputString(
                CEnums::eRepositoryStatus,
                bConflicted ? CRepoFile::sRepositoryStatusMerge : CRepoFile::sRepositoryStatusClean
                );
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::allFileStatus(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eAllFileStatus, sPath, sCommandTrackedFiles));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::changedFileStatus(const QString& sPath)
{
    // Ignored files stay out of the default refresh, they are fetched on
    // demand through ignoredFileStatus like the other backends
    exec(new CProcessCommand(CEnums::eChangedFileStatus, sPath, sCommandStatus));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::ignoredFileStatus(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eIgnoredFileStatus, sPath, sCommandStatusNoIgnore));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::pollFingerprint(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::ePollFingerprint, sPath, sCommandStatus));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::branches(const QString& sPath)
{
    exec(new CProcessCommand(CEnums::eBranches, sPath, sCommandListBranches));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::graph(const QString& sPath, int iFrom, int iCount)
{
    int iPotentialCount = 0;
    QString sCommand = QString(sCommandGraphLog).arg(iFrom + iCount);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eGraph, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::branchLog(const QString& sPath, int iFrom, int iCount)
{
    int iPotentialCount = 0;
    QString sCommand = QString(sCommandBranchLog).arg(iFrom + iCount);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eBranchLog, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::fileLog(const QString& sPath, const QString& sFullName, int iFrom, int iCount)
{
    int iPotentialCount = 0;
    QString sCommand = QString(sCommandFileLog).arg(iFrom + iCount).arg(sFullName);
    QString sUserData = QString("%1,%2").arg(iPotentialCount).arg(iFrom);
    exec(new CProcessCommand(CEnums::eFileLog, sPath, sCommand, false, QMap<QString, QString>(), sUserData));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::deleteFile(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandDelete).arg(sFullName);
    exec(new CProcessCommand(CEnums::eDeleteFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::toggleStaged(const QString& sPath, const QString& sFullName)
{
    QString sFileStatusCommand = QString(sCommandStatusForFile).arg(sFullName);
    QString sLine = execNow(sPath, sFileStatusCommand);

    CRepoFile* pFile = repoFileForLine(sPath, sLine);

    if (pFile != nullptr)
    {
        // Subversion has no staging area: only scheduling (add) and unscheduling (revert of an add) apply
        if (pFile->status() == CEnums::eUntracked)
            stageFile(sPath, sFullName, true);
        else if (pFile->status() == CEnums::eAdded)
            stageFile(sPath, sFullName, false);

        delete pFile;
    }
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::stageFile(const QString& sPath, const QString& sFullName, bool bStage)
{
    QString sFileStatusCommand = QString(sCommandStatusForFile).arg(sFullName);
    QString sLine = execNow(sPath, sFileStatusCommand);

    CRepoFile* pFile = repoFileForLine(sPath, sLine);
    CEnums::ERepoFileStatus eStatus = (pFile != nullptr) ? pFile->status() : CEnums::eClean;

    if (pFile != nullptr)
        delete pFile;

    // Reverting anything but a scheduled add would destroy user changes, so only adds are unscheduled
    if (bStage && eStatus == CEnums::eUntracked)
    {
        QString sCommand = QString(sCommandAdd).arg(sFullName);
        exec(new CProcessCommand(CEnums::eStageFile, sPath, sCommand, true));
    }
    else if (not bStage && eStatus == CEnums::eAdded)
    {
        QString sCommand = QString(sCommandRevert).arg(sFullName);
        exec(new CProcessCommand(CEnums::eStageFile, sPath, sCommand, true));
    }
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::stageAll(const QString& sPath, bool bStage)
{
    if (bStage)
    {
        emit newOutputString(CEnums::eNotification, tr("Scheduling all new files..."));
        QString sCommand = QString(sCommandAddForce).arg(sPath);
        exec(new CProcessCommand(CEnums::eStageAll, sPath, sCommand, true));
    }
    else
    {
        emit newOutputString(CEnums::eNotification, tr("Subversion has no staging area, nothing to unschedule"));
    }
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::revertFile(const QString& sPath, const QString& sFullName)
{
    emit newOutputString(CEnums::eNotification, tr("Reverting..."));
    QString sCommand = QString(sCommandRevert).arg(sFullName);
    exec(new CProcessCommand(CEnums::eRevertFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::undeleteFile(const QString& sPath, const QString& sFullName)
{
    emit newOutputString(CEnums::eNotification, tr("Undeleting..."));
    QString sCommand = QString(sCommandRevert).arg(sFullName);
    exec(new CProcessCommand(CEnums::eUndeleteFile, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::commit(const QString& sPath, const QString& sMessage)
{
    emit newOutputString(CEnums::eNotification, tr("Commiting..."));
    QString sCommand = QString(sCommandCommit).arg(sMessage);
    exec(new CProcessCommand(CEnums::eCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::pull(const QString& sPath)
{
    emit newOutputString(CEnums::eNotification, tr("Updating..."));
    QString sCommand = QString(sCommandUpdate);
    exec(new CProcessCommand(CEnums::ePull, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::unstagedFileDiff(const QString& sPath, const QString& sFullName)
{
    // Unversioned files are invisible to svn diff: synthesize an svn-style
    // new-file diff from the working tree content so it shows up as added
    // lines. The synthetic text reuses handleDiffOutput, so display stays
    // consistent with tracked files
    int iExitCode = 0;

    execNow(sPath, QString(sCommandInfoForFile).arg(sFullName), QMap<QString, QString>(), &iExitCode);

    if (iExitCode == 0)
    {
        QString sCommand = QString(sCommandDiffFile).arg(sFullName);
        exec(new CProcessCommand(CEnums::eUnstagedFileDiff, sPath, sCommand));
        return;
    }

    QString sRelativeName = QDir(sPath).relativeFilePath(sFullName);
    QString sContent = CUtils::getTextFileContents(sFullName);
    QStringList lFileLines;

    if (not sContent.isEmpty())
    {
        lFileLines = sContent.split(NEW_LINE);

        // A trailing newline is a line terminator, not an extra empty line
        if (not lFileLines.isEmpty() && lFileLines.last().isEmpty() && sContent.endsWith(NEW_LINE))
            lFileLines.removeLast();
    }

    QStringList lDiff;
    lDiff << QString("Index: %1").arg(sRelativeName);
    lDiff << QString("--- %1\t(revision 0)").arg(sRelativeName);
    lDiff << QString("+++ %1\t(working copy)").arg(sRelativeName);
    lDiff << QString("@@ -0,0 +1,%1 @@").arg(lFileLines.count());

    for (QString sLine : lFileLines)
        lDiff << ("+" + sLine);

    handleDiffOutput(CProcessResult(sPath, CEnums::eUnstagedFileDiff, lDiff.join(NEW_LINE), ""));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::blame(const QString& sPath, const QString& sFullName)
{
    QString sCommand = QString(sCommandBlame).arg(sFullName);
    exec(new CProcessCommand(CEnums::eBlame, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::commitDiffPrevious(const QString& sPath, const QString& sCommitId)
{
    QString sCommand = QString(sCommandDiffRevision).arg(sCommitId);
    exec(new CProcessCommand(CEnums::eTwoCommitDiff, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::twoCommitDiff(const QString& sPath, const QString& sFromCommitId, const QString& sToCommitId)
{
    QString sCommand = QString(sCommandDiffTwoRevisions).arg(sFromCommitId).arg(sToCommitId);
    exec(new CProcessCommand(CEnums::eTwoCommitDiff, sPath, sCommand));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::setCurrentBranch(const QString& sPath, const QString& sBranch)
{
    QString sCommand = (sBranch == sTrunkName)
            ? QString(sCommandSwitchTrunk)
            : QString(sCommandSwitchBranch).arg(sBranch);
    emit newOutputString(CEnums::eNotification, QString(tr("Switching to %1...")).arg(sBranch));
    exec(new CProcessCommand(CEnums::eSetCurrentBranch, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::createBranchOnCommit(const QString& sPath, const QString& sCommitId, const QString& sBranchName)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Creating branch %1...")).arg(sBranchName));
    QString sMessage = QString(tr("Create branch %1")).arg(sBranchName);
    QString sCommand = QString(sCommandCopyBranch).arg(sCommitId).arg(sBranchName).arg(sMessage);
    exec(new CProcessCommand(CEnums::eCreateBranchOnCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::createTagOnCommit(const QString& sPath, const QString& sCommitId, const QString& sTagName, const QString& sMessage)
{
    emit newOutputString(CEnums::eNotification, QString(tr("Creating tag %1...")).arg(sTagName));
    QString sFinalMessage = sMessage.isEmpty() ? QString(tr("Create tag %1")).arg(sTagName) : sMessage;
    QString sCommand = QString(sCommandCopyTag).arg(sCommitId).arg(sTagName).arg(sFinalMessage);
    exec(new CProcessCommand(CEnums::eCreateTagOnCommit, sPath, sCommand, true));
}

//-------------------------------------------------------------------------------------------------

CRepoFile* CSvnCommands::repoFileForLine(const QString& sPath, QString sLine)
{
    if (sLine.length() < 8)
        return nullptr;

    QString sFlags = sLine.left(7);
    QString sRelativeName = sLine.mid(7).trimmed();

    if (sRelativeName.isEmpty())
        return nullptr;

    if (sRelativeName.startsWith("Performing status"))
        return nullptr;

    QString sItemStatus = QString(sFlags[0]);
    QString sPropertiesStatus = QString(sFlags[1]);

    CEnums::ERepoFileStatus eStatus = CEnums::eClean;
    bool bStaged = false;
    bool bKnown = true;

    if (sItemStatus == sStatusAdded)
    {
        eStatus = CEnums::eAdded;
        bStaged = true;
    }
    else if (sItemStatus == sStatusModified || sItemStatus == sStatusReplaced || sItemStatus == sStatusObstructed)
    {
        eStatus = CEnums::eModified;
        bStaged = true;
    }
    else if (sItemStatus == sStatusConflicted)
    {
        eStatus = CEnums::eModified;
        bStaged = true;
    }
    else if (sItemStatus == sStatusDeleted)
    {
        eStatus = CEnums::eDeleted;
        bStaged = true;
    }
    else if (sItemStatus == sStatusMissing)
    {
        eStatus = CEnums::eMissing;
        bStaged = true;
    }
    else if (sItemStatus == sStatusUntracked)
    {
        eStatus = CEnums::eUntracked;
        bStaged = false;
    }
    else if (sItemStatus == sStatusIgnored || sItemStatus == sStatusExternal)
    {
        eStatus = CEnums::eIgnored;
        bStaged = false;
    }
    else if (sItemStatus.trimmed().isEmpty())
    {
        if (sPropertiesStatus == sStatusModified || sPropertiesStatus == sStatusConflicted)
        {
            eStatus = CEnums::eModified;
            bStaged = true;
        }
        else
        {
            bKnown = false;
        }
    }
    else
    {
        bKnown = false;
    }

    if (not bKnown)
        return nullptr;

    QString sFullName = sPath + PATH_SEP + sRelativeName;
    QString sFileName = QFileInfo(sFullName).fileName();

    CRepoFile* pFile = new CRepoFile();
    pFile->setFullName(sFullName);
    pFile->setFileName(sFileName);
    pFile->setRelativeName(sRelativeName);
    pFile->setStatus(eStatus);
    pFile->setStaged(bStaged);

    return pFile;
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::handleBranchOutput(const CProcessResult& tResult)
{
    QList<CBranch*> lReturnValue;

    CBranch* pTrunk = new CBranch();
    pTrunk->setType(CEnums::LocalBranchLabel);
    pTrunk->setName(sTrunkName);
    lReturnValue << pTrunk;

    for (QString sLine : tResult.m_sValue.split(NEW_LINE))
    {
        sLine = sLine.trimmed();

        if (sLine.endsWith('/'))
            sLine.chop(1);

        if (not sLine.isEmpty())
        {
            CBranch* pNewBranch = new CBranch();
            pNewBranch->setType(CEnums::LocalBranchLabel);
            pNewBranch->setName(sLine);
            lReturnValue << pNewBranch;
        }
    }

    QString sCurrentBranch = currentBranchFromInfo(execNow(tResult.m_sPath, sCommandInfo));
    emit newOutputString(CEnums::eCurrentBranch, sCurrentBranch);

    emit newOutputListOfCBranch(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::handleFileStatusOutput(const CProcessResult& tResult)
{
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

void CSvnCommands::handleTrackedFilesOutput(const CProcessResult& tResult)
{
    // The recursive info carries one entry per versioned node: file kind
    // entries seed the inventory as clean, dirtiness arrives through the
    // status batches. XML stays locale independent, unlike human output
    QStringList lRelativeNames;
    CXMLNode xInfo = CXMLNode::parseXML(tResult.m_sValue);

    for (CXMLNode xEntry : xInfo.getNodesByTagName("entry"))
    {
        if (xEntry.attributes().value("kind").trimmed() == "file")
            lRelativeNames << xEntry.attributes().value("path").trimmed();
    }

    CRepoFileList lReturnValue = cleanFilesForRelativePaths(tResult.m_sPath, lRelativeNames);

    emit newOutputListOfCRepoFile(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::handleLogOutput(const CProcessResult& tResult)
{
    CLogLineCollection lReturnValue;

    CXMLNode xLog = CXMLNode::parseXML(tResult.m_sValue);
    CXMLNodeList xEntries = xLog.getNodesByTagName("logentry");

    QStringList lUserValues = tResult.m_sUserData.split(",");
    int iPotentialCount = (lUserValues.count() > 0) ? lUserValues[0].toInt() : 0;
    int iStartIndex = (lUserValues.count() > 1) ? lUserValues[1].toInt() : 0;

    int iSkipped = 0;

    for (CXMLNode xEntry : xEntries)
    {
        if (iSkipped < iStartIndex)
        {
            iSkipped++;
            continue;
        }

        CLogLine* pLine = new CLogLine();

        pLine->setCommitId(xEntry.attributes().value("revision").trimmed());
        pLine->setAuthor(xEntry.getNodeByTagName("author").value().trimmed());
        pLine->setDate(parseSvnDate(xEntry.getNodeByTagName("date").value()));
        pLine->setMessage(xEntry.getNodeByTagName("msg").value().trimmed());
        pLine->setMessageIsComplete(true);

        lReturnValue.add(pLine);
    }

    if (iPotentialCount == 0)
        iPotentialCount = lReturnValue.lines().count();

    lReturnValue.setPotentialCount(iPotentialCount);
    lReturnValue.setStartIndex(iStartIndex);

    emit newOutputCLogLineCollection(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::handleGraphOutput(const CProcessResult& tResult)
{
    CGraphLineCollection lReturnValue;

    CXMLNode xLog = CXMLNode::parseXML(tResult.m_sValue);
    CXMLNodeList xEntries = xLog.getNodesByTagName("logentry");

    QStringList lUserValues = tResult.m_sUserData.split(",");
    int iPotentialCount = (lUserValues.count() > 0) ? lUserValues[0].toInt() : 0;
    int iStartIndex = (lUserValues.count() > 1) ? lUserValues[1].toInt() : 0;

    QStringList lRevisions;

    for (CXMLNode xEntry : xEntries)
    {
        QString sRevision = xEntry.attributes().value("revision").trimmed();

        if (not sRevision.isEmpty())
            lRevisions << sRevision;
    }

    int iEntryIndex = 0;

    for (CXMLNode xEntry : xEntries)
    {
        QString sRevision = xEntry.attributes().value("revision").trimmed();

        if (sRevision.isEmpty())
            continue;

        if (iEntryIndex < iStartIndex)
        {
            iEntryIndex++;
            continue;
        }

        CGraphLine* pLine = new CGraphLine();

        pLine->setCommitId(sRevision);

        // Subversion history is linear: the parent of a revision is the next older one
        if (iEntryIndex + 1 < lRevisions.count())
            pLine->setParents(QStringList() << lRevisions[iEntryIndex + 1]);

        pLine->setMessage(xEntry.getNodeByTagName("msg").value().trimmed());
        pLine->setMessageIsComplete(true);
        pLine->setAuthor(xEntry.getNodeByTagName("author").value().trimmed());
        pLine->setDate(parseSvnDate(xEntry.getNodeByTagName("date").value()));

        lReturnValue.add(pLine);
        iEntryIndex++;
    }

    if (iPotentialCount == 0)
        iPotentialCount = lReturnValue.lines().count();

    lReturnValue.setPotentialCount(iPotentialCount);
    lReturnValue.setStartIndex(iStartIndex);

    emit newOutputCGraphLineCollection(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::handleDiffOutput(const CProcessResult& tResult)
{
    QList<CDiffLine*> lReturnValue;
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);
    bool bAtLeastOneLineOK = false;

    for (QString sLine : lLines)
    {
        QString sTrimmedLine = sLine.trimmed();

        if (not sTrimmedLine.isEmpty())
        {
            bAtLeastOneLineOK = true;

            CDiffLine* pDiffLine = new CDiffLine();
            pDiffLine->setText(sLine);

            if (sLine.startsWith("Index: "))
            {
                pDiffLine->setOperation(CEnums::DiffFileName);
                pDiffLine->setText(sLine.mid(QString("Index: ").length()).trimmed());
                lReturnValue << pDiffLine;
            }
            else if (sLine.startsWith("index") || sLine.startsWith("+++") || sLine.startsWith("---"))
            {
                delete pDiffLine;
            }
            else
            {
                if (sLine.startsWith("diff"))
                {
                    pDiffLine->setOperation(CEnums::DiffFileName);
                }
                else if (not (sLine.startsWith("@@")))
                {
                    if (sLine.startsWith("+"))
                        pDiffLine->setOperation(CEnums::DiffAdd);

                    if (sLine.startsWith("-"))
                        pDiffLine->setOperation(CEnums::DiffDelete);
                }

                lReturnValue << pDiffLine;
            }
        }
    }

    if (not bAtLeastOneLineOK)
    {
        qDeleteAll(lReturnValue);
        lReturnValue.clear();
    }

    emit newOutputString(tResult.m_eCommand, tResult.m_sValue);
    emit newOutputListOfCDiffLine(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::handleBlameOutput(const CProcessResult& tResult)
{
    QList<CDiffLine*> lReturnValue;
    QStringList lLines = tResult.m_sValue.split(NEW_LINE);

    QRegularExpression tRegExp(sBlameLineRegExp);
    int iLineNumber = 0;

    for (QString sLine : lLines)
    {
        iLineNumber++;

        QRegularExpressionMatch match = tRegExp.match(sLine);

        if (match.hasMatch())
        {
            QString sRevision = match.captured(1).trimmed();
            QString sAuthor = match.captured(2).trimmed();
            QString sText = match.captured(3);

            QString sFinalText = QString("%1: %2 %3 | %4")
                    .arg(iLineNumber, 5, 10, QChar('0'))
                    .arg(sAuthor, -20)
                    .arg(sRevision)
                    .arg(sText);

            CDiffLine* pDiffLine = new CDiffLine();
            pDiffLine->setText(sFinalText);

            lReturnValue << pDiffLine;
        }
    }

    emit newOutputListOfCDiffLine(tResult.m_eCommand, lReturnValue);
}

//-------------------------------------------------------------------------------------------------

void CSvnCommands::onExecFinished(const CProcessResult& tResult)
{
    switch (tResult.m_eCommand)
    {

    case CEnums::eNothing:
    case CEnums::eNotification:
    case CEnums::eCurrentBranch:
    case CEnums::eRepositoryStatus:
        break;

    case CEnums::eIssuedCommand:
    case CEnums::eCloneRepository:
    case CEnums::eCloneRepositoryFinished:
    case CEnums::ePollFingerprint:
    case CEnums::eDeleteFile:
    case CEnums::eUndeleteFile:
    case CEnums::eStageFile:
    case CEnums::eStageAll:
    case CEnums::eRevertFile:
    case CEnums::eCommit:
    case CEnums::ePull:
    case CEnums::eSetCurrentBranch:
    case CEnums::eCreateBranchOnCommit:
    case CEnums::eCreateTagOnCommit:
    {
        // Throw the returned string of the process
        emit newOutputString(tResult.m_eCommand, tResult.m_sValue, QString(), tResult.m_iExitCode);
        break;
    }

    case CEnums::eUnstagedFileDiff:
    case CEnums::eTwoCommitDiff:
    {
        handleDiffOutput(tResult);
        break;
    }

    case CEnums::eBlame:
    {
        handleBlameOutput(tResult);
        break;
    }

    case CEnums::eBranches:
    {
        handleBranchOutput(tResult);
        break;
    }

    case CEnums::eAllFileStatus:
    {
        handleTrackedFilesOutput(tResult);
        break;
    }

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

    case CEnums::eGraph:
    {
        handleGraphOutput(tResult);
        break;
    }

    default:
    {
        break;
    }

    }
}

//-------------------------------------------------------------------------------------------------
