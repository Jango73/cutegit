
// Application
#include "CDiffModel.h"

//-------------------------------------------------------------------------------------------------

CDiffModel::CDiffModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

//-------------------------------------------------------------------------------------------------

CDiffModel::~CDiffModel()
{
    qDeleteAll(m_lLines);
}

//-------------------------------------------------------------------------------------------------

void CDiffModel::setLines(QList<CDiffLine*> lNewLines)
{
    // Identical content keeps loaded rows untouched: no reset, no flicker,
    // the scrollbar and the selection stay where they are
    if (isEqualTo(lNewLines))
    {
        qDeleteAll(lNewLines);
        return;
    }

    int iExistingCount = m_lLines.count();
    int iIncomingCount = lNewLines.count();
    int iCommonCount = qMin(iExistingCount, iIncomingCount);

    for (int iIndex = 0; iIndex < iCommonCount; iIndex++)
    {
        if (isSameLine(m_lLines[iIndex], lNewLines[iIndex]))
        {
            delete lNewLines[iIndex];
        }
        else
        {
            delete m_lLines[iIndex];
            m_lLines[iIndex] = lNewLines[iIndex];
            QModelIndex tIndex = index(iIndex, 0);
            emit dataChanged(tIndex, tIndex);
        }

        lNewLines[iIndex] = nullptr;
    }

    if (iIncomingCount > iExistingCount)
    {
        beginInsertRows(QModelIndex(), iExistingCount, iIncomingCount - 1);

        for (int iIndex = iCommonCount; iIndex < iIncomingCount; iIndex++)
            m_lLines << lNewLines[iIndex];

        endInsertRows();
    }
    else if (iExistingCount > iIncomingCount)
    {
        beginRemoveRows(QModelIndex(), iIncomingCount, iExistingCount - 1);

        while (m_lLines.count() > iIncomingCount)
            delete m_lLines.takeLast();

        endRemoveRows();
    }
}

//-------------------------------------------------------------------------------------------------

bool CDiffModel::isSameLine(const CDiffLine* pExistingLine, const CDiffLine* pIncomingLine)
{
    if (pExistingLine == nullptr || pIncomingLine == nullptr)
        return pExistingLine == pIncomingLine;

    if (pExistingLine->operation() != pIncomingLine->operation())
        return false;

    if (pExistingLine->text() != pIncomingLine->text())
        return false;

    if (pExistingLine->commitId() != pIncomingLine->commitId())
        return false;

    if (pExistingLine->fileName() != pIncomingLine->fileName())
        return false;

    if (pExistingLine->hunkId() != pIncomingLine->hunkId())
        return false;

    if (pExistingLine->oldLineNumber() != pIncomingLine->oldLineNumber())
        return false;

    if (pExistingLine->newLineNumber() != pIncomingLine->newLineNumber())
        return false;

    return true;
}

//-------------------------------------------------------------------------------------------------

bool CDiffModel::isEqualTo(const QList<CDiffLine*>& lNewLines) const
{
    if (m_lLines.count() != lNewLines.count())
        return false;

    for (int iIndex = 0; iIndex < m_lLines.count(); iIndex++)
    {
        if (not isSameLine(m_lLines[iIndex], lNewLines[iIndex]))
            return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CDiffModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames;
    hRoleNames[eOperationRole] = "operation";
    hRoleNames[eTextRole] = "text";
    hRoleNames[eCommitIdRole] = "commitId";
    hRoleNames[eFileNameRole] = "fileName";
    hRoleNames[eHunkIdRole] = "hunkId";
    hRoleNames[eOldLineNumberRole] = "oldLineNumber";
    hRoleNames[eNewLineNumberRole] = "newLineNumber";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

int CDiffModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_lLines.count();
}

//-------------------------------------------------------------------------------------------------

QVariant CDiffModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if (!index.isValid())
        return QVariant();

    if ((row < 0) || (row > (rowCount() - 1)))
        return QVariant();

    switch (role)
    {
    case eOperationRole:
        return m_lLines[row]->operation();

    case eTextRole:
        return m_lLines[row]->text();

    case eCommitIdRole:
        return m_lLines[row]->commitId();

    case eFileNameRole:
        return m_lLines[row]->fileName();

    case eHunkIdRole:
        return m_lLines[row]->hunkId();

    case eOldLineNumberRole:
        return m_lLines[row]->oldLineNumber();

    case eNewLineNumberRole:
        return m_lLines[row]->newLineNumber();
    }

    return QVariant();
}

//-------------------------------------------------------------------------------------------------

CDiffLine* CDiffModel::lineAt(int iRow) const
{
    if ((iRow < 0) || (iRow >= m_lLines.count()))
        return nullptr;

    return m_lLines[iRow];
}

//-------------------------------------------------------------------------------------------------

const QList<CDiffLine*>& CDiffModel::lines() const
{
    return m_lLines;
}
