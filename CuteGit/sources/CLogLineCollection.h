
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
    // Constructors and destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CLogLineCollection(QObject *parent = nullptr);

    //! Copy constructor
    CLogLineCollection(const CLogLineCollection& target);

    //! Destructor
    virtual ~CLogLineCollection();

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Appends a line without taking ownership: the collection holds the pointer in a shallow copy
    //! and the model receiving the collection owns every line afterwards, deleting the lines it rejects
    virtual void add(CLogLine* pLine);
};
