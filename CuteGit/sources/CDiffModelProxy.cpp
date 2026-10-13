
// Qt
#include <QDebug>

// Application
#include "CDiffModelProxy.h"

/***************************************************************************/

CDiffModelProxy::CDiffModelProxy(CRepository *pRepository, QObject* parent)
    : QSortFilterProxyModel(parent)
{
    Q_UNUSED(pRepository)
}

/***************************************************************************/

/*! Sets the source model and tracks its structure changes to keep the file name cache valid
 * \param pSourceModel New source model
 */
void CDiffModelProxy::setSourceModel(QAbstractItemModel* pSourceModel)
{
    if (sourceModel() != nullptr)
        sourceModel()->disconnect(this);

    QSortFilterProxyModel::setSourceModel(pSourceModel);
    invalidateFileNameCache();

    if (pSourceModel != nullptr)
    {
        connect(pSourceModel, &QAbstractItemModel::modelReset, this, &CDiffModelProxy::invalidateFileNameCache);
        connect(pSourceModel, &QAbstractItemModel::layoutChanged, this, &CDiffModelProxy::invalidateFileNameCache);
        connect(pSourceModel, &QAbstractItemModel::rowsInserted, this, &CDiffModelProxy::invalidateFileNameCache);
        connect(pSourceModel, &QAbstractItemModel::rowsRemoved, this, &CDiffModelProxy::invalidateFileNameCache);
        connect(pSourceModel, &QAbstractItemModel::dataChanged, this, &CDiffModelProxy::invalidateFileNameCache);
    }
}

/***************************************************************************/

void CDiffModelProxy::filterChanged()
{
    invalidateFilter();
}

/***************************************************************************/

/*! Returns the source model casted to CDiffModel, nullptr when incompatible
 */
CDiffModel* CDiffModelProxy::sourceDiffModel() const
{
    return dynamic_cast<CDiffModel*>(sourceModel());
}

/***************************************************************************/

/*! Rebuilds the source row to file name cache in a single forward pass
 */
void CDiffModelProxy::rebuildFileNameCache() const
{
    m_CachedFileNames.clear();

    CDiffModel* pModel = sourceDiffModel();

    if (pModel == nullptr || sourceModel() == nullptr)
        return;

    int iRowCount = sourceModel()->rowCount();
    m_CachedFileNames.resize(iRowCount);

    QString sCurrentFileName;

    for (int iRow = 0; iRow < iRowCount; iRow++)
    {
        QModelIndex tIndex = sourceModel()->index(iRow, 0);

        if (pModel->data(tIndex, CDiffModel::eOperationRole).toInt() == CEnums::DiffFileName)
            sCurrentFileName = pModel->data(tIndex, CDiffModel::eTextRole).toString();

        m_CachedFileNames[iRow] = sCurrentFileName;
    }
}

/***************************************************************************/

/*! Clears the source row to file name cache
 */
void CDiffModelProxy::invalidateFileNameCache()
{
    m_CachedFileNames.clear();
}

/***************************************************************************/

/*! Returns the file name owning the source row
 * \param iSourceRow Source row
 */
QString CDiffModelProxy::fileNameForSourceRow(int iSourceRow) const
{
    if (sourceModel() == nullptr)
        return QString();

    if (m_CachedFileNames.size() != sourceModel()->rowCount())
        rebuildFileNameCache();

    if (iSourceRow < 0 || iSourceRow >= m_CachedFileNames.size())
        return QString();

    return m_CachedFileNames.at(iSourceRow);
}

/***************************************************************************/

/*! Returns true when the source row matches the text filter
 * \param iSourceRow Source row
 */
bool CDiffModelProxy::textMatchesSourceRow(int iSourceRow) const
{
    CDiffModel* pModel = sourceDiffModel();

    if (pModel == nullptr)
        return true;

    QModelIndex tIndex = sourceModel()->index(iSourceRow, 0);
    QString sText = pModel->data(tIndex, CDiffModel::eTextRole).toString();

    return sText.contains(m_sTextFilter, Qt::CaseInsensitive);
}

/***************************************************************************/

/*! Returns role names
 */
QHash<int, QByteArray> CDiffModelProxy::roleNames() const
{
    QHash<int, QByteArray> hRoleNames = QSortFilterProxyModel::roleNames();
    hRoleNames[eCollapsedRole] = "collapsed";
    return hRoleNames;
}

/***************************************************************************/

/*! Returns data
 * \param index Proxy index
 * \param role Data role
 */
QVariant CDiffModelProxy::data(const QModelIndex& index, int role) const
{
    if (role == eCollapsedRole)
    {
        QModelIndex tSourceIndex = mapToSource(index);

        if (tSourceIndex.isValid())
            return m_CollapsedFileNames.contains(fileNameForSourceRow(tSourceIndex.row()));

        return false;
    }

    return QSortFilterProxyModel::data(index, role);
}

/***************************************************************************/

bool CDiffModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    CDiffModel* pModel = sourceDiffModel();

    if (pModel == nullptr)
        return true;

    QModelIndex tIndex = sourceModel()->index(sourceRow, 0);
    int iOperation = pModel->data(tIndex, CDiffModel::eOperationRole).toInt();
    bool bIsFileHeader = (iOperation == CEnums::DiffFileName);

    // Text filter overrides collapse: headers stay visible to keep context,
    // matching lines stay visible even inside collapsed files
    if (m_sTextFilter.isEmpty() == false)
    {
        if (bIsFileHeader)
            return true;

        return textMatchesSourceRow(sourceRow);
    }

    if (bIsFileHeader == false)
    {
        QString sFileName = fileNameForSourceRow(sourceRow);

        if (m_CollapsedFileNames.contains(sFileName))
            return false;
    }

    return true;
}

/***************************************************************************/

/*! Collapses or expands the file header
 * \param sFileName File name shown in the header line
 */
void CDiffModelProxy::toggleCollapsedFile(const QString& sFileName)
{
    if (m_CollapsedFileNames.contains(sFileName))
        m_CollapsedFileNames.remove(sFileName);
    else
        m_CollapsedFileNames.insert(sFileName);

    invalidateFilter();

    if (rowCount() > 0)
    {
        QModelIndex tTopLeft = index(0, 0);
        QModelIndex tBottomRight = index(rowCount() - 1, 0);
        emit dataChanged(tTopLeft, tBottomRight, { eCollapsedRole });
    }
}
