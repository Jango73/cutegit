
#pragma once

// Qt
#include <QObject>

// qt-plus
#include "Macros.h"

// Application
#include "CGraphLine.h"

//-------------------------------------------------------------------------------------------------

class CGraphLineCollection : public QObject
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(int, i, potentialCount, PotentialCount)
    Q_FAST_PROPERTY(int, i, startIndex, StartIndex)
    Q_FAST_PROPERTY(QList<CGraphLine*>, l, lines, Lines)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CGraphLineCollection(QObject *parent = nullptr);

    //!
    CGraphLineCollection(const CGraphLineCollection& target);

    //! Destructor
    virtual ~CGraphLineCollection();

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //!
    void add(CGraphLine* pLine);
};
