
// Qt
#include <QDebug>

// Application
#include "CTreeFileModel.h"
#include "CController.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

static inline QString sizeString(const QFileInfo &fi)
{
    if (!fi.isFile())
        return QString();
    const qint64 size = fi.size();
    if (size > 1024 * 1024 * 10)
        return QString::number(size / (1024 * 1024)) + QLatin1Char('M');
    if (size > 1024 * 10)
        return QString::number(size / 1024) + QLatin1Char('K');
    return QString::number(size);
}

//-------------------------------------------------------------------------------------------------

CTreeFileModel::CTreeFileModel(CRepository *pRepository, QObject* parent)
    : QFileSystemModel(parent)
    , m_pRepository(pRepository)
    , m_pFileSystemWatcher(new QFileSystemWatcher(this))
{
    setResolveSymlinks(true);

    connect(this, &QFileSystemModel::rootPathChanged, this, &CTreeFileModel::onRootPathChanged);

    connect(m_pFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &CTreeFileModel::onDirectoryChanged);
    connect(m_pFileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &CTreeFileModel::onFileChanged);
}

//-------------------------------------------------------------------------------------------------

CTreeFileModel::~CTreeFileModel()
{
}

//-------------------------------------------------------------------------------------------------

QModelIndex CTreeFileModel::rootPathIndex() const
{
    return index(rootPath());
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CTreeFileModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames = QFileSystemModel::roleNames();
    hRoleNames[eSizeRole] = "size";
    hRoleNames[eStatusRole] = "status";
    hRoleNames[eStagedRole] = "staged";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

QVariant CTreeFileModel::data(const QModelIndex& qIndex, int iRole) const
{
    if (qIndex.isValid() && iRole >= eSizeRole)
    {
        switch (iRole)
        {

        case eSizeRole:
        {
            return QVariant(sizeString(fileInfo(qIndex)));
        }

        case eStatusRole:
        {
            QString sFileFullName = fileInfo(qIndex).absoluteFilePath();
            CRepoFile* pFile = m_pRepository->fileByFullName(sFileFullName);

            if (pFile != nullptr)
                return pFile->statusToString();

            return CRepoFile::sTokenClean;
        }

        case eStagedRole:
        {
            QString sFileFullName = fileInfo(qIndex).absoluteFilePath();
            CRepoFile* pFile = m_pRepository->fileByFullName(sFileFullName);

            if (pFile != nullptr)
                return pFile->stagedToString();

            return CRepoFile::sTokenUnstaged;
        }

        default:
            break;
        }

    }

    return QFileSystemModel::data(qIndex, iRole);
}

//-------------------------------------------------------------------------------------------------

void CTreeFileModel::handleRepoFilesChanged()
{
    // TODO : fill this
}

//-------------------------------------------------------------------------------------------------

void CTreeFileModel::handleCurrentIndex(QModelIndex qIndex)
{
    QString sFileFullName = fileInfo(qIndex).absoluteFilePath();

    emit currentFileFullName(sFileFullName);
}

//-------------------------------------------------------------------------------------------------

QStringList CTreeFileModel::selectionToFullNameList(QModelIndexList lIndices)
{
    QStringList lFullNames;

    for (QModelIndex qIndex : lIndices)
    {
        lFullNames << fileInfo(qIndex).absoluteFilePath();
    }

    return lFullNames;
}

//-------------------------------------------------------------------------------------------------

void CTreeFileModel::onRootPathChanged(const QString& sNewPath)
{
    Q_UNUSED(sNewPath);

    if (not sNewPath.isEmpty())
    {
        m_pFileSystemWatcher->addPath(sNewPath);
    }
}

//-------------------------------------------------------------------------------------------------

void CTreeFileModel::onFileChanged(const QString& path)
{
    Q_UNUSED(path);

    // Tell the repository to refresh
}

//-------------------------------------------------------------------------------------------------

void CTreeFileModel::onDirectoryChanged(const QString& path)
{
    Q_UNUSED(path);

    // Tell the repository to refresh
    emit shouldRefreshFileStatus();
}
