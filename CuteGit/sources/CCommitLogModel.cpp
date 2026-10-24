
// Qt
#include <QDebug>
#include <QSet>

// Application
#include "CCommitLogModel.h"
#include "CEnums.h"
#include "CLabel.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

CCommitLogModel::CCommitLogModel(CRepository *pRepository, QObject* parent)
    : QAbstractListModel(parent)
    , m_pRepository(pRepository)
    , m_iPotentialCount(0)
{
}

//-------------------------------------------------------------------------------------------------

CCommitLogModel::~CCommitLogModel()
{
    clear();
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::clear()
{
    qDeleteAll(m_lLines);
    m_lLines.clear();
    qDeleteAll(m_mLabels);
    m_mLabels.clear();
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::reconcileLines(QList<CLogLine*> lIncomingLines, int iStartIndex, int iPotentialCount)
{
    setPotentialCount(iPotentialCount);

    int iIncomingCount = lIncomingLines.count();

    prepareLines(m_lLines, lIncomingLines, iStartIndex);

    // A refresh loads from row 0 while loaded rows stay visible: reconcile in place
    // so the view only updates rows that actually changed
    if (iStartIndex == 0)
    {
        if (iIncomingCount == 0)
        {
            if (not m_lLines.isEmpty())
            {
                beginRemoveRows(QModelIndex(), 0, m_lLines.count() - 1);
                qDeleteAll(m_lLines);
                m_lLines.clear();
                qDeleteAll(m_mLabels);
                m_mLabels.clear();
                endRemoveRows();
            }

            return;
        }

        if (m_lLines.count() >= iIncomingCount)
        {
            bool bIdentical = true;

            for (int iIndex = 0; iIndex < iIncomingCount; iIndex++)
            {
                if (not isSameLine(m_lLines[iIndex], lIncomingLines[iIndex]))
                {
                    bIdentical = false;
                    break;
                }
            }

            // Overlapping rows are unchanged: keep loaded rows (including any
            // prefetched tail) untouched so the view shows no flicker
            if (bIdentical)
            {
                if (m_iPotentialCount > iIncomingCount)
                    trimTailToCount(m_iPotentialCount);

                qDeleteAll(lIncomingLines);

                return;
            }
        }
    }

    for (int iIndex = 0; iIndex < iIncomingCount; iIndex++)
    {
        int iFinalIndex = iStartIndex + iIndex;

        if (iFinalIndex < m_lLines.count())
        {
            if (isSameLine(m_lLines[iFinalIndex], lIncomingLines[iIndex]))
            {
                delete lIncomingLines[iIndex];
                lIncomingLines[iIndex] = nullptr;
            }
            else
            {
                QString sOldCommitId = m_lLines[iFinalIndex] != nullptr ? m_lLines[iFinalIndex]->commitId() : QString("");

                delete m_lLines[iFinalIndex];
                m_lLines[iFinalIndex] = lIncomingLines[iIndex];
                lIncomingLines[iIndex] = nullptr;

                pruneUnusedLabels(sOldCommitId);

                QModelIndex qModelIndex = createIndex(iFinalIndex, 0);
                emit dataChanged(qModelIndex, qModelIndex);
            }
        }
        else if (iFinalIndex == m_lLines.count())
        {
            beginInsertRows(QModelIndex(), m_lLines.count(), m_lLines.count());
            m_lLines << lIncomingLines[iIndex];
            endInsertRows();
            lIncomingLines[iIndex] = nullptr;
        }
        else
        {
            qDebug() << "CCommitLogModel::reconcileLines() : log lines must be sequential";
            delete lIncomingLines[iIndex];
            lIncomingLines[iIndex] = nullptr;
        }
    }

    // A divergent refresh makes rows beyond the new coverage stale: drop them so
    // later paginated loads restart from fresh data
    if (iStartIndex == 0)
        trimTailToCount(iStartIndex + iIncomingCount);

    // Lines may load after the branch data: the label refreshes triggered
    // by branch arrivals ran over no rows then, so sync here. Unchanged
    // rows report no change and emit nothing
    refreshLabels();
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CCommitLogModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames;
    hRoleNames[eCommitIdRole] = "commitId";
    hRoleNames[eDateRole] = "date";
    hRoleNames[eAuthorRole] = "author";
    hRoleNames[eMessageRole] = "message";
    hRoleNames[eFullMessageRole] = "fullMessage";
    hRoleNames[eLabelsRole] = "labels";
    hRoleNames[eMessageIsCompleteRole] = "messageIsComplete";
    hRoleNames[eMarkedAsDiffFromRole] = "markedAsDiffFrom";
    hRoleNames[eMarkedAsDiffToRole] = "markedAsDiffTo";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

int CCommitLogModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_lLines.count();
}

//-------------------------------------------------------------------------------------------------

QVariant CCommitLogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const CLogLine* pLine = lineAtRow(index.row());

    if (pLine == nullptr)
        return QVariant();

    switch (role)
    {
    case eCommitIdRole:
        return pLine->commitId();

    case eDateRole:
        return pLine->date();

    case eAuthorRole:
        return pLine->author();

    case eMessageRole:
        return pLine->message().split(NEW_LINE).first();

    case eFullMessageRole:
        return pLine->message();

    case eLabelsRole:
    {
        QString sCommitId = pLine->commitId();

        if (m_mLabels.contains(sCommitId))
        {
            QVariant vReturnValue;
            vReturnValue.setValue<CLabelModel*>(m_mLabels[sCommitId]);
            return vReturnValue;
        }

        return QVariant();
    }

    case eMessageIsCompleteRole:
        return pLine->messageIsComplete();

    case eMarkedAsDiffFromRole:
        return m_pRepository->diffFromCommitId() == pLine->commitId();

    case eMarkedAsDiffToRole:
        return m_pRepository->diffToCommitId() == pLine->commitId();

    default:
        return QVariant();
    }
}

//-------------------------------------------------------------------------------------------------

bool CCommitLogModel::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_lLines.count() < m_iPotentialCount;
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::fetchMore(const QModelIndex& parent)
{
    Q_UNUSED(parent)

    int iCurrentCount = m_lLines.count();
    int iNewCount = defaultPageSize();

    if (iNewCount > m_iPotentialCount - iCurrentCount)
        iNewCount = m_iPotentialCount - iCurrentCount;

    requestLines(iCurrentCount, iNewCount);
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::commitChanged(const QString& sCommitId)
{
    // Diff markers are computed from the repository, not stored in lines:
    // labels cannot have changed, so only the marker roles refresh
    emitRowsForCommit(sCommitId, {eMarkedAsDiffFromRole, eMarkedAsDiffToRole});
}

/*************************************************************************/
/*!
    \fn const CLogLine* CCommitLogModel::findLine(const QString& sCommitId) const
    Returns the loaded line showing a commit, nullptr when not loaded.
*/
const CLogLine* CCommitLogModel::findLine(const QString& sCommitId) const
{
    if (sCommitId.isEmpty())
        return nullptr;

    for (const CLogLine* pLine : m_lLines)
    {
        if (pLine != nullptr && pLine->commitId() == sCommitId)
            return pLine;
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::refreshLabels()
{
    QSet<QString> sHandledCommitIds;
    QSet<QString> sChangedCommitIds;

    for (CLogLine* pLine : m_lLines)
    {
        if (pLine == nullptr)
            continue;

        QString sCommitId = pLine->commitId();

        if (sCommitId.isEmpty() || sHandledCommitIds.contains(sCommitId))
            continue;

        sHandledCommitIds << sCommitId;

        if (syncLabelsForCommit(sCommitId))
            sChangedCommitIds << sCommitId;
    }

    if (sChangedCommitIds.isEmpty())
        return;

    // Changed rows emit per contiguous run: a single spanning range would
    // also cover unchanged rows between sparse changes and scale poorly
    // on large graphs, and this refresh runs on every branch/tag arrival
    int iRunFirstRow = -1;
    int iRunLastRow = -1;

    for (int iLineIndex = 0; iLineIndex <= m_lLines.count(); iLineIndex++)
    {
        CLogLine* pLine = iLineIndex < m_lLines.count() ? m_lLines[iLineIndex] : nullptr;
        bool bIsChangedRow = pLine != nullptr && sChangedCommitIds.contains(pLine->commitId());

        if (bIsChangedRow)
        {
            if (iRunFirstRow < 0)
                iRunFirstRow = iLineIndex;

            iRunLastRow = iLineIndex;
        }
        else if (iRunFirstRow >= 0)
        {
            emit dataChanged(index(iRunFirstRow), index(iRunLastRow), {eLabelsRole});
            iRunFirstRow = -1;
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::refreshLabelsForCommit(const QString& sCommitId)
{
    if (sCommitId.isEmpty())
        return;

    if (not syncLabelsForCommit(sCommitId))
        return;

    emitRowsForCommit(sCommitId, {eLabelsRole});
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::prepareLines(QList<CLogLine*>& lLoadedLines, QList<CLogLine*>& lIncomingLines, int iStartIndex)
{
    Q_UNUSED(lLoadedLines)
    Q_UNUSED(lIncomingLines)
    Q_UNUSED(iStartIndex)
}

//-------------------------------------------------------------------------------------------------

QList<CLabel*> CCommitLogModel::labelsFromLine(const CLogLine* pLine) const
{
    Q_UNUSED(pLine)

    return QList<CLabel*>();
}

//-------------------------------------------------------------------------------------------------

// Returns true when both lines show the same commit content
// Existing lines may hold the full message while incoming lines hold the subject:
// only the first line is compared
bool CCommitLogModel::isSameCommitContent(const CLogLine* pExistingLine, const CLogLine* pIncomingLine)
{
    if (pExistingLine == nullptr || pIncomingLine == nullptr)
        return pExistingLine == pIncomingLine;

    if (pExistingLine->commitId() != pIncomingLine->commitId())
        return false;

    if (pExistingLine->author() != pIncomingLine->author())
        return false;

    if (pExistingLine->date() != pIncomingLine->date())
        return false;

    return pExistingLine->message().split(NEW_LINE).first() == pIncomingLine->message().split(NEW_LINE).first();
}

//-------------------------------------------------------------------------------------------------

// Drops loaded rows beyond a kept row count, deleting their lines and orphaned labels
void CCommitLogModel::trimTailToCount(int iKeepCount)
{
    while (m_lLines.count() > iKeepCount)
    {
        int iLastIndex = m_lLines.count() - 1;
        beginRemoveRows(QModelIndex(), iLastIndex, iLastIndex);
        CLogLine* pRemovedLine = m_lLines.takeLast();
        endRemoveRows();
        QString sRemovedCommitId = pRemovedLine->commitId();
        delete pRemovedLine;
        pruneUnusedLabels(sRemovedCommitId);
    }
}

//-------------------------------------------------------------------------------------------------

// Emits dataChanged on every row showing a commit, for the given roles (all roles when empty)
void CCommitLogModel::emitRowsForCommit(const QString& sCommitId, const QList<int>& lRoles)
{
    if (sCommitId.isEmpty())
        return;

    for (int iLineIndex = 0; iLineIndex < m_lLines.count(); iLineIndex++)
    {
        CLogLine* pLine = m_lLines[iLineIndex];

        if (pLine != nullptr && pLine->commitId() == sCommitId)
        {
            QModelIndex qModelIndex = index(iLineIndex);

            if (lRoles.isEmpty())
                emit dataChanged(qModelIndex, qModelIndex);
            else
                emit dataChanged(qModelIndex, qModelIndex, lRoles);
        }
    }
}

//-------------------------------------------------------------------------------------------------

/*************************************************************************/
/*!
    \fn const CLogLine* CCommitLogModel::lineAtRow(int iRow) const
    Returns the loaded line at a row, nullptr when out of range.
*/
const CLogLine* CCommitLogModel::lineAtRow(int iRow) const
{
    if (iRow < 0 || iRow >= m_lLines.count())
        return nullptr;

    return m_lLines[iRow];
}

//-------------------------------------------------------------------------------------------------

void CCommitLogModel::pruneUnusedLabels(const QString& sCommitId)
{
    if (sCommitId.isEmpty() || not m_mLabels.contains(sCommitId))
        return;

    for (const CLogLine* pLine : m_lLines)
    {
        if (pLine != nullptr && pLine->commitId() == sCommitId)
            return;
    }

    // No remaining row shows the commit: QML wrappers null themselves on
    // QObject destruction, so the orphaned model is deleted instead of kept
    // empty, otherwise the map grows by one entry per commit ever seen
    delete m_mLabels.take(sCommitId);
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn bool CCommitLogModel::syncLabelsForCommit(const QString& sCommitId)
    Rebuilds the label model of a commit from the repository and from the
    loaded lines, returns true when the visible labels changed. The model is
    updated in place so QML delegates caching its pointer stay valid.
*/
bool CCommitLogModel::syncLabelsForCommit(const QString& sCommitId)
{
    if (sCommitId.isEmpty() || m_pRepository == nullptr)
        return false;

    QList<CLabel*> lFreshLabels = m_pRepository->labelsForCommit(sCommitId);

    QSet<QString> sKnownSignatures;

    for (const CLabel* pLabel : lFreshLabels)
    {
        if (pLabel != nullptr)
            sKnownSignatures << CLabel::signature(pLabel);
    }

    // Lines may carry labels the repository models do not know, such as a
    // detached HEAD decoration: merge them, skipping duplicates already known
    for (const CLogLine* pLine : m_lLines)
    {
        if (pLine == nullptr || pLine->commitId() != sCommitId)
            continue;

        for (CLabel* pLabel : labelsFromLine(pLine))
        {
            if (pLabel == nullptr)
                continue;

            if (sKnownSignatures.contains(CLabel::signature(pLabel)))
                delete pLabel;
            else
            {
                sKnownSignatures << CLabel::signature(pLabel);
                lFreshLabels << pLabel;
            }
        }
    }

    CLabelModel* pModel = m_mLabels.value(sCommitId, nullptr);

    if (pModel == nullptr)
    {
        // No loaded row shows the commit: its labels stay invisible until
        // its lines load, so no model is kept and the map never grows
        // with entries pruneUnusedLabels cannot collect
        if (lFreshLabels.isEmpty() || findLine(sCommitId) == nullptr)
        {
            qDeleteAll(lFreshLabels);
            return false;
        }

        pModel = new CLabelModel(this);
        m_mLabels[sCommitId] = pModel;
    }

    return pModel->syncLabels(lFreshLabels);
}
