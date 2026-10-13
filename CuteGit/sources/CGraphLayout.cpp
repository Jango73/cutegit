
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

/*! Appends one vertical segment to the line */
static void addVertical(CGraphLine* pLine, int iLane, bool bTopAtDot, bool bBottomAtDot)
{
    QVariantList lVertical;
    lVertical << QVariant(iLane) << QVariant(bTopAtDot) << QVariant(bBottomAtDot);
    QVariantList lVerticals = pLine->graphVerticals();
    lVerticals << QVariant(lVertical);
    pLine->setGraphVerticals(lVerticals);
}

/***************************************************************************/

/*! Appends one dot-height connection to the line */
static void addConnection(CGraphLine* pLine, int iFromLane, int iToLane, int iColorLane)
{
    QVariantList lConnection;
    lConnection << QVariant(iFromLane) << QVariant(iToLane) << QVariant(iColorLane);
    QVariantList lConnections = pLine->graphConnections();
    lConnections << QVariant(lConnection);
    pLine->setGraphConnections(lConnections);
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

        QString sCommitId = pLine->commitId();
        QStringList lParents = pLine->parents();

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

        bool bTopAtDot = lExpectedCommits[iLane].isEmpty();
        pLine->setGraphLane(iLane);

        // Other lanes awaiting this commit join it here
        for (int iOtherLane = 0; iOtherLane < lExpectedCommits.count(); iOtherLane++)
        {
            if (iOtherLane != iLane && lExpectedCommits[iOtherLane] == sCommitId)
            {
                addVertical(pLine, iOtherLane, false, true);
                addConnection(pLine, iOtherLane, iLane, iOtherLane);
                lExpectedCommits[iOtherLane] = QString();
            }
        }

        // First parent continues this lane
        if (!lParents.isEmpty())
            lExpectedCommits[iLane] = lParents.first();
        else
            lExpectedCommits[iLane] = QString();

        // Additional parents fork new lanes, or join lanes already awaiting them
        for (int iParentIndex = 1; iParentIndex < lParents.count(); iParentIndex++)
        {
            QString sParentId = lParents[iParentIndex];
            int iParentLane = laneAwaitingCommit(lExpectedCommits, sParentId);

            if (iParentLane != -1 && iParentLane != iLane)
            {
                addVertical(pLine, iParentLane, false, true);
                addConnection(pLine, iParentLane, iLane, iParentLane);
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
                addVertical(pLine, iNewLane, true, false);
                addConnection(pLine, iLane, iNewLane, iNewLane);
            }
        }

        // Vertical of this lane
        addVertical(pLine, iLane, bTopAtDot, lExpectedCommits[iLane].isEmpty());

        // Pass-through lanes await other commits: draw them full height
        for (int iPassLane = 0; iPassLane < lExpectedCommits.count(); iPassLane++)
        {
            if (iPassLane == iLane)
                continue;

            if (lExpectedCommits[iPassLane].isEmpty())
                continue;

            bool bAlreadyDrawn = false;

            for (const QVariant& vVertical : pLine->graphVerticals())
            {
                if (vVertical.toList().first().toInt() == iPassLane)
                {
                    bAlreadyDrawn = true;
                    break;
                }
            }

            if (!bAlreadyDrawn)
                addVertical(pLine, iPassLane, false, false);
        }
    }
}
