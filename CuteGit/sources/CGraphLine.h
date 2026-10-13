
#pragma once

// Qt
#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QVariantList>

// qt-plus
#include "Macros.h"

// Application
#include "CLogLine.h"

//-------------------------------------------------------------------------------------------------

class CGraphLine : public CLogLine
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(QStringList, l, parents, Parents)
    Q_FAST_PROPERTY(QStringList, l, decorations, Decorations)
    Q_FAST_PROPERTY(int, i, graphLane, GraphLane)
    Q_FAST_PROPERTY(QVariantList, l, graphVerticals, GraphVerticals)
    Q_FAST_PROPERTY(QVariantList, l, graphConnections, GraphConnections)

public:

    CGraphLine()
        : m_iGraphLane(-1)
    {}
};
