
#pragma once

// Application
#include "CLogLineCollection.h"
#include "CGraphLine.h"

//-------------------------------------------------------------------------------------------------

class CGraphLineCollection : public CLogLineCollection
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CGraphLineCollection(QObject *parent = nullptr);

    //!
    CGraphLineCollection(const CGraphLineCollection& target);

    //! Destructor
    virtual ~CGraphLineCollection() override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    void add(CGraphLine* pLine);
};
