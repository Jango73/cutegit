
#pragma once

// Qt
#include <QAbstractListModel>

// qt-plus
#include "CGraphLine.h"

// Application
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
        eGraphSymbolRole,
        eMessageIsCompleteRole,
    };

public:

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CGraphModel(CRepository* pRepository, QObject *parent = nullptr);

    //! Destructor
    virtual ~CGraphModel();

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Clears all data
    void clear();

    //! Sets all lines
    void setLines(QList<CGraphLine*> lNewLines);

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const;

    //! Returns row count
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    //! Returns data
    virtual QVariant data(const QModelIndex& index, int role) const;

    //!
    void setCommitMessage(const QString& sCommitId, const QString& sMessage);

    //!
    void commitChanged(const QString& sCommitId);

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

private:

    //! Owning repository (plain member: no Q_PROPERTY to avoid an inclusion cycle)
    CRepository* m_pRepository;

    //! Lines of the graph
    QList<CGraphLine*>  m_lLines;

    //! Labels for commits
    QMap<QString, CLabelModel*> m_mLabels;
};
