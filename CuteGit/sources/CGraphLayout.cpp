
// Application
#include "CGraphLayout.h"

//-------------------------------------------------------------------------------------------------

//! Returns the lane awaiting the commit, or -1 when no lane awaits it
static int laneAwaitingCommit(const QStringList& lExpectedCommits, const QString& sCommitId)
{
    return lExpectedCommits.indexOf(sCommitId);
}

/***************************************************************************/

/*! Returns a free lane, or -1 when all lanes are occupied */
static int freeLane(const QStringList& lExpectedCommits)
{
    return lExpectedCommits.indexOf(QString());
}

/***************************************************************************/

/*! Allocates the lane of a commit, reusing a free lane when possible */
static int assignLane(QStringList& lExpectedCommits, const QString& sCommitId)
{
    int iLane = -1;

    if (!sCommitId.isEmpty())
        iLane = laneAwaitingCommit(lExpectedCommits, sCommitId);

    if (iLane == -1)
    {
        iLane = freeLane(lExpectedCommits);

        if (iLane == -1)
        {
            iLane = lExpectedCommits.count();
            lExpectedCommits << QString();
        }
    }

    return iLane;
}

/***************************************************************************/

/*! Returns true when the lane already holds a vertical segment */
static bool hasVertical(const QList<SGraphVertical>& lVerticals, int iLane)
{
    for (const SGraphVertical& tVertical : lVerticals)
    {
        if (tVertical.iLane == iLane)
            return true;
    }

    return false;
}

/***************************************************************************/

/*! Converts vertical segments to the variant form exposed to QML */
static QVariantList verticalsToVariant(const QList<SGraphVertical>& lVerticals)
{
    QVariantList lReturnValue;

    for (const SGraphVertical& tVertical : lVerticals)
    {
        QVariantList lVertical;
        lVertical << QVariant(tVertical.iLane) << QVariant(tVertical.bTopAtDot) << QVariant(tVertical.bBottomAtDot);
        lReturnValue << QVariant(lVertical);
    }

    return lReturnValue;
}

/***************************************************************************/

/*! Converts dot-height connections to the variant form exposed to QML */
static QVariantList connectionsToVariant(const QList<SGraphConnection>& lConnections)
{
    QVariantList lReturnValue;

    for (const SGraphConnection& tConnection : lConnections)
    {
        QVariantList lConnection;
        lConnection << QVariant(tConnection.iFromLane) << QVariant(tConnection.iToLane) << QVariant(tConnection.iColorLane);
        lReturnValue << QVariant(lConnection);
    }

    return lReturnValue;
}

/***************************************************************************/

/*! Joins other lanes awaiting the same commit into the commit lane */
static void joinMergingLanes(QStringList& lExpectedCommits, const QString& sCommitId, int iLane, QList<SGraphVertical>& lVerticals, QList<SGraphConnection>& lConnections)
{
    for (int iOtherLane = 0; iOtherLane < lExpectedCommits.count(); iOtherLane++)
    {
        if (iOtherLane != iLane && lExpectedCommits[iOtherLane] == sCommitId)
        {
            lVerticals << SGraphVertical{iOtherLane, false, true};
            lConnections << SGraphConnection{iOtherLane, iLane, iOtherLane};
            lExpectedCommits[iOtherLane] = QString();
        }
    }
}

/***************************************************************************/

/*! Makes the first parent continue the commit lane */
static void continueFirstParent(QStringList& lExpectedCommits, const QStringList& lParents, int iLane)
{
    if (!lParents.isEmpty())
        lExpectedCommits[iLane] = lParents.first();
    else
        lExpectedCommits[iLane] = QString();
}

/***************************************************************************/

/*! Forks new lanes for additional parents, or joins lanes already awaiting them */
static void forkAdditionalParents(QStringList& lExpectedCommits, const QStringList& lParents, int iLane, QList<SGraphVertical>& lVerticals, QList<SGraphConnection>& lConnections)
{
    for (int iParentIndex = 1; iParentIndex < lParents.count(); iParentIndex++)
    {
        QString sParentId = lParents[iParentIndex];
        int iParentLane = laneAwaitingCommit(lExpectedCommits, sParentId);

        if (iParentLane != -1 && iParentLane != iLane)
        {
            lVerticals << SGraphVertical{iParentLane, false, true};
            lConnections << SGraphConnection{iParentLane, iLane, iParentLane};
            lExpectedCommits[iParentLane] = QString();
        }
        else if (iParentLane == -1)
        {
            int iNewLane = freeLane(lExpectedCommits);

            if (iNewLane == -1)
            {
                iNewLane = lExpectedCommits.count();
                lExpectedCommits << QString();
            }

            lExpectedCommits[iNewLane] = sParentId;
            lVerticals << SGraphVertical{iNewLane, true, false};
            lConnections << SGraphConnection{iLane, iNewLane, iNewLane};
        }
    }
}

/***************************************************************************/

/*! Draws full height verticals for pass-through lanes awaiting other commits */
static void addPassThroughVerticals(const QStringList& lExpectedCommits, int iLane, QList<SGraphVertical>& lVerticals)
{
    for (int iPassLane = 0; iPassLane < lExpectedCommits.count(); iPassLane++)
    {
        if (iPassLane == iLane)
            continue;

        if (lExpectedCommits[iPassLane].isEmpty())
            continue;

        if (!hasVertical(lVerticals, iPassLane))
            lVerticals << SGraphVertical{iPassLane, false, false};
    }
}

/***************************************************************************/

/*! Computes lane, verticals and connections of a single line */
static void layoutSingleLine(QStringList& lExpectedCommits, CGraphLine* pLine)
{
    QString sCommitId = pLine->commitId();
    QStringList lParents = pLine->parents();

    int iLane = assignLane(lExpectedCommits, sCommitId);
    bool bTopAtDot = lExpectedCommits[iLane].isEmpty();
    pLine->setGraphLane(iLane);

    QList<SGraphVertical> lVerticals;
    QList<SGraphConnection> lConnections;

    joinMergingLanes(lExpectedCommits, sCommitId, iLane, lVerticals, lConnections);
    continueFirstParent(lExpectedCommits, lParents, iLane);
    forkAdditionalParents(lExpectedCommits, lParents, iLane, lVerticals, lConnections);

    lVerticals << SGraphVertical{iLane, bTopAtDot, lExpectedCommits[iLane].isEmpty()};
    addPassThroughVerticals(lExpectedCommits, iLane, lVerticals);

    pLine->setGraphVerticals(verticalsToVariant(lVerticals));
    pLine->setGraphConnections(connectionsToVariant(lConnections));
}

/***************************************************************************/

/*! Computes the lane data of all lines in display order (parents below children) */
void CGraphLayout::layoutLines(const QList<CGraphLine*>& lLines)
{
    QStringList lExpectedCommits;

    for (CGraphLine* pLine : lLines)
    {
        if (pLine == nullptr)
            continue;

        layoutSingleLine(lExpectedCommits, pLine);
    }
}
