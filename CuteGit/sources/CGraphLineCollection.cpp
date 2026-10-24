
// Application
#include "CGraphLineCollection.h"

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

void CGraphLineCollection::add(CGraphLine* pLine)
{
    CLogLineCollection::add(pLine);
}
