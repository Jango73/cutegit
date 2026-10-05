
// Qt
#include <QDebug>

// Application
#include "CFlatFileModelProxy.h"
#include "CFlatFileModel.h"
#include "CController.h"

//-------------------------------------------------------------------------------------------------

CFlatFileModelProxy::CFlatFileModelProxy(CController* pController, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_pController(pController)
    , m_eSortField(CEnums::SortFullName)
    , m_bSortDirection(false)
{
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModelProxy::setNameFilter(QString sNameFilter)
{
    m_sNameFilter = sNameFilter;
    invalidateFilter();
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModelProxy::setSortField(CEnums::ESortField eField)
{
    m_eSortField = eField;
    invalidate();
    sort(0);
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModelProxy::setSortDirection(bool bDirection)
{
    m_bSortDirection = bDirection;
    invalidate();
    sort(0);
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModelProxy::filterChanged()
{
    invalidateFilter();
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModelProxy::handleCurrentIndex(QModelIndex qIndex)
{
    if (qIndex.isValid())
    {
        CFlatFileModel* pModel = dynamic_cast<CFlatFileModel*>(sourceModel());
        if (pModel != nullptr)
            pModel->handleCurrentIndex(mapToSource(qIndex));
    }
}

//-------------------------------------------------------------------------------------------------

QStringList CFlatFileModelProxy::selectionToFullNameList(QModelIndexList lIndices)
{
    QStringList lFullNames;

    CFlatFileModel* pModel = dynamic_cast<CFlatFileModel*>(sourceModel());
    if (pModel != nullptr)
        return pModel->selectionToFullNameList(indexListToSource(lIndices));

    return lFullNames;
}

//-------------------------------------------------------------------------------------------------

bool CFlatFileModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    CFlatFileModel* pModel = dynamic_cast<CFlatFileModel*>(sourceModel());

    if (pModel != nullptr)
    {
        bool bShow = statusShown(pModel->data(index, CFlatFileModel::eStatusRole).toString());

        if (bShow)
            bShow = nameShown(pModel->data(index, CFlatFileModel::eFullNameRole).toString());

        return bShow;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool CFlatFileModelProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    switch (m_eSortField) {
    case CEnums::SortFileName:
    {
        QString leftName = sourceModel()->data(left, CFlatFileModel::eFileNameRole).toString();
        QString rightName = sourceModel()->data(right, CFlatFileModel::eFileNameRole).toString();
        return m_bSortDirection ? leftName > rightName : leftName < rightName;
    }
    case CEnums::SortFullName:
    {
        QString leftName = sourceModel()->data(left, CFlatFileModel::eFullNameRole).toString();
        QString rightName = sourceModel()->data(right, CFlatFileModel::eFullNameRole).toString();
        return m_bSortDirection ? leftName > rightName : leftName < rightName;
    }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

QModelIndexList CFlatFileModelProxy::indexListToSource(QModelIndexList lIndices) const
{
    QModelIndexList targetIndices;

    for (QModelIndex qIndex : lIndices)
    {
        if (qIndex.isValid())
        {
            targetIndices << mapToSource(qIndex);
        }
    }

    return targetIndices;
}

//-------------------------------------------------------------------------------------------------

bool CFlatFileModelProxy::statusShown(const QString& sStatus) const
{
    if (m_pController->showClean() && sStatus == CRepoFile::sTokenClean)
        return true;

    if (m_pController->showAdded() && sStatus == CRepoFile::sTokenAdded)
        return true;

    if (m_pController->showModified() && sStatus == CRepoFile::sTokenModified)
        return true;

    if (m_pController->showRenamed() && sStatus == CRepoFile::sTokenRenamed)
        return true;

    if (m_pController->showDeleted() && sStatus == CRepoFile::sTokenDeleted)
        return true;

    if (m_pController->showMissing() && sStatus == CRepoFile::sTokenMissing)
        return true;

    if (m_pController->showUntracked() && sStatus == CRepoFile::sTokenUntracked)
        return true;

    if (m_pController->showIgnored() && sStatus == CRepoFile::sTokenIgnored)
        return true;

    return false;
}

//-------------------------------------------------------------------------------------------------

bool CFlatFileModelProxy::nameShown(const QString& sName) const
{
    if (m_sNameFilter.isEmpty())
        return true;

    return sName.contains(m_sNameFilter, Qt::CaseInsensitive);
}
