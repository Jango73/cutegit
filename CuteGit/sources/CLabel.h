
#pragma once

// Qt
#include <QObject>
#include <QDateTime>

// qt-plus
#include "Macros.h"

// Application
#include "CEnums.h"

//-------------------------------------------------------------------------------------------------

class CLabel : public QObject
{
    Q_OBJECT

    //-------------------------------------------------------------------------------------------------
    // QML properties
    //-------------------------------------------------------------------------------------------------

    Q_FAST_PROPERTY(QString, s, name, Name)
    Q_FAST_PROPERTY(CEnums::ELabelType, e, type, Type)

public:

    //-------------------------------------------------------------------------------------------------
    // Constructors
    //-------------------------------------------------------------------------------------------------

    CLabel(QObject* parent = nullptr)
        : QObject(parent)
        , m_eType(CEnums::UnknownLabel)
    {}

    CLabel(const CLabel& target)
        : QObject(nullptr)
        , m_sName(target.m_sName)
        , m_eType(target.m_eType)
    {
    }

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Returns the signature identifying a label: its type and name
    static QString signature(const CLabel* pLabel)
    {
        return QString("%1:%2").arg(int(pLabel->type())).arg(pLabel->name());
    }
};
