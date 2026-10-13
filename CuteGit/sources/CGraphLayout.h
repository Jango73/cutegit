
#pragma once

// Qt
#include <QList>

// Application
#include "CGraphLine.h"

//-------------------------------------------------------------------------------------------------
// Type definitions
//-------------------------------------------------------------------------------------------------

//! Vertical graph segment of one lane for one row
typedef struct
{
    int iLane;
    bool bTopAtDot;
    bool bBottomAtDot;
} SGraphVertical;

//! Dot-height connection between two lanes for one row
typedef struct
{
    int iFromLane;
    int iToLane;
    int iColorLane;
} SGraphConnection;

//-------------------------------------------------------------------------------------------------

//! Assigns stable graph lanes to commit lines from their parent commits.
//! Lanes never shift: a lane keeps its index until freed, then the index is reused.
class CGraphLayout
{
public:

    //! Computes the lane data of all lines in display order (parents below children)
    static void layoutLines(const QList<CGraphLine*>& lLines);
};
