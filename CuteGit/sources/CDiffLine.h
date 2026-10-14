
#pragma once

// Qt
#include <QObject>
#include <QDateTime>

// qt-plus
#include "Macros.h"
#include "CXMLNode.h"

// Application
#include "CEnums.h"
#include "CDiffConstants.h"

//-------------------------------------------------------------------------------------------------

class CDiffLine : public QObject
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(CEnums::EDiffOperation, e, operation, Operation)
    Q_FAST_PROPERTY(QString, s, text, Text)
    Q_FAST_PROPERTY(QString, s, commitId, CommitId)
    Q_FAST_PROPERTY(QString, s, fileName, FileName)
    Q_FAST_PROPERTY(int, i, hunkId, HunkId)
    Q_FAST_PROPERTY(int, i, oldLineNumber, OldLineNumber)
    Q_FAST_PROPERTY(int, i, newLineNumber, NewLineNumber)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    CDiffLine() : m_eOperation(CEnums::None), m_iHunkId(CDiffConstants::iInvalidHunkId), m_iOldLineNumber(CDiffConstants::iNoLineNumber), m_iNewLineNumber(CDiffConstants::iNoLineNumber) {}
};
