
// Application
#include "CGraphLineCollection.h"

//-------------------------------------------------------------------------------------------------

CGraphLineCollection::CGraphLineCollection(QObject* parent)
    : QObject(parent)
    , m_iPotentialCount(0)
    , m_iStartIndex(0)
{
}

//-------------------------------------------------------------------------------------------------

CGraphLineCollection::CGraphLineCollection(const CGraphLineCollection& target)
    : QObject()
    , m_iPotentialCount(target.m_iPotentialCount)
    , m_iStartIndex(target.m_iStartIndex)
    , m_lLines(target.m_lLines)
{
}

//-------------------------------------------------------------------------------------------------

CGraphLineCollection::~CGraphLineCollection()
{
}

//-------------------------------------------------------------------------------------------------

void CGraphLineCollection::add(CGraphLine* pLine)
{
    m_lLines << pLine;
}
