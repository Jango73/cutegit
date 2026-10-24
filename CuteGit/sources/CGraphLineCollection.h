
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
    // Constructors and destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CGraphLineCollection(QObject *parent = nullptr);

    //! Copy constructor
    CGraphLineCollection(const CGraphLineCollection& target);

    //! Destructor
    virtual ~CGraphLineCollection() override;

    //-------------------------------------------------------------------------------------------------
    // Overridden methods
    //-------------------------------------------------------------------------------------------------

    //! Appends a line, keeping only graph lines and deleting the rejected ones
    virtual void add(CLogLine* pLine) Q_DECL_OVERRIDE;
};
