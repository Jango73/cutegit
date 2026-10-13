
// Qt
#include <QSet>

// Application
#include "CGraphModel.h"
#include "CGraphLayout.h"

//-------------------------------------------------------------------------------------------------

CGraphModel::CGraphModel(CRepository* pRepository, QObject* parent)
    : QAbstractListModel(parent)
    , m_pRepository(pRepository)
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

/*! Replaces the label model of a commit with owned labels */
static void syncLabelModel(QMap<QString, CLabelModel*>& mLabels, const QString& sCommitId, QList<CLabel*> lLabels)
{
    if (mLabels.contains(sCommitId))
        delete mLabels.take(sCommitId);

    if (!lLabels.isEmpty())
    {
        CLabelModel* pModel = new CLabelModel();
        pModel->setLabelList(lLabels);
        mLabels[sCommitId] = pModel;
    }
    else
    {
        qDeleteAll(lLabels);
    }
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

/*! Removes the label model of a commit when no remaining line shows it */
static void removeLabelModelIfOrphaned(QMap<QString, CLabelModel*>& mLabels, const QList<CGraphLine*>& lLines, const QString& sCommitId)
{
    if (sCommitId.isEmpty() || !mLabels.contains(sCommitId))
        return;

    if (!commitIdsOfLines(lLines).contains(sCommitId))
        delete mLabels.take(sCommitId);
}

/***************************************************************************/

/*! Removes label models of commits shown by no remaining line */
static void pruneOrphanedLabels(QMap<QString, CLabelModel*>& mLabels, const QList<CGraphLine*>& lLines)
{
    QSet<QString> sRemainingCommits = commitIdsOfLines(lLines);

    for (auto iIterator = mLabels.begin(); iIterator != mLabels.end();)
    {
        if (!sRemainingCommits.contains(iIterator.key()))
        {
            delete iIterator.value();
            iIterator = mLabels.erase(iIterator);
        }
        else
        {
            iIterator++;
        }
    }
}

/***************************************************************************/

void CGraphModel::setLines(QList<CGraphLine*> lNewLines)
{
    CGraphLayout::layoutLines(lNewLines);

    // Reconcile in place so refreshes without changes cause no view update at all
    int iCommonCount = qMin(m_lLines.count(), lNewLines.count());

    for (int iIndex = 0; iIndex < iCommonCount; iIndex++)
    {
        if (isSameGraphLine(m_lLines[iIndex], lNewLines[iIndex]))
        {
            delete lNewLines[iIndex];
        }
        else
        {
            QString sOldCommitId = m_lLines[iIndex]->commitId();
            CGraphLine* pNewLine = lNewLines[iIndex];
            QString sNewCommitId = pNewLine->commitId();
            QList<CLabel*> lNewLabels = pNewLine->createLabels();
            delete m_lLines[iIndex];
            m_lLines[iIndex] = pNewLine;
            removeLabelModelIfOrphaned(m_mLabels, m_lLines, sOldCommitId);
            syncLabelModel(m_mLabels, sNewCommitId, lNewLabels);
            emit dataChanged(createIndex(iIndex, 0), createIndex(iIndex, 0));
        }
    }

    if (lNewLines.count() > m_lLines.count())
    {
        int iFirstIndex = m_lLines.count();
        int iLastIndex = lNewLines.count() - 1;

        for (int iIndex = iFirstIndex; iIndex <= iLastIndex; iIndex++)
            syncLabelModel(m_mLabels, lNewLines[iIndex]->commitId(), lNewLines[iIndex]->createLabels());

        beginInsertRows(QModelIndex(), iFirstIndex, iLastIndex);

        while (m_lLines.count() < lNewLines.count())
            m_lLines << lNewLines[m_lLines.count()];

        endInsertRows();
    }
    else if (m_lLines.count() > lNewLines.count())
    {
        int iFirstIndex = lNewLines.count();
        int iLastIndex = m_lLines.count() - 1;
        beginRemoveRows(QModelIndex(), iFirstIndex, iLastIndex);

        while (m_lLines.count() > lNewLines.count())
            delete m_lLines.takeLast();

        endRemoveRows();

        pruneOrphanedLabels(m_mLabels, m_lLines);
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

void CGraphModel::setCommitMessage(const QString& sCommitId, const QString& sMessage)
{
    for (int iLineIndex = 0; iLineIndex < m_lLines.count(); iLineIndex++)
    {
        CLogLine* pLine = m_lLines[iLineIndex];

        if (pLine->commitId() == sCommitId)
        {
            // Same full message already shown: skip the notification so refreshes
            // without changes cause no view update
            if (pLine->messageIsComplete() && pLine->message() == sMessage)
                break;

            pLine->setMessage(sMessage);
            pLine->setMessageIsComplete(true);
            emit dataChanged(index(iLineIndex), index(iLineIndex));
            break;
        }
    }
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
