
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

    using CLogLineCollection::add;

    //! Appends a graph line, keeping the typed overload visible alongside the base one
    void add(CGraphLine* pLine);
};
