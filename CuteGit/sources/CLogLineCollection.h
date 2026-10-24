
#pragma once

// Qt
#include <QObject>

// qt-plus
#include "CLogLine.h"

//-------------------------------------------------------------------------------------------------

class CLogLineCollection : public QObject
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(int, i, potentialCount, PotentialCount)
    Q_FAST_PROPERTY(int, i, startIndex, StartIndex)
    Q_FAST_PROPERTY(QList<CLogLine*>, l, lines, Lines)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CLogLineCollection(QObject *parent = nullptr);

    //!
    CLogLineCollection(const CLogLineCollection& target);

    //! Destructor
    virtual ~CLogLineCollection();

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Appends a line, taking no ownership transfer beyond the collection copy
    virtual void add(CLogLine* pLine);
};
