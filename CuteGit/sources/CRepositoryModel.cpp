
// Application
#include "CRepositoryModel.h"
#include "CFlatFileModelProxy.h"

//-------------------------------------------------------------------------------------------------

CRepositoryModel::CRepositoryModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_iCount(0)
{
}

//-------------------------------------------------------------------------------------------------

CRepositoryModel::~CRepositoryModel()
{
    qDeleteAll(m_lRepositories);
}

//-------------------------------------------------------------------------------------------------

QList<CRepository*> CRepositoryModel::repositories() const
{
    return m_lRepositories;
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CRepositoryModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames;
    hRoleNames[eRepositoryRole] = "repository";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

int CRepositoryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_lRepositories.count();
}

//-------------------------------------------------------------------------------------------------

QVariant CRepositoryModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        switch (role)
        {
        case eRepositoryRole:
            QVariant vReturnValue;
            vReturnValue.setValue(m_lRepositories[index.row()]);
            return vReturnValue;
        }
    }

    return QVariant();
}

//-------------------------------------------------------------------------------------------------

void CRepositoryModel::addRepository(CRepository* pRepository)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_lRepositories << pRepository;
    endInsertRows();

    setCount(m_lRepositories.count());
}

//-------------------------------------------------------------------------------------------------

void CRepositoryModel::removeRepository(int iRepositoryIndex)
{
    if (iRepositoryIndex < rowCount())
    {
        beginRemoveRows(QModelIndex(), iRepositoryIndex, iRepositoryIndex);
        delete m_lRepositories[iRepositoryIndex];
        m_lRepositories.removeAt(iRepositoryIndex);
        endRemoveRows();

        setCount(m_lRepositories.count());
    }
}

//-------------------------------------------------------------------------------------------------

bool CRepositoryModel::hasRepository(const QString& sPath)
{
    for (CRepository* pRepository : m_lRepositories)
    {
        if (pRepository->repositoryPath() == sPath)
            return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

void CRepositoryModel::fileFiltersChanged()
{
    for (CRepository* pRepository : m_lRepositories)
    {
        if (pRepository != nullptr)
        {
            pRepository->flatFileModelProxy()->filterChanged();
        }
    }
}
