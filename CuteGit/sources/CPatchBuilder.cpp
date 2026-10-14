
// Qt
#include <QModelIndex>
#include <QRegularExpression>

// Application
#include "CPatchBuilder.h"
#include "CDiffLine.h"

//-------------------------------------------------------------------------------------------------

//! Hunk header pattern, mirrors CGitCommands::sDiffHunkRegExp
#define PATCH_HUNK_REGEXP "@@\\s+-([0-9]+)(,([0-9]+))?\\s+\\+([0-9]+)(,([0-9]+))?\\s+@@(.*)"

//-------------------------------------------------------------------------------------------------

/*!
    \fn CPatchBuilder::sourceRowsFromProxyRows
    Maps proxy rows coming from QML to source rows of a diff model.
*/
QSet<int> CPatchBuilder::sourceRowsFromProxyRows(CDiffModelProxy* pProxy, CDiffModel* pModel, const QVariantList& lProxyRows)
{
    QSet<int> sReturnValue;

    if (pProxy == nullptr || pModel == nullptr)
        return sReturnValue;

    for (QVariant vRow : lProxyRows)
    {
        bool bConverted = false;
        int iProxyRow = vRow.toInt(&bConverted);

        if (not bConverted)
            continue;

        QModelIndex tProxyIndex = pProxy->index(iProxyRow, 0);
        if (not tProxyIndex.isValid())
            continue;

        QModelIndex tSourceIndex = pProxy->mapToSource(tProxyIndex);
        if (not tSourceIndex.isValid())
            continue;

        sReturnValue.insert(tSourceIndex.row());
    }

    return sReturnValue;
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CPatchBuilder::selectableSourceRowsForHunk
    Returns the source rows holding added or deleted lines for a hunk.
*/
QList<int> CPatchBuilder::selectableSourceRowsForHunk(CDiffModel* pModel, int iHunkId)
{
    QList<int> lReturnValue;

    if (pModel == nullptr || iHunkId < 0)
        return lReturnValue;

    QList<CDiffLine*> lLines = pModel->lines();

    for (int iRow = 0; iRow < lLines.count(); iRow++)
    {
        CDiffLine* pLine = lLines[iRow];

        if (pLine != nullptr && pLine->hunkId() == iHunkId)
        {
            if (pLine->operation() == CEnums::DiffAdd || pLine->operation() == CEnums::DiffDelete)
                lReturnValue << iRow;
        }
    }

    return lReturnValue;
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CPatchBuilder::patchFromSelection
    Builds a unified patch for selected added or deleted source rows.
    Unselected added lines are dropped, unselected deleted lines are kept as
    context so the patch stays applicable with git apply --cached.
    Returns an empty string when no hunk holds a selected change.
*/
QString CPatchBuilder::patchFromSelection(CDiffModel* pModel, const QSet<int>& sSelectedSourceRows, const QString& sRelativeName)
{
    if (pModel == nullptr || sRelativeName.isEmpty() || sSelectedSourceRows.isEmpty())
        return QString();

    QList<CDiffLine*> lLines = pModel->lines();
    QRegularExpression tHunkRegExp(QString(PATCH_HUNK_REGEXP));

    // Group source rows by hunk, keeping model order
    QMap<int, QList<int>> mHunkRows;
    QMap<int, QString> mHunkHeaders;

    for (int iRow = 0; iRow < lLines.count(); iRow++)
    {
        CDiffLine* pLine = lLines[iRow];

        if (pLine == nullptr)
            continue;

        if (pLine->operation() == CEnums::DiffFileName)
            continue;

        int iHunkId = pLine->hunkId();

        if (iHunkId < 0)
            continue;

        if (pLine->operation() == CEnums::DiffHunk)
        {
            mHunkHeaders[iHunkId] = pLine->text();
            continue;
        }

        if (not mHunkRows.contains(iHunkId))
            mHunkRows[iHunkId] = QList<int>();

        mHunkRows[iHunkId] << iRow;
    }

    QStringList lPatchHunks;
    bool bAllOldStartZero = true;
    bool bAllNewStartZero = true;
    bool bAtLeastOneHunk = false;

    for (int iHunkId : mHunkRows.keys())
    {
        QList<int> lHunkRows = mHunkRows[iHunkId];

        bool bHunkHasSelection = false;

        for (int iRow : lHunkRows)
        {
            if (not sSelectedSourceRows.contains(iRow))
                continue;

            CEnums::EDiffOperation eOperation = lLines[iRow]->operation();

            if (eOperation == CEnums::DiffAdd || eOperation == CEnums::DiffDelete)
            {
                bHunkHasSelection = true;
                break;
            }
        }

        if (not bHunkHasSelection)
            continue;

        QStringList lKeptLines;
        int iOldCount = 0;
        int iNewCount = 0;
        bool bHunkHasChange = false;

        for (int iRow : lHunkRows)
        {
            CDiffLine* pLine = lLines[iRow];
            QString sText = pLine->text();

            switch (pLine->operation())
            {
            case CEnums::DiffAdd:
                if (sSelectedSourceRows.contains(iRow))
                {
                    lKeptLines << sText;
                    iNewCount++;
                    bHunkHasChange = true;
                }
                break;

            case CEnums::DiffDelete:
                if (sSelectedSourceRows.contains(iRow))
                {
                    lKeptLines << sText;
                    iOldCount++;
                    bHunkHasChange = true;
                }
                else
                {
                    lKeptLines << (QString(" ") + sText.mid(1));
                    iOldCount++;
                    iNewCount++;
                }
                break;

            default:
                lKeptLines << sText;
                if (sText.startsWith("\\"))
                    break;
                iOldCount++;
                iNewCount++;
                break;
            }
        }

        if (not bHunkHasChange)
            continue;

        int iOldStart = 0;
        int iNewStart = 0;
        QString sSection;

        QRegularExpressionMatch tMatch = tHunkRegExp.match(mHunkHeaders.value(iHunkId));
        if (tMatch.hasMatch())
        {
            iOldStart = tMatch.captured(1).toInt();
            iNewStart = tMatch.captured(4).toInt();
            sSection = tMatch.captured(7);
        }

        if (iOldStart != 0)
            bAllOldStartZero = false;
        if (iNewStart != 0)
            bAllNewStartZero = false;

        QString sNewHeader = QString("@@ -%1,%2 +%3,%4 @@%5").arg(iOldStart).arg(iOldCount).arg(iNewStart).arg(iNewCount).arg(sSection);

        lPatchHunks << sNewHeader;
        lPatchHunks << lKeptLines;
        bAtLeastOneHunk = true;
    }

    if (not bAtLeastOneHunk)
        return QString();

    QString sOldSide = bAllOldStartZero ? QString("/dev/null") : QString("a/%1").arg(sRelativeName);
    QString sNewSide = bAllNewStartZero ? QString("/dev/null") : QString("b/%1").arg(sRelativeName);

    QStringList lPatch;
    lPatch << QString("diff --git a/%1 b/%1").arg(sRelativeName);
    lPatch << QString("--- %1").arg(sOldSide);
    lPatch << QString("+++ %1").arg(sNewSide);
    lPatch << lPatchHunks;

    return lPatch.join(NEW_LINE) + NEW_LINE;
}
