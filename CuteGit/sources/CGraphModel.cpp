
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

QString CGraphModel::workingTreeCommitId()
{
    return QString("WORKING_TREE");
}

//-------------------------------------------------------------------------------------------------

CGraphModel::CGraphModel(CRepository* pRepository, QObject* parent)
    : CCommitLogModel(pRepository, parent)
    , m_bWorkingTreePresent(false)
{
}

//-------------------------------------------------------------------------------------------------

CGraphModel::~CGraphModel()
{
}

//-------------------------------------------------------------------------------------------------

bool CGraphModel::hasWorkingTreePresent() const
{
    return m_bWorkingTreePresent;
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::setWorkingTreePresent(bool bPresent)
{
    if (m_bWorkingTreePresent == bPresent)
    {
        // The flag may survive a clear without its row: restore the row then
        if (bPresent && (m_lLines.isEmpty() || m_lLines.first() == nullptr || m_lLines.first()->commitId() != workingTreeCommitId()))
        {
            CGraphLine* pLine = createWorkingTreeLine(firstRealCommitId());

            beginInsertRows(QModelIndex(), 0, 0);
            m_lLines.prepend(pLine);
            endInsertRows();

            refreshWorkingTreeParent();
            layoutLoadedLines();
        }

        return;
    }

    m_bWorkingTreePresent = bPresent;

    if (bPresent)
    {
        CGraphLine* pLine = createWorkingTreeLine(firstRealCommitId());

        beginInsertRows(QModelIndex(), 0, 0);
        m_lLines.prepend(pLine);
        endInsertRows();

        if (m_iPotentialCount > 0)
            setPotentialCount(m_iPotentialCount + 1);
        else
            setPotentialCount(m_lLines.count());

        refreshWorkingTreeParent();
        layoutLoadedLines();
    }
    else
    {
        if (not m_lLines.isEmpty() && m_lLines.first() != nullptr && m_lLines.first()->commitId() == workingTreeCommitId())
        {
            beginRemoveRows(QModelIndex(), 0, 0);
            delete m_lLines.takeFirst();
            endRemoveRows();
        }

        if (m_iPotentialCount > 0)
            setPotentialCount(qMax(0, m_iPotentialCount - 1));

        layoutLoadedLines();
    }
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

        // The working tree line never comes from the version control: drop any collision
        if (pGraphLine->commitId() == workingTreeCommitId())
        {
            qWarning() << "CGraphModel::setLines() : skipping a working tree collision";
            delete pLine;
            continue;
        }

        lIncomingLines << pGraphLine;
    }

    int iOriginalStartIndex = tCollection.startIndex();
    int iStartIndex = tCollection.startIndex();
    int iPotentialCount = tCollection.potentialCount();

    if (m_bWorkingTreePresent)
    {
        if (m_lLines.isEmpty() || m_lLines.first() == nullptr || m_lLines.first()->commitId() != workingTreeCommitId())
        {
            CGraphLine* pLine = createWorkingTreeLine(lIncomingLines.isEmpty() ? firstRealCommitId() : static_cast<CGraphLine*>(lIncomingLines.first())->commitId());

            beginInsertRows(QModelIndex(), 0, 0);
            m_lLines.prepend(pLine);
            endInsertRows();
        }
        else if (iStartIndex == 0 && not lIncomingLines.isEmpty())
        {
            CGraphLine* pWorkingTreeLine = static_cast<CGraphLine*>(m_lLines.first());
            CGraphLine* pFirstIncomingLine = static_cast<CGraphLine*>(lIncomingLines.first());

            if (pWorkingTreeLine != nullptr && pFirstIncomingLine != nullptr)
                pWorkingTreeLine->setParents(QStringList() << pFirstIncomingLine->commitId());
        }

        if (iPotentialCount == 0)
            iPotentialCount = lIncomingLines.count() + 1;
        else
            iPotentialCount += 1;

        iStartIndex += 1;
    }

    int iIncomingCount = lIncomingLines.count();

    reconcileLines(lIncomingLines, iStartIndex, iPotentialCount);

    // A refresh loads from row 0 but shifts to 1 with the working tree line:
    // reconcileLines only trims the stale tail on start 0, so a shrinking
    // history (squash, rebase, reset) keeps dead rows without this trim.
    // Selecting a dead row then diffs an unknown revision.
    if (iOriginalStartIndex == 0 && m_bWorkingTreePresent)
    {
        trimTailToCount(iStartIndex + iIncomingCount);
        refreshWorkingTreeParent();
        layoutLoadedLines();
    }
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CGraphModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames = CCommitLogModel::roleNames();
    hRoleNames[eGraphCommitLaneRole] = "graphCommitLane";
    hRoleNames[eGraphVerticalsRole] = "graphVerticals";
    hRoleNames[eGraphConnectionsRole] = "graphConnections";
    hRoleNames[eIsWorkingTreeRole] = "isWorkingTree";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

QVariant CGraphModel::data(const QModelIndex& index, int role) const
{
    const CLogLine* pLine = lineAtRow(index.row());

    if (pLine == nullptr)
        return QVariant();

    // Only graph lines ever enter this model: the typed setLines wrapper
    // guards every incoming line, and this runtime check keeps release builds
    // safe if that invariant is ever broken
    const CGraphLine* pGraphLine = qobject_cast<const CGraphLine*>(pLine);

    if (pGraphLine == nullptr)
    {
        qWarning() << "CGraphModel::data() : skipping a non-graph line";
        return CCommitLogModel::data(index, role);
    }

    switch (role)
    {
    case eGraphCommitLaneRole:
        return pGraphLine->graphLane();

    case eGraphVerticalsRole:
        return pGraphLine->graphVerticals();

    case eGraphConnectionsRole:
        return pGraphLine->graphConnections();

    case eIsWorkingTreeRole:
        return pGraphLine->commitId() == workingTreeCommitId();

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
        CGraphLine* pGraphLine = qobject_cast<CGraphLine*>(pLine);

        if (pGraphLine == nullptr)
        {
            qWarning() << "CGraphModel::prepareLines() : skipping a non-graph loaded line";
            continue;
        }

        lLoadedGraphLines << pGraphLine;
    }

    for (CLogLine* pLine : lIncomingLines)
    {
        CGraphLine* pGraphLine = qobject_cast<CGraphLine*>(pLine);

        if (pGraphLine == nullptr)
        {
            qWarning() << "CGraphModel::prepareLines() : skipping a non-graph incoming line";
            continue;
        }

        lIncomingGraphLines << pGraphLine;
    }

    // A refresh with the working tree line lays out the working tree line plus
    // the incoming page alone: the loaded tail holds stale duplicates of the
    // incoming page and must stay out of the layout like the no-ghost refresh
    if (m_bWorkingTreePresent && iStartIndex == 1 && not lLoadedGraphLines.isEmpty() && not lIncomingGraphLines.isEmpty())
    {
        if (lLoadedGraphLines.first()->commitId() == workingTreeCommitId())
        {
            QList<CGraphLine*> lRefreshLines;
            lRefreshLines << lLoadedGraphLines.first() << lIncomingGraphLines;
            CGraphLayout::layoutLines(lRefreshLines);
            return;
        }
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

    const CGraphLine* pExistingGraphLine = qobject_cast<const CGraphLine*>(pExistingLine);
    const CGraphLine* pIncomingGraphLine = qobject_cast<const CGraphLine*>(pIncomingLine);

    if (pExistingGraphLine == nullptr || pIncomingGraphLine == nullptr)
        return false;

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
    if (m_bWorkingTreePresent && iStartIndex > 0)
        iStartIndex -= 1;

    emit requestGraphData(iStartIndex, iCount);
}

//-------------------------------------------------------------------------------------------------

CGraphLine* CGraphModel::createWorkingTreeLine(const QString& sParentId) const
{
    CGraphLine* pLine = new CGraphLine();

    pLine->setCommitId(workingTreeCommitId());

    if (not sParentId.isEmpty() && sParentId != workingTreeCommitId())
        pLine->setParents(QStringList() << sParentId);

    pLine->setDecorations(QStringList());
    pLine->setMessage(tr("No changes"));
    pLine->setMessageIsComplete(true);

    return pLine;
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::setWorkingTreeMessage(const QString& sMessage)
{
    if (not m_bWorkingTreePresent || m_lLines.isEmpty())
        return;

    CGraphLine* pLine = qobject_cast<CGraphLine*>(m_lLines.first());

    if (pLine == nullptr || pLine->commitId() != workingTreeCommitId())
        return;

    if (pLine->message() == sMessage)
        return;

    pLine->setMessage(sMessage);
    pLine->setMessageIsComplete(true);

    QModelIndex tIndex = index(0, 0);

    emit dataChanged(tIndex, tIndex, {CCommitLogModel::eMessageRole, CCommitLogModel::eFullMessageRole});
}

//-------------------------------------------------------------------------------------------------

QString CGraphModel::firstRealCommitId() const
{
    for (const CLogLine* pLine : m_lLines)
    {
        if (pLine != nullptr && not pLine->commitId().isEmpty() && pLine->commitId() != workingTreeCommitId())
            return pLine->commitId();
    }

    return QString();
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::refreshWorkingTreeParent()
{
    if (not m_bWorkingTreePresent || m_lLines.isEmpty())
        return;

    CGraphLine* pWorkingTreeLine = qobject_cast<CGraphLine*>(m_lLines.first());

    if (pWorkingTreeLine == nullptr)
        return;

    QString sParentId = QString();

    for (int iRow = 1; iRow < m_lLines.count(); iRow++)
    {
        const CLogLine* pLine = m_lLines[iRow];

        if (pLine != nullptr && not pLine->commitId().isEmpty() && pLine->commitId() != workingTreeCommitId())
        {
            sParentId = pLine->commitId();
            break;
        }
    }

    if (sParentId.isEmpty())
        pWorkingTreeLine->setParents(QStringList());
    else
        pWorkingTreeLine->setParents(QStringList() << sParentId);
}

//-------------------------------------------------------------------------------------------------

void CGraphModel::layoutLoadedLines()
{
    QList<CGraphLine*> lGraphLines;

    for (CLogLine* pLine : m_lLines)
    {
        CGraphLine* pGraphLine = qobject_cast<CGraphLine*>(pLine);

        if (pGraphLine == nullptr)
        {
            qWarning() << "CGraphModel::layoutLoadedLines() : skipping a non-graph loaded line";
            continue;
        }

        lGraphLines << pGraphLine;
    }

    if (lGraphLines.isEmpty())
        return;

    CGraphLayout::layoutLines(lGraphLines);

    if (m_lLines.count() > 0)
        emit dataChanged(index(0), index(m_lLines.count() - 1), {eGraphCommitLaneRole, eGraphVerticalsRole, eGraphConnectionsRole});
}
