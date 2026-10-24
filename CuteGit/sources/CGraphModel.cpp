
// Qt
#include <QDebug>

// Application
#include "CGraphModel.h"
#include "CGraphLayout.h"
#include "CEnums.h"

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

//-------------------------------------------------------------------------------------------------

CGraphModel::CGraphModel(CRepository* pRepository, QObject* parent)
    : CCommitLogModel(pRepository, parent)
{
}

//-------------------------------------------------------------------------------------------------

CGraphModel::~CGraphModel()
{
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::setLines(CGraphLineCollection tCollection)
{
    QList<CLogLine*> lIncomingLines;

    for (CLogLine* pLine : tCollection.lines())
    {
        CGraphLine* pGraphLine = qobject_cast<CGraphLine*>(pLine);
        Q_ASSERT(pGraphLine != nullptr);

        if (pGraphLine == nullptr)
        {
            qWarning() << "CGraphModel::setLines() : skipping a non-graph line";
            delete pLine;
            continue;
        }

        lIncomingLines << pGraphLine;
    }

    reconcileLines(lIncomingLines, tCollection.startIndex(), tCollection.potentialCount());
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CGraphModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames = CCommitLogModel::roleNames();
    hRoleNames[eGraphCommitLaneRole] = "graphCommitLane";
    hRoleNames[eGraphVerticalsRole] = "graphVerticals";
    hRoleNames[eGraphConnectionsRole] = "graphConnections";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

QVariant CGraphModel::data(const QModelIndex& index, int role) const
{
    const CLogLine* pLine = lineAtRow(index.row());

    if (pLine == nullptr)
        return QVariant();

    // Only graph lines ever enter this model: the reconciler is protected and
    // the typed setLines wrapper guards every incoming line
    const CGraphLine* pGraphLine = static_cast<const CGraphLine*>(pLine);
    Q_ASSERT(qobject_cast<const CGraphLine*>(pLine) != nullptr);

    switch (role)
    {
    case eGraphCommitLaneRole:
        return pGraphLine->graphLane();

    case eGraphVerticalsRole:
        return pGraphLine->graphVerticals();

    case eGraphConnectionsRole:
        return pGraphLine->graphConnections();

    default:
        return CCommitLogModel::data(index, role);
    }
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::prepareLines(QList<CLogLine*>& lLoadedLines, QList<CLogLine*>& lIncomingLines, int iStartIndex)
{
    QList<CGraphLine*> lLoadedGraphLines;
    QList<CGraphLine*> lIncomingGraphLines;

    for (CLogLine* pLine : lLoadedLines)
    {
        Q_ASSERT(qobject_cast<CGraphLine*>(pLine) != nullptr);
        lLoadedGraphLines << static_cast<CGraphLine*>(pLine);
    }

    for (CLogLine* pLine : lIncomingLines)
    {
        Q_ASSERT(qobject_cast<CGraphLine*>(pLine) != nullptr);
        lIncomingGraphLines << static_cast<CGraphLine*>(pLine);
    }

    layoutIncomingLines(lLoadedGraphLines, lIncomingGraphLines, iStartIndex);
}

//-------------------------------------------------------------------------------------------------

bool CGraphModel::isSameLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const
{
    if (not isSameCommitContent(pExistingLine, pIncomingLine))
        return false;

    if (pExistingLine == nullptr || pIncomingLine == nullptr)
        return true;

    const CGraphLine* pExistingGraphLine = static_cast<const CGraphLine*>(pExistingLine);
    const CGraphLine* pIncomingGraphLine = static_cast<const CGraphLine*>(pIncomingLine);

    if (pExistingGraphLine->decorations() != pIncomingGraphLine->decorations())
        return false;

    if (pExistingGraphLine->graphLane() != pIncomingGraphLine->graphLane())
        return false;

    if (pExistingGraphLine->graphVerticals() != pIncomingGraphLine->graphVerticals())
        return false;

    if (pExistingGraphLine->graphConnections() != pIncomingGraphLine->graphConnections())
        return false;

    return true;
}

//-------------------------------------------------------------------------------------------------

QList<CLabel*> CGraphModel::labelsFromLine(const CLogLine* pLine) const
{
    const CGraphLine* pGraphLine = qobject_cast<const CGraphLine*>(pLine);

    if (pGraphLine == nullptr)
        return QList<CLabel*>();

    return pGraphLine->createLabels();
}

//-------------------------------------------------------------------------------------------------

int CGraphModel::defaultPageSize() const
{
    return GRAPH_COUNT_DEFAULT;
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::requestLines(int iStartIndex, int iCount)
{
    emit requestGraphData(iStartIndex, iCount);
}
