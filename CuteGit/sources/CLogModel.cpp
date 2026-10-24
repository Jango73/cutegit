
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
    CCommitLogModel::setLines(tCollection.lines(), tCollection.startIndex(), tCollection.potentialCount());
}

//-------------------------------------------------------------------------------------------------

bool CLogModel::isSameLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const
{
    if (pExistingLine == nullptr || pIncomingLine == nullptr)
        return pExistingLine == pIncomingLine;

    if (pExistingLine->commitId() != pIncomingLine->commitId())
        return false;

    if (pExistingLine->author() != pIncomingLine->author())
        return false;

    if (pExistingLine->date() != pIncomingLine->date())
        return false;

    // Existing lines may hold the full message while incoming lines hold the subject:
    // compare the first line only
    return pExistingLine->message().split(NEW_LINE).first() == pIncomingLine->message().split(NEW_LINE).first();
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
