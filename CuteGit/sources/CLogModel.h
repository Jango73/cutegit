
#pragma once

// Application
#include "CCommitLogModel.h"
#include "CLogLineCollection.h"

//-------------------------------------------------------------------------------------------------

class CLogModel : public CCommitLogModel
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(QString, s, fullSourceName, FullSourceName)
    Q_FAST_PROPERTY(QString, s, relativeSourceName, RelativeSourceName)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CLogModel(CRepository* pRepository, QObject *parent = nullptr);

    //! Destructor
    virtual ~CLogModel() override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Sets some lines
    void setLines(CLogLineCollection tCollection);

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    void requestLogData(int iStartIndex, int iCount);

protected:

    //-------------------------------------------------------------------------------------------------
    // CCommitLogModel hooks
    //-------------------------------------------------------------------------------------------------

    virtual bool isSameLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const override;
    virtual int defaultPageSize() const override;
    virtual void requestLines(int iStartIndex, int iCount) override;
};
