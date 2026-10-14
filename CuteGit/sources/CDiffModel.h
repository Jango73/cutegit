
#pragma once

// Qt
#include <QAbstractListModel>

// qt-plus
#include "CDiffLine.h"

//-------------------------------------------------------------------------------------------------

class CDiffModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum ERoles
    {
        eOperationRole = Qt::UserRole + 1,
        eTextRole,
        eCommitIdRole,
        eFileNameRole,
        eHunkIdRole,
        eOldLineNumberRole,
        eNewLineNumberRole
    };

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
    CDiffModel(QObject *parent = nullptr);

    //! Destructor
    virtual ~CDiffModel();

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Sets all lines
    void setLines(QList<CDiffLine*> lNewLines);

    //! Returns the line at row, nullptr when out of range
    CDiffLine* lineAt(int iRow) const;

    //! Returns all lines (borrowed pointers, owned by the model)
    const QList<CDiffLine*>& lines() const;

    //! Returns role names
    virtual QHash<int, QByteArray> roleNames() const;

    //! Returns row count
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    //! Returns data
    virtual QVariant data(const QModelIndex& index, int role) const;

private:

    //! Diff lines
    QList<CDiffLine*>   m_lLines;
};
