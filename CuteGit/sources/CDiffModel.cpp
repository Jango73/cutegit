
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
    beginResetModel();

    qDeleteAll(m_lLines);
    m_lLines.clear();

    for (CDiffLine* pLine : lNewLines)
        m_lLines << pLine;

    endResetModel();
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
