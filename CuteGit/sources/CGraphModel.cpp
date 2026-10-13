
// Application
#include "CGraphModel.h"
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

//-------------------------------------------------------------------------------------------------

void CGraphModel::setLines(QList<CGraphLine*> lNewLines)
{
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
            delete m_lLines[iIndex];
            m_lLines[iIndex] = lNewLines[iIndex];
            emit dataChanged(createIndex(iIndex, 0), createIndex(iIndex, 0));
        }
    }

    if (lNewLines.count() > m_lLines.count())
    {
        int iFirstIndex = m_lLines.count();
        int iLastIndex = lNewLines.count() - 1;
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
    hRoleNames[eGraphSymbolRole] = "graphSymbol";
    hRoleNames[eMessageIsCompleteRole] = "messageIsComplete";
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
        return QVariant();
    }

    case eGraphSymbolRole:
        return m_lLines[row]->graphSymbol();

    case eMessageIsCompleteRole:
        return m_lLines[row]->messageIsComplete();
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

    if (pExistingLine->graphSymbol() != pIncomingLine->graphSymbol())
        return false;

    if (pExistingLine->commitId().isEmpty())
        return true;

    if (pExistingLine->author() != pIncomingLine->author())
        return false;

    if (pExistingLine->date() != pIncomingLine->date())
        return false;

    // Existing lines may hold the full message while incoming lines hold the subject:
    // compare the first line only
    return pExistingLine->message().split(NEW_LINE).first() == pIncomingLine->message().split(NEW_LINE).first();
}
