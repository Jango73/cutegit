
#pragma once

// Qt
#include <QAbstractListModel>

// qt-plus
#include "Macros.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CRepository;

//-------------------------------------------------------------------------------------------------

class CRepositoryModel : public QAbstractListModel
{
    Q_OBJECT

    Q_FAST_PROPERTY(int, i, count, Count)

public:

    enum ERoles
    {
        eRepositoryRole = Qt::UserRole + 1
    };

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CRepositoryModel(QObject *parent = nullptr);

    //! Destructor
    virtual ~CRepositoryModel();

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //!
    QList<CRepository*> repositories() const;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const;

    //! Returns row count
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    //! Returns data
    virtual QVariant data(const QModelIndex& index, int role) const;

    //!
    void addRepository(CRepository* pRepository);

    //!
    void removeRepository(int iRepositoryIndex);

    //!
    bool hasRepository(const QString& sPath);

    //!
    void fileFiltersChanged();

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

protected:

    //! List of repositories
    QList<CRepository*>  m_lRepositories;
};
