
#pragma once

// Application
#include "CCommitLogModel.h"
#include "CGraphLineCollection.h"

//-------------------------------------------------------------------------------------------------

class CGraphModel : public CCommitLogModel
{
    Q_OBJECT

public:

    enum ERoles
    {
        eGraphCommitLaneRole = Qt::UserRole + 0x0A,
        eGraphVerticalsRole,
        eGraphConnectionsRole,
    };

public:

    //-------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CGraphModel(CRepository* pRepository, QObject *parent = nullptr);

    //! Destructor
    virtual ~CGraphModel() override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Lays out graph lines, then reconciles loaded rows with them without flicker
    void setLines(CGraphLineCollection tCollection);

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const override;

    //! Returns data
    virtual QVariant data(const QModelIndex& index, int role) const override;

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    void requestGraphData(int iStartIndex, int iCount);

protected:

    //-------------------------------------------------------------------------------------------------
    // Overridden methods
    //-------------------------------------------------------------------------------------------------

    virtual void prepareLines(QList<CLogLine*>& lLoadedLines, QList<CLogLine*>& lIncomingLines, int iStartIndex) override;
    virtual bool isSameLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const override;
    virtual int defaultPageSize() const override;
    virtual void requestLines(int iStartIndex, int iCount) override;
    virtual QList<CLabel*> labelsFromLine(const CLogLine* pLine) const override;
};
