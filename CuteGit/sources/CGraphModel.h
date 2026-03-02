
#pragma once

// Qt
#include <QAbstractListModel>

// qt-plus
#include "CGraphLine.h"

// Application
#include "CLabelModel.h"
#include "CRepository.h"

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

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(CRepository*, p, repository, Repository)

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

    //! Lines of the graph
    QList<CGraphLine*>  m_lLines;

    //! Labels for commits
    QMap<QString, CLabelModel*> m_mLabels;
};
