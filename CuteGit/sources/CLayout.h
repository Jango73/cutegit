
#pragma once

// Qt
#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariantList>
#include <QVariantMap>

//-------------------------------------------------------------------------------------------------

//! Single panel of a layout, placed by relative geometry (percent of content area)
typedef struct
{
    QString     sId;
    double      fPosX;
    double      fPosY;
    double      fWidth;
    double      fHeight;
    QStringList lTabIds;
} SLayoutPanel;

//-------------------------------------------------------------------------------------------------

//! Single named layout: ordered list of panels
typedef struct
{
    QString              sName;
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
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Loads layouts from JSON file, returns default layouts when loading fails
    static QVector<SLayoutDefinition> loadLayouts(const QString& sFileName, QString* pErrorString = nullptr);

    //! Built-in fallback layouts (first entry reproduces the historical layout)
    static QVector<SLayoutDefinition> defaultLayouts();

    //! Converts a layout definition to a QML friendly panel list
    //! Variant keys below (sKey*) are the single C++ side of the C++/QML contract,
    //! mirrored in qml/components/Const.qml (panelIdKey, panelPosXKey, ...)
    static QVariantList toVariantPanels(const SLayoutDefinition& tLayout);

    //! Returns the known tab identifiers (single C++ source of the tab vocabulary,
    //! mirrored in qml/components/Const.qml as knownTabIds)
    static QStringList knownTabIds();

    //-------------------------------------------------------------------------------------------------
    // Defines
    //-------------------------------------------------------------------------------------------------

    static const char* sFileName;
    static const char* sResourceFileName;
    static const char* sTagLayout;
    static const char* sTagPanel;
    static const char* sTagTab;
    static const char* sAttributeName;
    static const char* sAttributeId;
    static const char* sAttributePosX;
    static const char* sAttributePosY;
    static const char* sAttributeWidth;
    static const char* sAttributeHeight;
    static const char* sKeyPanelId;
    static const char* sKeyPosX;
    static const char* sKeyPosY;
    static const char* sKeyWidth;
    static const char* sKeyHeight;
    static const char* sKeyTabs;
    static const char* sKeyTabId;
};
