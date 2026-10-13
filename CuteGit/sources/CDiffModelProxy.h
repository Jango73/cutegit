
#pragma once

// Qt
#include <QSet>
#include <QSortFilterProxyModel>
#include <QVector>

// qt-plus
#include "CLogLineCollection.h"

// Application
#include "CDiffModel.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CRepository;

//-------------------------------------------------------------------------------------------------

class CDiffModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(QString, s, textFilter, TextFilter)

public:

    enum ERoles
    {
        eCollapsedRole = Qt::UserRole + 10
    };

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CDiffModelProxy(CRepository* pRepository, QObject *parent = nullptr);

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    void filterChanged();

    //! Collapses or expands the file header
    Q_INVOKABLE void toggleCollapsedFile(const QString& sFileName);

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const override;

    //! Returns data
    virtual QVariant data(const QModelIndex& index, int role) const override;

    //! Sets the source model
    virtual void setSourceModel(QAbstractItemModel* pSourceModel) override;

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:

    //! Returns the source model casted to CDiffModel, nullptr when incompatible
    CDiffModel* sourceDiffModel() const;

    //! Rebuilds the source row to file name cache in a single forward pass
    void rebuildFileNameCache() const;

    //! Clears the source row to file name cache
    void invalidateFileNameCache();

    //! Returns the file name owning the source row
    QString fileNameForSourceRow(int iSourceRow) const;

    //! Returns true when the source row matches the text filter
    bool textMatchesSourceRow(int iSourceRow) const;

    //! Collapsed file names
    QSet<QString> m_CollapsedFileNames;

    //! File name owning each source row, rebuilt on demand
    mutable QVector<QString> m_CachedFileNames;
};
