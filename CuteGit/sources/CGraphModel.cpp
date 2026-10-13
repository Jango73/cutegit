
// Application
#include "CGraphModel.h"
#include "CGraphLayout.h"
#include "CRepository.h"

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

/*! Builds the label list of a graph line from its git decorations */
static QList<CLabel*> labelsFromDecorations(const QStringList& lDecorations)
{
    QList<CLabel*> lReturnValue;

    for (QString sToken : lDecorations)
    {
        sToken = sToken.trimmed();

        if (sToken.isEmpty())
            continue;

        CLabel* pLabel = new CLabel();

        if (sToken.startsWith("HEAD -> "))
        {
            pLabel->setName(sToken.mid(8).trimmed());
            pLabel->setType(CEnums::LocalBranchLabel);
        }
        else if (sToken.startsWith("tag: "))
        {
            pLabel->setName(sToken.mid(5).trimmed());
            pLabel->setType(CEnums::TagLabel);
        }
        else if (sToken == "HEAD")
        {
            pLabel->setName(sToken);
            pLabel->setType(CEnums::LocalBranchLabel);
        }
        else if (sToken.contains(PATH_SEP))
        {
            pLabel->setName(sToken);
            pLabel->setType(CEnums::RemoteBranchLabel);
        }
        else
        {
            pLabel->setName(sToken);
            pLabel->setType(CEnums::LocalBranchLabel);
        }

        lReturnValue << pLabel;
    }

    return lReturnValue;
}

/***************************************************************************/

/*! Replaces the label model of a commit with its decorations */
static void syncLabelModel(QMap<QString, CLabelModel*>& mLabels, const QString& sCommitId, const QStringList& lDecorations)
{
    if (mLabels.contains(sCommitId))
    {
        delete mLabels.take(sCommitId);
    }

    QList<CLabel*> lLabels = labelsFromDecorations(lDecorations);

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

/*! Removes the label model of a commit when no remaining line shows it */
static void removeLabelModel(QMap<QString, CLabelModel*>& mLabels, const QList<CGraphLine*>& lLines, const QString& sCommitId)
{
    for (const CGraphLine* pLine : lLines)
    {
        if (pLine != nullptr && pLine->commitId() == sCommitId)
            return;
    }

    if (mLabels.contains(sCommitId))
        delete mLabels.take(sCommitId);
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
            QString sNewCommitId = lNewLines[iIndex]->commitId();
            QStringList lNewDecorations = lNewLines[iIndex]->decorations();
            delete m_lLines[iIndex];
            m_lLines[iIndex] = lNewLines[iIndex];
            removeLabelModel(m_mLabels, m_lLines, sOldCommitId);
            syncLabelModel(m_mLabels, sNewCommitId, lNewDecorations);
            emit dataChanged(createIndex(iIndex, 0), createIndex(iIndex, 0));
        }
    }

    if (lNewLines.count() > m_lLines.count())
    {
        int iFirstIndex = m_lLines.count();
        int iLastIndex = lNewLines.count() - 1;

        for (int iIndex = iFirstIndex; iIndex <= iLastIndex; iIndex++)
            syncLabelModel(m_mLabels, lNewLines[iIndex]->commitId(), lNewLines[iIndex]->decorations());

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

        QStringList lRemainingCommits;

        for (const CGraphLine* pLine : m_lLines)
        {
            if (pLine != nullptr && !lRemainingCommits.contains(pLine->commitId()))
                lRemainingCommits << pLine->commitId();
        }

        for (auto iIterator = m_mLabels.begin(); iIterator != m_mLabels.end();)
        {
            if (!lRemainingCommits.contains(iIterator.key()))
            {
                delete iIterator.value();
                iIterator = m_mLabels.erase(iIterator);
            }
            else
            {
                iIterator++;
            }
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
