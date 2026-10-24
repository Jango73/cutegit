
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
        lIncomingLines << pLine;

    CCommitLogModel::setLines(lIncomingLines, tCollection.startIndex(), tCollection.potentialCount());
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

    const CGraphLine* pGraphLine = static_cast<const CGraphLine*>(pLine);

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
        lLoadedGraphLines << static_cast<CGraphLine*>(pLine);

    for (CLogLine* pLine : lIncomingLines)
        lIncomingGraphLines << static_cast<CGraphLine*>(pLine);

    layoutIncomingLines(lLoadedGraphLines, lIncomingGraphLines, iStartIndex);
}

//-------------------------------------------------------------------------------------------------

bool CGraphModel::isSameLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const
{
    if (pExistingLine == nullptr || pIncomingLine == nullptr)
        return pExistingLine == pIncomingLine;

    const CGraphLine* pExistingGraphLine = static_cast<const CGraphLine*>(pExistingLine);
    const CGraphLine* pIncomingGraphLine = static_cast<const CGraphLine*>(pIncomingLine);

    if (pExistingGraphLine->commitId() != pIncomingGraphLine->commitId())
        return false;

    if (pExistingGraphLine->decorations() != pIncomingGraphLine->decorations())
        return false;

    if (pExistingGraphLine->graphLane() != pIncomingGraphLine->graphLane())
        return false;

    if (pExistingGraphLine->graphVerticals() != pIncomingGraphLine->graphVerticals())
        return false;

    if (pExistingGraphLine->graphConnections() != pIncomingGraphLine->graphConnections())
        return false;

    if (pExistingGraphLine->author() != pIncomingGraphLine->author())
        return false;

    if (pExistingGraphLine->date() != pIncomingGraphLine->date())
        return false;

    // Existing lines may hold the full message while incoming lines hold the subject:
    // compare the first line only
    return pExistingGraphLine->message().split(NEW_LINE).first() == pIncomingGraphLine->message().split(NEW_LINE).first();
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
