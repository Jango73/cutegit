
#pragma once

// Qt
#include <QList>
#include <QSet>
#include <QString>
#include <QVariantList>

// Application
#include "CDiffModel.h"
#include "CDiffModelProxy.h"

//-------------------------------------------------------------------------------------------------

class CPatchBuilder
{
public:

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Maps proxy rows coming from QML to source rows of a diff model
    static QSet<int> sourceRowsFromProxyRows(CDiffModelProxy* pProxy, CDiffModel* pModel, const QVariantList& lProxyRows);

    //! Returns the source rows holding added or deleted lines for a hunk
    static QList<int> selectableSourceRowsForHunk(CDiffModel* pModel, int iHunkId);

    //! Builds a unified patch for selected added or deleted source rows
    static QString patchFromSelection(CDiffModel* pModel, const QSet<int>& sSelectedSourceRows, const QString& sRelativeName);
};
