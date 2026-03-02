
#pragma once

// Qt
#include <QSortFilterProxyModel>

// qt-plus
#include "Macros.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CController;

// MOC needs complete type for Q_PROPERTY
#include "CController.h"

//-------------------------------------------------------------------------------------------------

class CTreeFileModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(CController*, p, controller, Controller)
    Q_PROPERTY(QModelIndex rootPathIndex READ rootPathIndex NOTIFY rootPathIndexChanged)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor
    //-------------------------------------------------------------------------------------------------

    CTreeFileModelProxy(CController* pController, QObject* parent = nullptr);

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //!
    QModelIndex rootPathIndex() const;

    //!
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    void filterChanged();

    //-------------------------------------------------------------------------------------------------
    // Invokables
    //-------------------------------------------------------------------------------------------------

    //!
    Q_INVOKABLE QString statusForIndex(QModelIndex qIndex);

    //!
    Q_INVOKABLE QString stagedForIndex(QModelIndex qIndex);

    //!
    Q_INVOKABLE QString fullNameForIndex(QModelIndex qIndex);

    //!
    Q_INVOKABLE void handleCurrentIndex(QModelIndex qIndex);

    //!
    Q_INVOKABLE QStringList selectionToFullNameList(QModelIndexList lIndices);

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    void rootPathIndexChanged();

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    QModelIndexList indexListToSource(QModelIndexList lIndices) const;

    //!
    bool hasToBeDisplayed(const QModelIndex qIndex) const;

    //!
    bool statusShown(const QString& sStatus) const;
};
