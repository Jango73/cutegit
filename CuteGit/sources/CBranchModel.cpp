
// Qt
#include <QDebug>
#include <QMap>

// Application
#include "CBranchModel.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

CBranchModel::CBranchModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

//-------------------------------------------------------------------------------------------------

CBranchModel::~CBranchModel()
{
}

//-------------------------------------------------------------------------------------------------

QList<CLabel*> CBranchModel::labelsForCommit(const QString& sCommitId) const
{
    QList<CLabel*> lReturnValue;

    for (CBranch* pBranch : m_lBranches)
    {
        if (pBranch->commitId() == sCommitId)
            lReturnValue << new CBranch(*pBranch);
    }

    return lReturnValue;
}

//-------------------------------------------------------------------------------------------------

QStringList CBranchModel::branchNames() const
{
    QStringList lReturnValue;

    for (CBranch* pBranch : m_lBranches)
        lReturnValue << pBranch->name();

    return lReturnValue;
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CBranchModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames;
    hRoleNames[eNameRole] = "name";
    hRoleNames[eCommitIdRole]= "commitId";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

int CBranchModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_lBranches.count();
}

//-------------------------------------------------------------------------------------------------

QVariant CBranchModel::data(const QModelIndex& qIndex, int iRole) const
{
    if (qIndex.isValid())
    {
        switch (iRole)
        {
        case eNameRole:
            return m_lBranches[qIndex.row()]->name();

        case eCommitIdRole:
            return m_lBranches[qIndex.row()]->commitId();
        }
    }

    return QVariant();
}

//-------------------------------------------------------------------------------------------------

void CBranchModel::setBranchList(QList<CBranch*> lNewList, CEnums::ELabelType eType)
{
    QMap<QString, QString> mKnownHeadCommits;

    for (CBranch* pBranch : m_lBranches)
        mKnownHeadCommits[pBranch->name()] = pBranch->commitId();

    QList<CBranch*> lFilteredBranches;

    for (CBranch* pBranch : lNewList)
    {
        if (pBranch->type() == eType)
        {
            if (mKnownHeadCommits.contains(pBranch->name()))
                pBranch->setCommitId(mKnownHeadCommits[pBranch->name()]);

            lFilteredBranches << pBranch;
        }
    }

    // Identical content keeps loaded rows untouched: no reset, no flicker,
    // the scrollbar and the selection stay where they are
    if (m_lBranches.count() == lFilteredBranches.count())
    {
        bool bIdentical = true;

        for (int iIndex = 0; iIndex < m_lBranches.count(); iIndex++)
        {
            if (m_lBranches[iIndex]->name() != lFilteredBranches[iIndex]->name()
                    || m_lBranches[iIndex]->commitId() != lFilteredBranches[iIndex]->commitId()
                    || m_lBranches[iIndex]->type() != lFilteredBranches[iIndex]->type())
            {
                bIdentical = false;
                break;
            }
        }

        if (bIdentical)
        {
            qDeleteAll(lFilteredBranches);
            return;
        }
    }

    beginResetModel();

    qDeleteAll(m_lBranches);
    m_lBranches.clear();

    for (CBranch* pBranch : lFilteredBranches)
        m_lBranches << pBranch;

    endResetModel();
}

//-------------------------------------------------------------------------------------------------

QString CBranchModel::setBranchHeadCommit(const QString& sBranch, const QString& sCommitId)
{
    for (int iBranchIndex = 0; iBranchIndex < m_lBranches.count(); iBranchIndex++)
    {
        CBranch* pBranch = m_lBranches[iBranchIndex];

        if (pBranch->name() == sBranch)
        {
            QString sPreviousCommitId = pBranch->commitId();
            pBranch->setCommitId(sCommitId);
            emit dataChanged(index(iBranchIndex), index(iBranchIndex));

            return sPreviousCommitId;
        }
    }

    return QString("");
}
