// Application
#include "CLayout.h"

// qt-plus
#include "CXMLNode.h"

// Qt
#include <QDebug>

//-------------------------------------------------------------------------------------------------

/*!
    \class CLayout
    \inmodule CuteGit
    Descriptive UI layouts: each layout is an ordered list of panels placed by
    relative geometry, each panel holds an ordered list of tab identifiers.
*/

const QString CLayout::sFileName = "layouts.json";
const QString CLayout::sResourceFileName = ":/deploy/layouts.json";
const QString CLayout::sTagLayout = "Layout";
const QString CLayout::sTagPanel = "Panel";
const QString CLayout::sTagTab = "Tab";
const QString CLayout::sAttributeName = "Name";
const QString CLayout::sAttributeIdentifier = "Id";
const QString CLayout::sAttributePositionX = "X";
const QString CLayout::sAttributePositionY = "Y";
const QString CLayout::sAttributeWidth = "Width";
const QString CLayout::sAttributeHeight = "Height";
const QString CLayout::sKeyPanelIdentifier = "panelId";
const QString CLayout::sKeyPositionX = "posX";
const QString CLayout::sKeyPositionY = "posY";
const QString CLayout::sKeyWidth = "width";
const QString CLayout::sKeyHeight = "height";
const QString CLayout::sKeyTabs = "tabs";
const QString CLayout::sKeyTabIdentifier = "tabId";
const QString CLayout::sKeyVisibleWhen = "visibleWhen";
const QString CLayout::sAttributeVisibleWhen = "VisibleWhen";

const double CLayout::fMinimumPercent = 0.0;
const double CLayout::fMaximumPercent = 100.0;

const QString CLayout::sDefaultPositionText = "0";
const QString CLayout::sDefaultSizeText = "100";
const QString CLayout::sInvalidLayoutMessage = "No valid layout found in %1, using defaults";

//-------------------------------------------------------------------------------------------------

/*************************************************************************/
/*!
    \fn static double toPercent(const QString& sValue, double fFallback)
    Converts a text geometry value to a percent value clamped to the valid range.
*/
static double toPercent(const QString& sValue, double fFallback)
{
    bool bConversionSucceeded = false;
    double fValue = sValue.toDouble(&bConversionSucceeded);
    if (bConversionSucceeded == false)
        return fFallback;
    if (fValue < CLayout::fMinimumPercent)
        return CLayout::fMinimumPercent;
    if (fValue > CLayout::fMaximumPercent)
        return CLayout::fMaximumPercent;
    return fValue;
}

/*************************************************************************/
/*!
    \fn static QStringList toVisibleWhen(const CXMLNode& xTabNode)
    Collects the visibility condition of a Tab node.
    Accepts a comma separated VisibleWhen attribute and VisibleWhen child nodes
    (JSON array form), trims entries and drops empties.
*/
static QStringList toVisibleWhen(const CXMLNode& xTabNode)
{
    QStringList lVisibleWhen;
    const QStringList& lKnownTabIdentifiers = CLayout::knownTabIds();
    QString sAttributeValue = xTabNode.attributes().value(CLayout::sAttributeVisibleWhen, "");
    for (const QString& sPart : sAttributeValue.split(","))
    {
        QString sIdentifier = sPart.trimmed();
        if (sIdentifier.isEmpty())
            continue;
        if (lKnownTabIdentifiers.contains(sIdentifier) == false)
        {
            qWarning() << QString("CLayout: unknown VisibleWhen identifier [%1] ignored").arg(sIdentifier);
            continue;
        }
        if (lVisibleWhen.contains(sIdentifier) == false)
            lVisibleWhen << sIdentifier;
    }
    CXMLNodeList lConditionNodes = xTabNode.getNodesByTagName(CLayout::sAttributeVisibleWhen);
    for (const CXMLNode& xConditionNode : lConditionNodes)
    {
        QString sIdentifier = xConditionNode.value().trimmed();
        if (sIdentifier.isEmpty())
            continue;
        if (lKnownTabIdentifiers.contains(sIdentifier) == false)
        {
            qWarning() << QString("CLayout: unknown VisibleWhen identifier [%1] ignored").arg(sIdentifier);
            continue;
        }
        if (lVisibleWhen.contains(sIdentifier) == false)
            lVisibleWhen << sIdentifier;
    }
    return lVisibleWhen;
}

/*************************************************************************/
/*!
    \fn static SLayoutPanel toPanel(const CXMLNode& xPanelNode)
    Builds a single panel definition from a Panel node of the layout file.
*/
static SLayoutPanel toPanel(const CXMLNode& xPanelNode)
{
    SLayoutPanel tPanel;
    tPanel.sIdentifier = xPanelNode.attributes().value(CLayout::sAttributeIdentifier, "");
    tPanel.fPositionX = toPercent(xPanelNode.attributes().value(CLayout::sAttributePositionX, CLayout::sDefaultPositionText), CLayout::fMinimumPercent);
    tPanel.fPositionY = toPercent(xPanelNode.attributes().value(CLayout::sAttributePositionY, CLayout::sDefaultPositionText), CLayout::fMinimumPercent);
    tPanel.fWidth = toPercent(xPanelNode.attributes().value(CLayout::sAttributeWidth, CLayout::sDefaultSizeText), CLayout::fMaximumPercent);
    tPanel.fHeight = toPercent(xPanelNode.attributes().value(CLayout::sAttributeHeight, CLayout::sDefaultSizeText), CLayout::fMaximumPercent);
    const QStringList& lKnownTabIdentifiers = CLayout::knownTabIds();
    CXMLNodeList lTabNodes = xPanelNode.getNodesByTagName(CLayout::sTagTab);
    for (const CXMLNode& xTabNode : lTabNodes)
    {
        QString sTabIdentifier = xTabNode.attributes().value(CLayout::sAttributeIdentifier, "");
        if (sTabIdentifier.isEmpty())
            continue;
        if (lKnownTabIdentifiers.contains(sTabIdentifier) == false)
        {
            qWarning() << QString("CLayout: unknown tab identifier [%1] ignored").arg(sTabIdentifier);
            continue;
        }
        SLayoutTab tTab;
        tTab.sIdentifier = sTabIdentifier;
        tTab.lVisibleWhen = toVisibleWhen(xTabNode);
        tPanel.vTabs << tTab;
    }
    return tPanel;
}

/*************************************************************************/
/*!
    \fn static QVector<SLayoutDefinition> parseLayoutNodes(const CXMLNode& xRootNode)
    Parses layout definitions from an already loaded JSON root node.
*/
static QVector<SLayoutDefinition> parseLayoutNodes(const CXMLNode& xRootNode)
{
    QVector<SLayoutDefinition> vLayoutDefinitions;
    CXMLNodeList lLayoutNodes = xRootNode.getNodesByTagName(CLayout::sTagLayout);
    for (const CXMLNode& xLayoutNode : lLayoutNodes)
    {
        SLayoutDefinition tLayoutDefinition;
        tLayoutDefinition.sName = xLayoutNode.attributes().value(CLayout::sAttributeName, "");
        if (tLayoutDefinition.sName.isEmpty())
            continue;
        CXMLNodeList lPanelNodes = xLayoutNode.getNodesByTagName(CLayout::sTagPanel);
        for (const CXMLNode& xPanelNode : lPanelNodes)
        {
            SLayoutPanel tPanel = toPanel(xPanelNode);
            if (tPanel.vTabs.isEmpty())
                continue;
            tLayoutDefinition.vPanels << tPanel;
        }
        if (tLayoutDefinition.vPanels.isEmpty() == false)
            vLayoutDefinitions << tLayoutDefinition;
    }
    return vLayoutDefinitions;
}

/*************************************************************************/
/*!
    \fn static QVector<SLayoutTab> toTabsWithoutCondition(const QStringList& lTabIdentifiers)
    Wraps plain tab identifiers into always visible tab definitions.
*/
static QVector<SLayoutTab> toTabsWithoutCondition(const QStringList& lTabIdentifiers)
{
    QVector<SLayoutTab> vTabs;
    for (const QString& sTabIdentifier : lTabIdentifiers)
    {
        SLayoutTab tTab;
        tTab.sIdentifier = sTabIdentifier;
        vTabs << tTab;
    }
    return vTabs;
}

/*************************************************************************/
/*!
    \fn static SLayoutPanel createPanel(const QString& sIdentifier, double fPositionX, double fPositionY, double fWidth, double fHeight, const QVector<SLayoutTab>& vTabs)
    Builds a single panel without repeating member assignments at every call site.
*/
static SLayoutPanel createPanel(const QString& sIdentifier, double fPositionX, double fPositionY, double fWidth, double fHeight, const QVector<SLayoutTab>& vTabs)
{
    SLayoutPanel tPanel;
    tPanel.sIdentifier = sIdentifier;
    tPanel.fPositionX = fPositionX;
    tPanel.fPositionY = fPositionY;
    tPanel.fWidth = fWidth;
    tPanel.fHeight = fHeight;
    tPanel.vTabs = vTabs;
    return tPanel;
}

/*************************************************************************/
/*!
    \fn static QVector<SLayoutDefinition> minimalFallbackLayouts()
    Last resort layouts when neither the file next to the binary nor the embedded
    resource can be loaded. A single panel holding every known tab, so no tab
    vocabulary is duplicated here.
*/
static QVector<SLayoutDefinition> minimalFallbackLayouts()
{
    SLayoutDefinition tFallback;
    tFallback.sName = "Fallback";
    tFallback.vPanels << createPanel("Fallback", CLayout::fMinimumPercent, CLayout::fMinimumPercent, CLayout::fMaximumPercent, CLayout::fMaximumPercent, toTabsWithoutCondition(CLayout::knownTabIds()));
    return QVector<SLayoutDefinition>() << tFallback;
}

/*************************************************************************/
QVector<SLayoutDefinition> CLayout::loadLayouts(const QString& sLayoutFileName, QString* pErrorMessage)
{
    QString sError;
    CXMLNode xRootNode = CXMLNode::loadJSONFromFile(sLayoutFileName, &sError);

    if (xRootNode.isEmpty())
    {
        if (pErrorMessage != nullptr)
            *pErrorMessage = sError;
        return defaultLayouts();
    }

    QVector<SLayoutDefinition> vLayoutDefinitions = parseLayoutNodes(xRootNode);

    if (vLayoutDefinitions.isEmpty())
    {
        if (pErrorMessage != nullptr)
            *pErrorMessage = sInvalidLayoutMessage.arg(sLayoutFileName);
        return defaultLayouts();
    }

    return vLayoutDefinitions;
}

/*************************************************************************/
QVector<SLayoutDefinition> CLayout::defaultLayouts()
{
    QString sError;
    CXMLNode xRootNode = CXMLNode::loadJSONFromFile(CLayout::sResourceFileName, &sError);
    if (xRootNode.isEmpty() == false)
    {
        QVector<SLayoutDefinition> vLayoutDefinitions = parseLayoutNodes(xRootNode);
        if (vLayoutDefinitions.isEmpty() == false)
            return vLayoutDefinitions;
        qWarning() << QString("CLayout: embedded layouts invalid, using minimal fallback (%1)").arg(sError);
    }
    else
    {
        qWarning() << QString("CLayout: embedded layouts not found, using minimal fallback (%1)").arg(sError);
    }

    return minimalFallbackLayouts();
}

/*************************************************************************/
QVariantList CLayout::toVariantPanels(const SLayoutDefinition& tLayoutDefinition)
{
    QVariantList lPanels;
    for (const SLayoutPanel& tPanel : tLayoutDefinition.vPanels)
    {
        QVariantMap mPanel;
        mPanel[sKeyPanelIdentifier] = tPanel.sIdentifier;
        mPanel[sKeyPositionX] = tPanel.fPositionX;
        mPanel[sKeyPositionY] = tPanel.fPositionY;
        mPanel[sKeyWidth] = tPanel.fWidth;
        mPanel[sKeyHeight] = tPanel.fHeight;
        QVariantList lTabs;
        for (const SLayoutTab& tTab : tPanel.vTabs)
        {
            QVariantMap mTab;
            mTab[sKeyTabIdentifier] = tTab.sIdentifier;
            mTab[sKeyVisibleWhen] = tTab.lVisibleWhen;
            lTabs << mTab;
        }
        mPanel[sKeyTabs] = lTabs;
        lPanels << mPanel;
    }
    return lPanels;
}

/*************************************************************************/
const QStringList& CLayout::knownTabIds()
{
    static const QStringList lTabIdentifiers = QStringList()
        << "Files" << "Branches" << "Tags" << "BranchLog" << "Graph" << "RefLog"
        << "CommitDetails" << "FileDiff" << "CommitDiff" << "FileLog" << "Blame" << "Output";
    return lTabIdentifiers;
}
