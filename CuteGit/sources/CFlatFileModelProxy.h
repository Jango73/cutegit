
#pragma once

// Qt
#include <QSortFilterProxyModel>

// qt-plus
#include "Macros.h"

// Application
#include "CEnums.h"
#include "CRepoFile.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CController;

//-------------------------------------------------------------------------------------------------

class CFlatFileModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY_NO_SET_IMPL(QString, s, nameFilter, NameFilter)
    Q_FAST_PROPERTY_NO_SET_IMPL(CEnums::ESortField, e, sortField, SortField)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, sortDirection, SortDirection)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    CFlatFileModelProxy(CController* pController, QObject* parent = nullptr);

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    void filterChanged();

    //-------------------------------------------------------------------------------------------------
    // Invokables
    //-------------------------------------------------------------------------------------------------

    //!
    Q_INVOKABLE void handleCurrentIndex(QModelIndex qIndex);

    //!
    Q_INVOKABLE QStringList selectionToFullNameList(QModelIndexList lIndices);

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

    //!
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    //!
    QModelIndexList indexListToSource(QModelIndexList lIndices) const;

    //!
    bool statusShown(const QString& sStatus) const;

    //!
    bool nameShown(const QString& sName) const;

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

protected:

    //! Controlling controller (plain member: no Q_PROPERTY to avoid an inclusion cycle)
    CController* m_pController;
};
