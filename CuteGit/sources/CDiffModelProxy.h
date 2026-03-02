
#pragma once

// Qt
#include <QSortFilterProxyModel>

// qt-plus
#include "CLogLineCollection.h"

// Application
#include "CDiffModel.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

class CDiffModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(QString, s, textFilter, TextFilter)

public:

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

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

protected:

    //!
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
};
