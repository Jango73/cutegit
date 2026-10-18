
// Qt
#include <QDebug>
#include <QSet>

// Application
#include "CGraphModel.h"
#include "CGraphLayout.h"

//-------------------------------------------------------------------------------------------------

CGraphModel::CGraphModel(CRepository* pRepository, QObject* parent)
    : QAbstractListModel(parent)
    , m_pRepository(pRepository)
    , m_iPotentialCount(0)
{
}

//-------------------------------------------------------------------------------------------------

CGraphModel::~CGraphModel()
{
    clear();
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::clear()
{
    qDeleteAll(m_lLines);
    m_lLines.clear();
    qDeleteAll(m_mLabels);
    m_mLabels.clear();
}

/***************************************************************************/

/*! Updates the label model of a commit with owned labels, keeping the model identity */
static void syncLabelModel(QObject* pParent, QMap<QString, CLabelModel*>& mLabels, const QString& sCommitId, QList<CLabel*> lLabels)
{
    CLabelModel* pModel = mLabels.value(sCommitId, nullptr);

    if (pModel == nullptr)
    {
        if (lLabels.isEmpty())
            return;

        pModel = new CLabelModel(pParent);
        mLabels[sCommitId] = pModel;
    }

    // Update in place: the model pointer is exposed to QML and must stay valid
    pModel->syncLabels(lLabels);
}

/***************************************************************************/

/*! Returns the commit ids shown by lines */
static QSet<QString> commitIdsOfLines(const QList<CGraphLine*>& lLines)
{
    QSet<QString> sReturnValue;

    for (const CGraphLine* pLine : lLines)
    {
        if (pLine != nullptr && !pLine->commitId().isEmpty())
            sReturnValue << pLine->commitId();
    }

    return sReturnValue;
}

/***************************************************************************/

/*! Clears the label model of a commit when no remaining line shows it, keeping the model identity */
static void clearLabelModelIfOrphaned(QMap<QString, CLabelModel*>& mLabels, const QList<CGraphLine*>& lLines, const QString& sCommitId)
{
    if (sCommitId.isEmpty() || !mLabels.contains(sCommitId))
        return;

    if (!commitIdsOfLines(lLines).contains(sCommitId))
    {
        QList<CLabel*> lEmptyLabels;
        mLabels[sCommitId]->setLabelList(lEmptyLabels);
    }
}

/***************************************************************************/

/*! Lays out incoming lines in the context of already loaded lines */
// Layout is sequential: lanes of loaded rows never change when rows are appended.
// A refresh lays out its page alone, an append lays out loaded rows plus the tail.
static void layoutIncomingLines(const QList<CGraphLine*>& lLoadedLines, QList<CGraphLine*>& lIncomingLines, int iStartIndex)
{
    if (iStartIndex == 0)
    {
        CGraphLayout::layoutLines(lIncomingLines);
    }
    else
    {
        QList<CGraphLine*> lCombined = lLoadedLines + lIncomingLines;
        CGraphLayout::layoutLines(lCombined);
    }
}

/***************************************************************************/

void CGraphModel::setLines(CGraphLineCollection tCollection)
{
    setPotentialCount(tCollection.potentialCount());

    QList<CGraphLine*> lIncomingLines = tCollection.lines();
    int iStartIndex = tCollection.startIndex();
    int iIncomingCount = lIncomingLines.count();

    layoutIncomingLines(m_lLines, lIncomingLines, iStartIndex);

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
                if (not isSameGraphLine(m_lLines[iIndex], lIncomingLines[iIndex]))
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
                        CGraphLine* pRemovedLine = m_lLines.takeLast();
                        endRemoveRows();
                        QString sRemovedCommitId = pRemovedLine->commitId();
                        delete pRemovedLine;
                        clearLabelModelIfOrphaned(m_mLabels, m_lLines, sRemovedCommitId);
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
            if (isSameGraphLine(m_lLines[iFinalIndex], lIncomingLines[iIndex]))
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

                clearLabelModelIfOrphaned(m_mLabels, m_lLines, sOldCommitId);
                syncLabelModel(this, m_mLabels, m_lLines[iFinalIndex]->commitId(), m_lLines[iFinalIndex]->createLabels());

                QModelIndex qModelIndex = createIndex(iFinalIndex, 0);
                emit dataChanged(qModelIndex, qModelIndex);
            }
        }
        else if (iFinalIndex == m_lLines.count())
        {
            syncLabelModel(this, m_mLabels, lIncomingLines[iIndex]->commitId(), lIncomingLines[iIndex]->createLabels());
            beginInsertRows(QModelIndex(), m_lLines.count(), m_lLines.count());
            m_lLines << lIncomingLines[iIndex];
            endInsertRows();
            lIncomingLines[iIndex] = nullptr;
        }
        else
        {
            qDebug() << "CGraphModel::setLines() : graph lines must be sequential";
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
            CGraphLine* pRemovedLine = m_lLines.takeLast();
            endRemoveRows();
            QString sRemovedCommitId = pRemovedLine->commitId();
            delete pRemovedLine;
            clearLabelModelIfOrphaned(m_mLabels, m_lLines, sRemovedCommitId);
        }
    }
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CGraphModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames;
    hRoleNames[eCommitIdRole] = "commitId";
    hRoleNames[eDateRole] = "date";
    hRoleNames[eAuthorRole] = "author";
    hRoleNames[eMessageRole] = "message";
    hRoleNames[eLabelsRole] = "labels";
    hRoleNames[eMessageIsCompleteRole] = "messageIsComplete";
    hRoleNames[eGraphCommitLaneRole] = "graphCommitLane";
    hRoleNames[eGraphVerticalsRole] = "graphVerticals";
    hRoleNames[eGraphConnectionsRole] = "graphConnections";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

int CGraphModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_lLines.count();
}

//-------------------------------------------------------------------------------------------------

QVariant CGraphModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if (!index.isValid())
        return QVariant();

    if ((row < 0) || (row > (rowCount() - 1)))
        return QVariant();

    switch (role)
    {
    case eCommitIdRole:
        return m_lLines[row]->commitId();

    case eDateRole:
        return m_lLines[row]->date();

    case eAuthorRole:
        return m_lLines[row]->author();

    case eMessageRole:
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

    case eGraphCommitLaneRole:
        return m_lLines[row]->graphLane();

    case eGraphVerticalsRole:
        return m_lLines[row]->graphVerticals();

    case eGraphConnectionsRole:
        return m_lLines[row]->graphConnections();
    }

    return QVariant();
}

//-------------------------------------------------------------------------------------------------

bool CGraphModel::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_lLines.count() < m_iPotentialCount;
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::fetchMore(const QModelIndex& parent)
{
    Q_UNUSED(parent)

    int iCurrentCount = m_lLines.count();
    int iNewCount = GRAPH_COUNT_DEFAULT;

    if (iNewCount > m_iPotentialCount - iCurrentCount)
        iNewCount = m_iPotentialCount - iCurrentCount;

    emit requestGraphData(iCurrentCount, iNewCount);
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::commitChanged(const QString& sCommitId)
{
    for (int iLineIndex = 0; iLineIndex < m_lLines.count(); iLineIndex++)
    {
        CGraphLine* pLine = m_lLines[iLineIndex];

        if (pLine->commitId() == sCommitId)
        {
            emit dataChanged(index(iLineIndex), index(iLineIndex));
            break;
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

//-------------------------------------------------------------------------------------------------

bool CGraphModel::isSameGraphLine(const CGraphLine* pExistingLine, const CGraphLine* pIncomingLine) const
{
    if (pExistingLine == nullptr || pIncomingLine == nullptr)
        return pExistingLine == pIncomingLine;

    if (pExistingLine->commitId() != pIncomingLine->commitId())
        return false;

    if (pExistingLine->decorations() != pIncomingLine->decorations())
        return false;

    if (pExistingLine->graphLane() != pIncomingLine->graphLane())
        return false;

    if (pExistingLine->graphVerticals() != pIncomingLine->graphVerticals())
        return false;

    if (pExistingLine->graphConnections() != pIncomingLine->graphConnections())
        return false;

    if (pExistingLine->author() != pIncomingLine->author())
        return false;

    if (pExistingLine->date() != pIncomingLine->date())
        return false;

    // Existing lines may hold the full message while incoming lines hold the subject:
    // compare the first line only
    return pExistingLine->message().split(NEW_LINE).first() == pIncomingLine->message().split(NEW_LINE).first();
}
