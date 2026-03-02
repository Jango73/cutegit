
// Qt
#include <QDebug>

// Application
#include "CDiffModelProxy.h"

//-------------------------------------------------------------------------------------------------

CDiffModelProxy::CDiffModelProxy(CRepository *pRepository, QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

//-------------------------------------------------------------------------------------------------

void CDiffModelProxy::filterChanged()
{
    invalidateFilter();
}

//-------------------------------------------------------------------------------------------------

bool CDiffModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_sTextFilter.isEmpty())
        return true;

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    CDiffModel* pModel = dynamic_cast<CDiffModel*>(sourceModel());

    if (pModel != nullptr)
    {
        QString sText;

        sText = pModel->data(index, CDiffModel::eTextRole).toString();
        if (sText.contains(m_sTextFilter, Qt::CaseInsensitive))
            return true;
    }

    return false;
}
