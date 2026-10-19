
#pragma once

// Qt
#include <QAbstractListModel>

// Application
#include "CGraphLine.h"
#include "CGraphLineCollection.h"
#include "CLabelModel.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CRepository;

//-------------------------------------------------------------------------------------------------

class CGraphModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum ERoles
    {
        eCommitIdRole = Qt::UserRole + 1,
        eDateRole,
        eAuthorRole,
        eMessageRole,
        eLabelsRole,
        eMessageIsCompleteRole,
        eGraphCommitLaneRole,
        eGraphVerticalsRole,
        eGraphConnectionsRole,
    };

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(int, i, potentialCount, PotentialCount)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CGraphModel(CRepository* pRepository, QObject *parent = nullptr);

    //! Destructor
    virtual ~CGraphModel() override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Clears all data
    void clear();

    //! Sets some lines
    void setLines(CGraphLineCollection tCollection);

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const override;

    //! Returns row count
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    //! Returns data
    virtual QVariant data(const QModelIndex& index, int role) const override;

    //!
    virtual bool canFetchMore(const QModelIndex& parent) const override;

    //!
    virtual void fetchMore(const QModelIndex& parent) override;

    //!
    void commitChanged(const QString& sCommitId);

    //! Returns the line showing a commit, nullptr when not loaded
    const CGraphLine* findLine(const QString& sCommitId) const;

    //!
    void invalidate();

    //! Returns true when both lines show the same commit
    bool isSameGraphLine(const CGraphLine* pExistingLine, const CGraphLine* pIncomingLine) const;

    //-------------------------------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------------------------------

signals:

    void requestGraphData(int iStartIndex, int iCount);

private:

    //! Owning repository (plain member: no Q_PROPERTY to avoid an inclusion cycle)
    CRepository* m_pRepository;

    //! Lines of the graph
    QList<CGraphLine*>  m_lLines;

    //! Labels for commits
    QMap<QString, CLabelModel*> m_mLabels;
};
