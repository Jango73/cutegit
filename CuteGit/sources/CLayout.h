#pragma once

// Qt
#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariantList>
#include <QVariantMap>

//-------------------------------------------------------------------------------------------------

//! Single tab of a panel, with an optional visibility condition
//! lVisibleWhen holds source tab identifiers that make this tab visible
//! An empty list means the tab is always visible
typedef struct
{
    QString     sIdentifier;
    QStringList lVisibleWhen;
} SLayoutTab;

//-------------------------------------------------------------------------------------------------

//! Single panel of a layout, placed by relative geometry (percent of content area)
typedef struct
{
    QString            sIdentifier;
    double             fPositionX;
    double             fPositionY;
    double             fWidth;
    double             fHeight;
    QVector<SLayoutTab> vTabs;
} SLayoutPanel;

//-------------------------------------------------------------------------------------------------

//! Single named layout: ordered list of panels
typedef struct
{
    QString               sName;
    QVector<SLayoutPanel> vPanels;
} SLayoutDefinition;

//-------------------------------------------------------------------------------------------------

/*!
    \class CLayout
    \inmodule CuteGit
    Parses descriptive UI layouts from a JSON file located next to the binary.
    Uses CXMLNode for parsing (scalars are read as text attributes).
*/
class CLayout
{
public:

    //-------------------------------------------------------------------------------------------------
    // Defines
    //-------------------------------------------------------------------------------------------------

    static const QString sFileName;
    static const QString sResourceFileName;
    static const QString sTagLayout;
    static const QString sTagPanel;
    static const QString sTagTab;
    static const QString sAttributeName;
    static const QString sAttributeIdentifier;
    static const QString sAttributePositionX;
    static const QString sAttributePositionY;
    static const QString sAttributeWidth;
    static const QString sAttributeHeight;
    static const QString sKeyPanelIdentifier;
    static const QString sKeyPositionX;
    static const QString sKeyPositionY;
    static const QString sKeyWidth;
    static const QString sKeyHeight;
    static const QString sKeyTabs;
    static const QString sKeyTabIdentifier;
    static const QString sKeyVisibleWhen;
    static const QString sAttributeVisibleWhen;

    //! Minimum and maximum values for panel geometry expressed in percent
    static const double fMinimumPercent;
    static const double fMaximumPercent;

    //! Fallback text values used when a geometry attribute is missing
    static const QString sDefaultPositionText;
    static const QString sDefaultSizeText;

    //! Error message used when no valid layout is found, %1 is the file name
    static const QString sInvalidLayoutMessage;

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Loads layouts from JSON file, returns default layouts when loading fails
    static QVector<SLayoutDefinition> loadLayouts(const QString& sLayoutFileName, QString* pErrorMessage = nullptr);

    //! Built-in fallback layouts (first entry reproduces the historical layout)
    static QVector<SLayoutDefinition> defaultLayouts();

    //! Converts a layout definition to a QML friendly panel list
    //! Variant keys below (sKey*) are the single C++ side of the C++/QML contract,
    //! mirrored in qml/components/Const.qml (panelIdKey, panelPosXKey, ...)
    static QVariantList toVariantPanels(const SLayoutDefinition& tLayoutDefinition);

    //! Returns the known tab identifiers (single C++ source of the tab vocabulary,
    //! mirrored in qml/components/Const.qml as knownTabIds)
    static const QStringList& knownTabIds();
};
