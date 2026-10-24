
#pragma once

// Qt
#include <QAbstractListModel>
#include <QMap>

// qt-plus
#include "Macros.h"

// Application
#include "CLogLine.h"
#include "CLabelModel.h"

//-------------------------------------------------------------------------------------------------
// Forward declarations

class CRepository;

//-------------------------------------------------------------------------------------------------

class CCommitLogModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum ERoles
    {
        eCommitIdRole = Qt::UserRole + 1,
        eDateRole,
        eAuthorRole,
        eMessageRole,
        eFullMessageRole,
        eLabelsRole,
        eMessageIsCompleteRole,
        eMarkedAsDiffFromRole,
        eMarkedAsDiffToRole,
    };

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(int, i, potentialCount, PotentialCount)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CCommitLogModel(CRepository* pRepository, QObject* parent = nullptr);

    //! Destructor
    virtual ~CCommitLogModel() override;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Clears all lines
    void clear();

    //! Reconciles loaded rows with incoming lines without flicker
    void setLines(QList<CLogLine*> lIncomingLines, int iStartIndex, int iPotentialCount);

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

    //! Returns the line showing a commit, nullptr when not loaded
    const CLogLine* findLine(const QString& sCommitId) const;

    //!
    void commitChanged(const QString& sCommitId);

    //! Rebuilds branch/tag labels of every loaded commit from the repository
    void refreshLabels();

    //! Rebuilds branch/tag labels of a single commit from the repository
    void refreshLabelsForCommit(const QString& sCommitId);

    //!
    void invalidate();

protected:

    //-------------------------------------------------------------------------------------------------
    // Hooks for subclasses
    //-------------------------------------------------------------------------------------------------

    //! Lays out incoming lines before reconciliation, no-op by default
    virtual void prepareLines(QList<CLogLine*>& lLoadedLines, QList<CLogLine*>& lIncomingLines, int iStartIndex);

    //! Returns true when both lines show the same commit
    virtual bool isSameLine(const CLogLine* pExistingLine, const CLogLine* pIncomingLine) const = 0;

    //! Returns the page size of fetchMore requests
    virtual int defaultPageSize() const = 0;

    //! Emits the fetchMore request of the subclass
    virtual void requestLines(int iStartIndex, int iCount) = 0;

    //! Returns the line at a row, nullptr when out of range
    const CLogLine* lineAtRow(int iRow) const;

    //! Deletes the labels of a commit when no remaining line shows it
    void pruneUnusedLabels(const QString& sCommitId);
    //! Rebuilds the label model of a commit, returns true when it changed
    bool syncLabelsForCommit(const QString& sCommitId);

    //! Owning repository (plain member: no Q_PROPERTY to avoid an inclusion cycle)
    CRepository* m_pRepository;

    //! Lines of the log
    QList<CLogLine*> m_lLines;

    //! Labels for commits
    QMap<QString, CLabelModel*> m_mLabels;
};
