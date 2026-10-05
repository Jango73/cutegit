
#pragma once

// Qt
#include <QFileSystemModel>
#include <QFileSystemWatcher>
#include <QStringListModel>

// qt-plus
#include "Macros.h"
#include "CXMLNode.h"

// Application
#include "CEnums.h"
#include "CRepoFile.h"
#include "CLogModel.h"
#include "CDiffModel.h"
#include "commands/CCommands.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CRepository;

//-------------------------------------------------------------------------------------------------

class CTreeFileModel : public QFileSystemModel
{
    Q_OBJECT

public:

    enum ERoles
    {
        eSizeRole = Qt::UserRole + 10,
        eStatusRole,
        eStagedRole
    };

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_PROPERTY(QModelIndex rootPathIndex READ rootPathIndex NOTIFY rootPathIndexChanged)

    Q_FAST_PROPERTY(QFileSystemWatcher*, p, fileSystemWatcher, FileSystemWatcher)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CTreeFileModel(CRepository* pRepository, QObject *parent = nullptr);

    //! Destructor
    virtual ~CTreeFileModel() override;

    //-------------------------------------------------------------------------------------------------
    // Setters
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //!
    QModelIndex rootPathIndex() const;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const override;

    //! Returns data
    virtual QVariant data(const QModelIndex& qIndex, int iRole) const override;

    //!
    void handleRepoFilesChanged();

    //!
    void handleCurrentIndex(QModelIndex qIndex);

    //!
    QStringList selectionToFullNameList(QModelIndexList lIndices);

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //! Owning repository (plain member: no Q_PROPERTY to avoid an inclusion cycle)
    CRepository* m_pRepository;

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    //!
    void rootPathIndexChanged();

    //!
    void currentFileFullName(QString sFileFullName);

    //!
    void shouldRefreshFileStatus();

    //-------------------------------------------------------------------------------------------------
    // Slots
    //-------------------------------------------------------------------------------------------------

protected slots:

    //!
    void onRootPathChanged(const QString& sNewPath);

    //!
    void onFileChanged(const QString& path);

    //!
    void onDirectoryChanged(const QString& path);
};
