
// Qt
#include <QDebug>

// Application
#include "CFlatFileModel.h"
#include "CController.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

CFlatFileModel::CFlatFileModel(CRepository* pRepository, QObject* parent)
    : QAbstractListModel(parent)
    , m_pRepository(pRepository)
{
}

//-------------------------------------------------------------------------------------------------

CFlatFileModel::~CFlatFileModel()
{
    qDeleteAll(m_lRepoFiles);
}

//-------------------------------------------------------------------------------------------------

QHash<int, QByteArray> CFlatFileModel::roleNames() const
{
    QHash<int, QByteArray> hRoleNames;
    hRoleNames[eFullNameRole] = "fullName";
    hRoleNames[eFileNameRole] = "fileName";
    hRoleNames[eRelativeNameRole] = "relativeName";
    hRoleNames[eSizeRole] = "size";
    hRoleNames[eStatusRole] = "status";
    hRoleNames[eStagedRole] = "staged";
    hRoleNames[eChangingRole] = "changing";
    return hRoleNames;
}

//-------------------------------------------------------------------------------------------------

int CFlatFileModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_lRepoFiles.count();
}

//-------------------------------------------------------------------------------------------------

QVariant CFlatFileModel::data(const QModelIndex& qIndex, int iRole) const
{
    if (qIndex.isValid())
    {
        switch (iRole)
        {
        case eFullNameRole:
            return m_lRepoFiles[qIndex.row()]->fullName();

        case eFileNameRole:
            return m_lRepoFiles[qIndex.row()]->fileName();

        case eRelativeNameRole:
            return m_lRepoFiles[qIndex.row()]->relativeName();

        case eSizeRole:
            return 0;

        case eStatusRole:
            return m_lRepoFiles[qIndex.row()]->statusToString();

        case eStagedRole:
            return m_lRepoFiles[qIndex.row()]->stagedToString();

        case eChangingRole:
            return m_lRepoFiles[qIndex.row()]->changing();
        }
    }

    return QVariant();
}

//-------------------------------------------------------------------------------------------------

bool CFlatFileModel::setData(const QModelIndex& qIndex, const QVariant& vValue, int iRole)
{
    if (qIndex.isValid())
    {
        QVector<int> roles;

        switch (iRole)
        {
        case eStatusRole:
            m_lRepoFiles[qIndex.row()]->setStatus(static_cast<CEnums::ERepoFileStatus>(vValue.toInt()));
            roles << iRole;
            emit dataChanged(qIndex, qIndex, roles);
            break;

        case eStagedRole:
            m_lRepoFiles[qIndex.row()]->setStaged(vValue.toBool());
            roles << iRole;
            emit dataChanged(qIndex, qIndex, roles);
            break;
        }

        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModel::setFileChangingByFullName(const QString& sFullName, bool bChanging)
{
    CRepoFile* pFile = fileByFullName(sFullName);

    if (pFile != nullptr)
    {
        if (not bChanging || pFile->isNotClean())
        {
            pFile->setChanging(bChanging);
            int iIndex = m_lRepoFiles.indexOf(pFile);
            QModelIndex qIndex = createIndex(iIndex, 0);
            emit dataChanged(qIndex, qIndex);
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModel::setFilesChangingByFullName(const QStringList& lFullNames, bool bChanging)
{
    for (QString sFullName : lFullNames)
    {
        setFileChangingByFullName(sFullName, bChanging);
    }
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModel::setAllFilesChanging(bool bChanging)
{
    for (int iIndex = 0; iIndex < m_lRepoFiles.count(); iIndex++)
    {
        CRepoFile* pFile = m_lRepoFiles[iIndex];

        if (not bChanging || pFile->isNotClean())
        {
            pFile->setChanging(bChanging);
            QModelIndex qIndex = createIndex(iIndex, 0);
            emit dataChanged(qIndex, qIndex);
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModel::handleRepoFilesChanged()
{
    CRepoFileList lNewFiles = m_pRepository->repoFiles();

    for (int iNewFileIndex = 0; iNewFileIndex < lNewFiles.count(); iNewFileIndex++)
    {
        CRepoFile* pNewFile = lNewFiles[iNewFileIndex];
        QString sNewKey = pNewFile->fullName();

        if (m_lRepoFiles.containsKey(sNewKey))
        {
            CRepoFile* pExistingFile = m_lRepoFiles.itemByKey(sNewKey);

            if (pExistingFile->staged() != pNewFile->staged() || pExistingFile->status() != pNewFile->status())
            {
                QModelIndex qIndex = createIndex(iNewFileIndex, 0);

                pExistingFile->setStaged(pNewFile->staged());
                pExistingFile->setStatus(pNewFile->status());
                pExistingFile->setChanging(false);

                emit dataChanged(qIndex, qIndex);
            }
        }
        else
        {
            beginInsertRows(QModelIndex(), iNewFileIndex, iNewFileIndex);

            CRepoFile* pNewFileForThis = new CRepoFile(*pNewFile, this);
            m_lRepoFiles.addItem(sNewKey, pNewFileForThis, iNewFileIndex);

            endInsertRows();
        }
    }

    for (int iExistingFileIndex = 0; iExistingFileIndex < m_lRepoFiles.count(); iExistingFileIndex++)
    {
        CRepoFile* pExistingFile = m_lRepoFiles[iExistingFileIndex];
        QString sExistingKey = pExistingFile->fullName();

        if (not lNewFiles.containsKey(sExistingKey))
        {
            beginRemoveRows(QModelIndex(), iExistingFileIndex, iExistingFileIndex);

            delete m_lRepoFiles[iExistingFileIndex];
            m_lRepoFiles.removeItem(sExistingKey);

            endRemoveRows();

            iExistingFileIndex--;
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CFlatFileModel::handleCurrentIndex(QModelIndex qIndex)
{
    if (qIndex.isValid())
    {
        emit currentFileFullName(m_lRepoFiles[qIndex.row()]->fullName());
    }
}

//-------------------------------------------------------------------------------------------------

QStringList CFlatFileModel::selectionToFullNameList(QModelIndexList lIndices)
{
    QStringList lFullNames;

    for (QModelIndex qIndex : lIndices)
    {
        lFullNames << m_lRepoFiles[qIndex.row()]->fullName();
    }

    return lFullNames;
}


//-------------------------------------------------------------------------------------------------

CRepoFile* CFlatFileModel::fileByFullName(const QString& sFullName) const
{
    auto itr = std::find_if(
                m_lRepoFiles.begin(),
                m_lRepoFiles.end(),
                [sFullName](CRepoFile* pFile) { return pFile->fullName() == sFullName; }
            );

    if (itr != m_lRepoFiles.end())
        return *itr;

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
