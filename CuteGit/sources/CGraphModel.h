
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
        eIsWorkingTreeRole,
    };

    //-------------------------------------------------------------------------------------------------
    // Static control methods
    //-------------------------------------------------------------------------------------------------

    //! Returns the commit identifier of the working tree line, never a real commit
    static QString workingTreeCommitId();

public:

    //-------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CGraphModel(CRepository* pRepository, QObject *parent = nullptr);

    //! Destructor
    virtual ~CGraphModel() override;

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //! Returns true when the working tree line is shown on top of the graph
    bool hasWorkingTreePresent() const;

    //-------------------------------------------------------------------------------------------------
    // Setters
    //-------------------------------------------------------------------------------------------------

    //! Shows or hides the working tree line on top of the graph, keeping lanes stable
    void setWorkingTreePresent(bool bPresent);

    //! Updates the working tree line message, no-op when absent
    void setWorkingTreeMessage(const QString& sMessage);

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

    //-------------------------------------------------------------------------------------------------
    // Protected control methods
    //-------------------------------------------------------------------------------------------------

    //! Builds the working tree line with the given parent, owned by the caller
    CGraphLine* createWorkingTreeLine(const QString& sParentId) const;

    //! Returns the commit identifier of the first real commit, empty when none is loaded
    QString firstRealCommitId() const;

    //! Updates the parent of the working tree line to the first real commit
    void refreshWorkingTreeParent();

    //! Lays out every loaded line, working tree line included
    void layoutLoadedLines();

protected:

    //! True when the working tree line is shown on top of the graph
    bool m_bWorkingTreePresent;
};
