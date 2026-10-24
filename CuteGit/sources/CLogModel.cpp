
// Application
#include "CLogModel.h"
#include "CEnums.h"

//-------------------------------------------------------------------------------------------------

CLogModel::CLogModel(CRepository *pRepository, QObject* parent)
    : CCommitLogModel(pRepository, parent)
{
}

//-------------------------------------------------------------------------------------------------

CLogModel::~CLogModel()
{
}

//-------------------------------------------------------------------------------------------------

void CLogModel::setLines(CLogLineCollection tCollection)
{
    reconcileLines(tCollection.lines(), tCollection.startIndex(), tCollection.potentialCount());
}

//-------------------------------------------------------------------------------------------------

bool CLogModel::isSameLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const
{
    return isSameCommitContent(pExistingLine, pIncomingLine);
}

//-------------------------------------------------------------------------------------------------

int CLogModel::defaultPageSize() const
{
    return LOG_COUNT_DEFAULT;
}

//-------------------------------------------------------------------------------------------------

void CLogModel::requestLines(int iStartIndex, int iCount)
{
    emit requestLogData(iStartIndex, iCount);
}
