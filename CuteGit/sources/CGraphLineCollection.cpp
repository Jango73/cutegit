
// Application
#include "CGraphLineCollection.h"

// Qt
#include <QDebug>

//-------------------------------------------------------------------------------------------------

CGraphLineCollection::CGraphLineCollection(QObject* parent)
    : CLogLineCollection(parent)
{
}

//-------------------------------------------------------------------------------------------------

CGraphLineCollection::CGraphLineCollection(const CGraphLineCollection& target)
    : CLogLineCollection(target)
{
}

//-------------------------------------------------------------------------------------------------

CGraphLineCollection::~CGraphLineCollection()
{
}

//-------------------------------------------------------------------------------------------------

void CGraphLineCollection::add(CLogLine* pLine)
{
    if (qobject_cast<CGraphLine*>(pLine) == nullptr)
    {
        qWarning() << "CGraphLineCollection::add() : skipping a non-graph line";
        delete pLine;
        return;
    }

    CLogLineCollection::add(pLine);
}
