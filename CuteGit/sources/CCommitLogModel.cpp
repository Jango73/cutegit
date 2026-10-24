
// Qt
#include <QDebug>
#include <QSet>

// Application
#include "CCommitLogModel.h"
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

void CCommitLogModel::setLines(QList<CLogLine*> lIncomingLines, int iStartIndex, int iPotentialCount)
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
                {
                    while (m_lLines.count() > m_iPotentialCount)
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
            qDebug() << "CCommitLogModel::setLines() : log lines must be sequential";
            delete lIncomingLines[iIndex];
            lIncomingLines[iIndex] = nullptr;
        }
    }

    // A divergent refresh makes rows beyond the new coverage stale: drop them so
    // later paginated loads restart from fresh data
    if (iStartIndex == 0)
    {
        while (m_lLines.count() > iStartIndex + iIncomingCount)
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
    syncLabelsForCommit(sCommitId);

    for (int iLineIndex = 0; iLineIndex < m_lLines.count(); iLineIndex++)
    {
        CLogLine* pLine = m_lLines[iLineIndex];

        if (pLine != nullptr)
        {
            if (pLine->commitId() == sCommitId)
            {
                emit dataChanged(index(iLineIndex), index(iLineIndex));
                break;
            }
        }
    }
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

    for (CLogLine* pLine : m_lLines)
    {
        if (pLine == nullptr)
            continue;

        QString sCommitId = pLine->commitId();

        if (sCommitId.isEmpty() || sHandledCommitIds.contains(sCommitId))
            continue;

        sHandledCommitIds << sCommitId;

        if (not syncLabelsForCommit(sCommitId))
            continue;

        for (int iLineIndex = 0; iLineIndex < m_lLines.count(); iLineIndex++)
        {
            CLogLine* pOtherLine = m_lLines[iLineIndex];

            if (pOtherLine != nullptr && pOtherLine->commitId() == sCommitId)
                emit dataChanged(index(iLineIndex), index(iLineIndex));
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

    for (int iLineIndex = 0; iLineIndex < m_lLines.count(); iLineIndex++)
    {
        CLogLine* pLine = m_lLines[iLineIndex];

        if (pLine != nullptr && pLine->commitId() == sCommitId)
            emit dataChanged(index(iLineIndex), index(iLineIndex));
    }
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

    // Clear the model in place instead of deleting it: QML delegates cache
    // the model pointer, deleting it leaves a dangling wrapper behind
    QList<CLabel*> lEmptyLabels;
    m_mLabels[sCommitId]->setLabelList(lEmptyLabels);
}

//-------------------------------------------------------------------------------------------------

/************************************************************************/
/*!
    \fn bool CCommitLogModel::syncLabelsForCommit(const QString& sCommitId)
    Rebuilds the label model of a commit from the repository, returns true
    when the visible labels changed. The model is updated in place so QML
    delegates caching its pointer stay valid.
*/
bool CCommitLogModel::syncLabelsForCommit(const QString& sCommitId)
{
    if (sCommitId.isEmpty() || m_pRepository == nullptr)
        return false;

    QList<CLabel*> lFreshLabels = m_pRepository->labelsForCommit(sCommitId);

    CLabelModel* pModel = m_mLabels.value(sCommitId, nullptr);

    if (pModel == nullptr)
    {
        if (lFreshLabels.isEmpty())
            return false;

        pModel = new CLabelModel(this);
        m_mLabels[sCommitId] = pModel;
    }

    return pModel->syncLabels(lFreshLabels);
}
