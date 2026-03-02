
// Qt
#include <QDebug>

// Application
#include "CLogModelProxy.h"
#include "CLogModel.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

CLogModelProxy::CLogModelProxy(CRepository *pRepository, QObject* parent)
    : QSortFilterProxyModel(parent)
    , m_pRepository(pRepository)
{
}

//-------------------------------------------------------------------------------------------------

bool CLogModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_sTextFilter.isEmpty())
        return true;

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    CLogModel* pModel = dynamic_cast<CLogModel*>(sourceModel());

    if (pModel != nullptr)
    {
        QString sText;

        sText = pModel->data(index, CLogModel::eCommitIdRole).toString();
        if (sText.contains(m_sTextFilter, Qt::CaseInsensitive))
            return true;

        sText = pModel->data(index, CLogModel::eFullMessageRole).toString();
        if (sText.contains(m_sTextFilter, Qt::CaseInsensitive))
            return true;

        sText = pModel->data(index, CLogModel::eAuthorRole).toString();
        if (sText.contains(m_sTextFilter, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

void CLogModelProxy::filterChanged()
{
    invalidateFilter();
}
