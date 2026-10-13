
#pragma once

// Qt
#include <QSet>
#include <QSortFilterProxyModel>

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

    //! Returns true when the file header is collapsed
    Q_INVOKABLE bool isFileCollapsed(const QString& sFileName) const;

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const override;

    //! Returns data
    virtual QVariant data(const QModelIndex& index, int role) const override;

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    //! Emitted when the collapsed state changes
    void collapsedChanged();

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:

    //! Returns the file name owning the source row
    QString fileNameForSourceRow(int iSourceRow) const;

    //! Returns true when the source row matches the text filter
    bool textMatchesSourceRow(int iSourceRow) const;

    //! Collapsed file names
    QSet<QString> m_CollapsedFileNames;
};
