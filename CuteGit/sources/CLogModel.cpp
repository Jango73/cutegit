
// Qt
#include <QDebug>
#include <QSet>

// Application
#include "CLogModel.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

CLogModel::CLogModel(CRepository *pRepository, QObject* parent)
    : QAbstractListModel(parent)
    , m_pRepository(pRepository)
    , m_iPotentialCount(0)
{
}

//-------------------------------------------------------------------------------------------------

CLogModel::~CLogModel()
{
    clear();
}

//-------------------------------------------------------------------------------------------------

void CLogModel::clear()
{
    qDeleteAll(m_lLines);
    m_lLines.clear();
    qDeleteAll(m_mLabels);
    m_mLabels.clear();
}

//-------------------------------------------------------------------------------------------------

void CLogModel::setLines(CLogLineCollection tCollection)
{
    setPotentialCount(tCollection.potentialCount());

    QList<CLogLine*> lIncomingLines = tCollection.lines();
    int iStartIndex = tCollection.startIndex();
    int iIncomingCount = lIncomingLines.count();

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
                if (not isSameLogLine(m_lLines[iIndex], lIncomingLines[iIndex]))
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
            if (isSameLogLine(m_lLines[iFinalIndex], lIncomingLines[iIndex]))
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
            qDebug() << "CLogModel::setLines() : log lines must be sequential";
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

QHash<int, QByteArray> CLogModel::roleNames() const
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

int CLogModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_lLines.count();
}

//-------------------------------------------------------------------------------------------------

QVariant CLogModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if (!index.isValid())
        return QVariant();

    if (row < 0 || row >= m_lLines.count())
    {
        return QVariant();
    }

    switch (role)
    {
    case eCommitIdRole:
        return m_lLines[row]->commitId();

    case eDateRole:
        return m_lLines[row]->date();

    case eAuthorRole:
        return m_lLines[row]->author();

    case eMessageRole:
        return m_lLines[row]->message().split(NEW_LINE).first();

    case eFullMessageRole:
        return m_lLines[row]->message();

    case eLabelsRole:
    {
        QString sCommitId = m_lLines[row]->commitId();

        if (m_mLabels.contains(sCommitId))
        {
            QVariant vReturnValue;
            vReturnValue.setValue<CLabelModel*>(m_mLabels[sCommitId]);
            return vReturnValue;
        }

        return QVariant();
    }

    case eMessageIsCompleteRole:
        return m_lLines[row]->messageIsComplete();

    case eMarkedAsDiffFromRole:
        return m_pRepository->diffFromCommitId() == m_lLines[row]->commitId();

    case eMarkedAsDiffToRole:
        return m_pRepository->diffToCommitId() == m_lLines[row]->commitId();

    default:
        return QVariant();
    }
}

//-------------------------------------------------------------------------------------------------

bool CLogModel::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_lLines.count() < m_iPotentialCount;
}

//-------------------------------------------------------------------------------------------------

void CLogModel::fetchMore(const QModelIndex& parent)
{
    Q_UNUSED(parent)

    int iCurrentCount = m_lLines.count();
    int iNewCount = LOG_COUNT_DEFAULT;

    if (iNewCount > m_iPotentialCount - iCurrentCount)
        iNewCount = m_iPotentialCount - iCurrentCount;

    emit requestLogData(iCurrentCount, iNewCount);
}

//-------------------------------------------------------------------------------------------------

void CLogModel::commitChanged(const QString& sCommitId)
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

//-------------------------------------------------------------------------------------------------

void CLogModel::refreshLabels()
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

void CLogModel::refreshLabelsForCommit(const QString& sCommitId)
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

void CLogModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

//-------------------------------------------------------------------------------------------------

void CLogModel::pruneUnusedLabels(const QString& sCommitId)
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
    \fn bool CLogModel::syncLabelsForCommit(const QString& sCommitId)
    Rebuilds the label model of a commit from the repository, returns true
    when the visible labels changed. The model is updated in place so QML
    delegates caching its pointer stay valid.
*/
bool CLogModel::syncLabelsForCommit(const QString& sCommitId)
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

//-------------------------------------------------------------------------------------------------

bool CLogModel::isSameLogLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const
{
    if (pExistingLine == nullptr || pIncomingLine == nullptr)
        return pExistingLine == pIncomingLine;

    if (pExistingLine->commitId() != pIncomingLine->commitId())
        return false;

    if (pExistingLine->author() != pIncomingLine->author())
        return false;

    if (pExistingLine->date() != pIncomingLine->date())
        return false;

    // Existing lines may hold the full message while incoming lines hold the subject:
    // compare the first line only
    return pExistingLine->message().split(NEW_LINE).first() == pIncomingLine->message().split(NEW_LINE).first();
}
