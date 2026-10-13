
#pragma once

// Qt
#include <QList>

// Application
#include "CGraphLine.h"

//-------------------------------------------------------------------------------------------------

//! Assigns stable graph lanes to commit lines from their parent commits.
//! Lanes never shift: a lane keeps its index until freed, then the index is reused.
class CGraphLayout
{
public:

    //! Computes the lane data of all lines in display order (parents below children)
    static void layoutLines(const QList<CGraphLine*>& lLines);
};
