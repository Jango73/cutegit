
// Qt
#include <QDir>
#include <QTemporaryFile>

// Application
#include "CRepository.h"
#include "CUtils.h"
#include "CPatchBuilder.h"
#include "CDiffConstants.h"

//-------------------------------------------------------------------------------------------------

void CRepository::saveFileDiffAsPatch(const QString& sFullName)
{
    QString sFile = CUtils::localFilePath(sFullName);

    if (fileDiffFromCommit())
        CUtils::setTextFileContents(sFile, m_sFileDiffString);
    else
    {
        // Fixed staged-then-unstaged order with a guaranteed separator:
        // arrival order of the two asynchronous diffs must not matter
        QString sPatch(m_sStagedFileDiffString);

        if (not sPatch.isEmpty() && not m_sUnstagedFileDiffString.isEmpty() && not sPatch.endsWith(NEW_LINE))
            sPatch += NEW_LINE;

        sPatch += m_sUnstagedFileDiffString;

        if (not sPatch.isEmpty() && not sPatch.endsWith(NEW_LINE))
            sPatch += NEW_LINE;

        CUtils::setTextFileContents(sFile, sPatch);
    }

    qDebug() << "saveFileDiffAsPatch:" << sFile;
}

//-------------------------------------------------------------------------------------------------

void CRepository::patchApply(const QString& sFullName)
{
    QString sFile = CUtils::localFilePath(sFullName);

    m_pCommands->patchApply(m_sRepositoryPath, sFile);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::stageDiffLines
    Stages selected unstaged lines by applying a filtered patch on the index.
*/
void CRepository::stageDiffLines(QVariantList lProxyRows)
{
    applyDiffLines(m_pUnstagedFileDiffModel, m_pUnstagedFileDiffModelProxy, lProxyRows, true);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::unstageDiffLines
    Unstages selected staged lines by reverse-applying a filtered patch on the index.
*/
void CRepository::unstageDiffLines(QVariantList lProxyRows)
{
    applyDiffLines(m_pStagedFileDiffModel, m_pStagedFileDiffModelProxy, lProxyRows, false);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::stageDiffHunk
    Stages every change of a hunk in the unstaged diff.
*/
void CRepository::stageDiffHunk(int iHunkId)
{
    applyDiffHunk(m_pUnstagedFileDiffModel, m_pUnstagedFileDiffModelProxy, iHunkId, true);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::unstageDiffHunk
    Unstages every change of a hunk in the staged diff.
*/
void CRepository::unstageDiffHunk(int iHunkId)
{
    applyDiffHunk(m_pStagedFileDiffModel, m_pStagedFileDiffModelProxy, iHunkId, false);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::requestWorkingTreeDiffs
    Requests staged and unstaged diffs for a file.
*/
void CRepository::requestWorkingTreeDiffs(const QString& sFileFullName)
{
    // Drop stale views immediately: results arrive asynchronously and the
    // slots below ignore outputs tagged for a previously selected file
    m_pStagedFileDiffModel->setLines(QList<CDiffLine*>());
    m_pUnstagedFileDiffModel->setLines(QList<CDiffLine*>());
    setStagedFileDiffString("");
    setUnstagedFileDiffString("");

    m_pCommands->stagedFileDiff(m_sRepositoryPath, sFileFullName);
    m_pCommands->unstagedFileDiff(m_sRepositoryPath, sFileFullName);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::workingTreeDiffMatchesCurrentFile
    Checks that an asynchronous working tree diff result still belongs to the
    displayed file. Results without a file tag are accepted for compatibility.
*/
bool CRepository::workingTreeDiffMatchesCurrentFile(CDiffModel* pModel, const QString& sUserData) const
{
    if (pModel == nullptr)
        return false;

    if (sUserData.isEmpty())
        return true;

    return sUserData == pModel->fullSourceName();
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::applyDiffLines
    Builds a patch from selected rows and applies it on the index.
    When bStage is true the patch comes from the unstaged model and is
    applied forward, otherwise it comes from the staged model and is
    applied in reverse.
*/
void CRepository::applyDiffLines(CDiffModel* pModel, CDiffModelProxy* pProxy, const QVariantList& lProxyRows, bool bStage)
{
    if (pModel == nullptr || pProxy == nullptr || m_pCommands == nullptr)
        return;

    if (lProxyRows.isEmpty())
        return;

    applyDiffSourceRows(pModel, CPatchBuilder::sourceRowsFromProxyRows(pProxy, pModel, lProxyRows), bStage);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::applyDiffSourceRows
    Builds a patch from selected source rows and applies it on the index.
    When bStage is true the patch is applied forward, otherwise in reverse.
*/
void CRepository::applyDiffSourceRows(CDiffModel* pModel, const QSet<int>& sSelectedSourceRows, bool bStage)
{
    if (pModel == nullptr || m_pCommands == nullptr)
        return;

    if (sSelectedSourceRows.isEmpty())
        return;

    QString sFullName = pModel->fullSourceName();

    if (sFullName.isEmpty())
        return;

    QString sPatch = CPatchBuilder::patchFromSelection(pModel, sSelectedSourceRows, relativeFileName(sFullName));

    if (sPatch.isEmpty())
    {
        onNewOutput(bStage ? tr("No staged change in selection") : tr("No unstaged change in selection"), false);
        return;
    }

    QTemporaryFile tPatchFile(CDiffConstants::sStagePatchFileTemplate.arg(QDir::tempPath()));

    if (not tPatchFile.open())
    {
        onNewOutput(tr("Unable to create patch file"), false);
        return;
    }

    tPatchFile.write(sPatch.toUtf8());
    tPatchFile.flush();

    QString sOutput = m_pCommands->applyPatchNow(m_sRepositoryPath, tPatchFile.fileName(), true, not bStage);

    if (sOutput.isEmpty())
    {
        checkChangedFileStatus();
    }
    else
    {
        onNewOutput(sOutput, false);
    }
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::applyDiffHunk
    Builds a patch from every change of a hunk and applies it on the index.
    Only rows visible through the proxy are kept, so hunk staging follows
    the same text filter as line staging. When bStage is true the patch is
    applied forward, otherwise in reverse.
*/
void CRepository::applyDiffHunk(CDiffModel* pModel, CDiffModelProxy* pProxy, int iHunkId, bool bStage)
{
    if (pModel == nullptr || pProxy == nullptr)
        return;

    if (iHunkId == CDiffConstants::iInvalidHunkId)
        return;

    QSet<int> sSelectedSourceRows;

    for (int iSourceRow : CPatchBuilder::selectableSourceRowsForHunk(pModel, iHunkId))
    {
        QModelIndex tSourceIndex = pModel->index(iSourceRow, CDiffConstants::iFirstColumn);

        if (not pProxy->mapFromSource(tSourceIndex).isValid())
            continue;

        sSelectedSourceRows.insert(iSourceRow);
    }

    if (sSelectedSourceRows.isEmpty())
        return;

    applyDiffSourceRows(pModel, sSelectedSourceRows, bStage);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::workingTreeDiffModel
    Returns the staged or unstaged diff model for a working tree diff command.
    Returns nullptr for any other command.
*/
CDiffModel* CRepository::workingTreeDiffModel(CEnums::EProcessCommand eCommand) const
{
    switch (eCommand)
    {
    case CEnums::eStagedFileDiff:
        return m_pStagedFileDiffModel;

    case CEnums::eUnstagedFileDiff:
        return m_pUnstagedFileDiffModel;

    default:
        return nullptr;
    }
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::setWorkingTreeDiffString
    Stores the raw output of a staged or unstaged diff, dropping late results
    requested for a commit diff and stale results tagged for another file.
*/
void CRepository::setWorkingTreeDiffString(CEnums::EProcessCommand eCommand, const QString& sOutput, const QString& sUserData)
{
    CDiffModel* pModel = workingTreeDiffModel(eCommand);

    if (pModel == nullptr)
        return;

    if (fileDiffFromCommit())
        return;

    if (not workingTreeDiffMatchesCurrentFile(pModel, sUserData))
        return;

    if (eCommand == CEnums::eStagedFileDiff)
        setStagedFileDiffString(sOutput);
    else
        setUnstagedFileDiffString(sOutput);
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CRepository::setWorkingTreeDiffLines
    Stores parsed lines of a staged or unstaged diff, dropping late results
    requested for a commit diff and stale results tagged for another file.
    Takes ownership of lNewLines in all cases.
*/
void CRepository::setWorkingTreeDiffLines(CEnums::EProcessCommand eCommand, QList<CDiffLine*> lNewLines, const QString& sUserData)
{
    CDiffModel* pModel = workingTreeDiffModel(eCommand);

    if (pModel == nullptr || fileDiffFromCommit() || not workingTreeDiffMatchesCurrentFile(pModel, sUserData))
    {
        qDeleteAll(lNewLines);
        return;
    }

    pModel->setLines(lNewLines);
}
