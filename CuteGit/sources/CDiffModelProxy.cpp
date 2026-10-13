
// Qt
#include <QDebug>

// Application
#include "CDiffModelProxy.h"

//-------------------------------------------------------------------------------------------------

CDiffModelProxy::CDiffModelProxy(CRepository *pRepository, QObject* parent)
    : QSortFilterProxyModel(parent)
{
    Q_UNUSED(pRepository)
}

//-------------------------------------------------------------------------------------------------

void CDiffModelProxy::filterChanged()
{
    invalidateFilter();
}

//*************************************************************************

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

    emit collapsedChanged();
}

//*************************************************************************

/*! Returns true when the file header is collapsed
 * \param sFileName File name shown in the header line
 */
bool CDiffModelProxy::isFileCollapsed(const QString& sFileName) const
{
    return m_CollapsedFileNames.contains(sFileName);
}

//*************************************************************************

/*! Returns role names
 */
QHash<int, QByteArray> CDiffModelProxy::roleNames() const
{
    QHash<int, QByteArray> hRoleNames = QSortFilterProxyModel::roleNames();
    hRoleNames[eCollapsedRole] = "collapsed";
    return hRoleNames;
}

//*************************************************************************

/*! Returns data
 * \param index Proxy index
 * \param role Data role
 */
QVariant CDiffModelProxy::data(const QModelIndex& index, int role) const
{
    if (role == eCollapsedRole)
    {
        QModelIndex tSourceIndex = mapToSource(index);

        if (tSourceIndex.isValid() && sourceModel() != nullptr)
        {
            CDiffModel* pModel = dynamic_cast<CDiffModel*>(sourceModel());

            if (pModel != nullptr)
            {
                int iOperation = pModel->data(tSourceIndex, CDiffModel::eOperationRole).toInt();

                QString sFileName;

                if (iOperation == CEnums::DiffFileName)
                    sFileName = pModel->data(tSourceIndex, CDiffModel::eTextRole).toString();
                else
                    sFileName = fileNameForSourceRow(tSourceIndex.row());

                return m_CollapsedFileNames.contains(sFileName);
            }
        }

        return false;
    }

    return QSortFilterProxyModel::data(index, role);
}

//-------------------------------------------------------------------------------------------------

bool CDiffModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    CDiffModel* pModel = dynamic_cast<CDiffModel*>(sourceModel());

    if (pModel == nullptr)
        return true;

    QModelIndex tIndex = sourceModel()->index(sourceRow, 0);
    int iOperation = pModel->data(tIndex, CDiffModel::eOperationRole).toInt();
    bool bIsFileHeader = (iOperation == CEnums::DiffFileName);

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

//*************************************************************************

/*! Returns the file name owning the source row
 * \param iSourceRow Source row
 */
QString CDiffModelProxy::fileNameForSourceRow(int iSourceRow) const
{
    CDiffModel* pModel = dynamic_cast<CDiffModel*>(sourceModel());

    if (pModel == nullptr)
        return QString();

    for (int iRow = iSourceRow; iRow >= 0; iRow--)
    {
        QModelIndex tIndex = sourceModel()->index(iRow, 0);

        if (pModel->data(tIndex, CDiffModel::eOperationRole).toInt() == CEnums::DiffFileName)
            return pModel->data(tIndex, CDiffModel::eTextRole).toString();
    }

    return QString();
}

//*************************************************************************

/*! Returns true when the source row matches the text filter
 * \param iSourceRow Source row
 */
bool CDiffModelProxy::textMatchesSourceRow(int iSourceRow) const
{
    CDiffModel* pModel = dynamic_cast<CDiffModel*>(sourceModel());

    if (pModel == nullptr)
        return true;

    QModelIndex tIndex = sourceModel()->index(iSourceRow, 0);
    QString sText = pModel->data(tIndex, CDiffModel::eTextRole).toString();

    return sText.contains(m_sTextFilter, Qt::CaseInsensitive);
}
