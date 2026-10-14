
// Qt
#include <QModelIndex>
#include <QRegularExpression>

// Application
#include "CPatchBuilder.h"
#include "CDiffLine.h"
#include "CDiffConstants.h"

//-------------------------------------------------------------------------------------------------

// Capture positions inside CDiffConstants::sHunkHeaderPattern
static const int iHunkOldStartCapture = CDiffConstants::iHunkOldStartCapture;
static const int iHunkNewStartCapture = CDiffConstants::iHunkNewStartCapture;
static const int iHunkNewCountWithCommaCapture = CDiffConstants::iHunkNewCountWithCommaCapture;
static const int iHunkNewCountCapture = CDiffConstants::iHunkNewCountCapture;
static const int iHunkSectionCapture = CDiffConstants::iHunkSectionCapture;

// Hunk starts for a file that appears from nothing or disappears entirely
static const int iNullHunkStart = 0;

// Line count used when the hunk header omits the ,count part
static const int iImplicitHunkLineCount = 1;

// Cumulative new-side shift, initially none
static const int iNoNewSideShift = 0;

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

        // Validate that the mapped row belongs to the expected model:
        // the proxy may be rebound while QML still holds stale rows
        if (tSourceIndex.model() != pModel)
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

    if (pModel == nullptr || iHunkId == CDiffConstants::iInvalidHunkId)
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
    QRegularExpression tHunkRegExp(CDiffConstants::sHunkHeaderPattern);

    // Group source rows by hunk, keeping model order. Rows from other files
    // are skipped so a multi-file model never merges hunks into one patch.
    QMap<int, QList<int>> mHunkRows;
    QMap<int, QString> mHunkHeaders;

    for (int iRow = 0; iRow < lLines.count(); iRow++)
    {
        CDiffLine* pLine = lLines[iRow];

        if (pLine == nullptr)
            continue;

        if (pLine->operation() == CEnums::DiffFileName)
            continue;

        if (not pLine->fileName().isEmpty() && pLine->fileName() != sRelativeName)
            continue;

        int iHunkId = pLine->hunkId();

        if (iHunkId == CDiffConstants::iInvalidHunkId)
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
    int iNewOffset = iNoNewSideShift;

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

        QString sOriginalHeader = mHunkHeaders.value(iHunkId);
        QRegularExpressionMatch tMatch = tHunkRegExp.match(sOriginalHeader);

        // Unparseable headers are kept verbatim: rebuilding them with 0
        // starts would wrongly switch patch sides to /dev/null
        if (not tMatch.hasMatch())
        {
            bAllOldStartZero = false;
            bAllNewStartZero = false;
            lPatchHunks << sOriginalHeader;
            lPatchHunks << lKeptLines;
            bAtLeastOneHunk = true;
            continue;
        }

        int iOriginalOldStart = tMatch.captured(iHunkOldStartCapture).toInt();
        int iOriginalNewStart = tMatch.captured(iHunkNewStartCapture).toInt();
        int iOriginalNewCount = tMatch.captured(iHunkNewCountWithCommaCapture).isEmpty() ? iImplicitHunkLineCount : tMatch.captured(iHunkNewCountCapture).toInt();
        QString sSection = tMatch.captured(iHunkSectionCapture);

        if (iOriginalOldStart != iNullHunkStart)
            bAllOldStartZero = false;
        if (iOriginalNewStart != iNullHunkStart)
            bAllNewStartZero = false;

        // Dropped added lines and context-kept deletions shift the new side
        // of every following hunk: adjust with the cumulative difference
        int iAdjustedNewStart = iOriginalNewStart + iNewOffset;

        if (iOriginalNewStart == iNullHunkStart)
            iAdjustedNewStart = iNullHunkStart;

        QString sNewHeader = QString("@@ -%1,%2 +%3,%4 @@%5").arg(iOriginalOldStart).arg(iOldCount).arg(iAdjustedNewStart).arg(iNewCount).arg(sSection);

        lPatchHunks << sNewHeader;
        lPatchHunks << lKeptLines;
        bAtLeastOneHunk = true;

        iNewOffset += (iNewCount - iOriginalNewCount);
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
