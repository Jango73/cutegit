
#pragma once

// Qt
#include <QAbstractListModel>

// Application
#include "CLabel.h"

//-------------------------------------------------------------------------------------------------

class CLabelModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum ERoles
    {
        eNameRole = Qt::UserRole + 1,
        eTypeRole,
    };

public:

    //-------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //-------------------------------------------------------------------------------------------------

    //! Default constructor
    CLabelModel(QObject *parent = nullptr);

    //! Destructor
    virtual ~CLabelModel();

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const;

    //! Returns row count
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    //! Returns data
    virtual QVariant data(const QModelIndex& qIndex, int iRole) const;

    //! Replaces the shown labels, taking ownership of the new list
    void setLabelList(QList<CLabel*> lNewList);

    //! Replaces the shown labels with owned fresh labels in place, returns true when changed
    //! The model identity never changes so views bound to it stay valid
    bool syncLabels(QList<CLabel*> lFreshLabels);

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

private:

    //! Returns true when the shown labels match the fresh labels
    bool isEqualTo(const QList<CLabel*>& lFreshLabels) const;

    //! List of labels
    QList<CLabel*> m_lLabels;
};
