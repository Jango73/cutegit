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

const char* CLayout::sFileName = "layouts.json";
const char* CLayout::sResourceFileName = ":/deploy/layouts.json";
const char* CLayout::sTagLayout = "Layout";
const char* CLayout::sTagPanel = "Panel";
const char* CLayout::sTagTab = "Tab";
const char* CLayout::sAttributeName = "Name";
const char* CLayout::sAttributeId = "Id";
const char* CLayout::sAttributePosX = "X";
const char* CLayout::sAttributePosY = "Y";
const char* CLayout::sAttributeWidth = "Width";
const char* CLayout::sAttributeHeight = "Height";
const char* CLayout::sKeyPanelId = "panelId";
const char* CLayout::sKeyPosX = "posX";
const char* CLayout::sKeyPosY = "posY";
const char* CLayout::sKeyWidth = "width";
const char* CLayout::sKeyHeight = "height";
const char* CLayout::sKeyTabs = "tabs";
const char* CLayout::sKeyTabId = "tabId";

//-------------------------------------------------------------------------------------------------

/************************************************************************/
static double toPercent(const QString& sValue, double fFallback)
{
    bool bConversionSucceeded = false;
    double fValue = sValue.toDouble(&bConversionSucceeded);
    if (bConversionSucceeded == false)
        return fFallback;
    if (fValue < 0.0)
        return 0.0;
    if (fValue > 100.0)
        return 100.0;
    return fValue;
}

//************************************************************************
static SLayoutPanel toPanel(const CXMLNode& xPanelNode)
{
    SLayoutPanel tPanel;
    tPanel.sId = xPanelNode.attributes().value(CLayout::sAttributeId, "");
    tPanel.fPosX = toPercent(xPanelNode.attributes().value(CLayout::sAttributePosX, "0"), 0.0);
    tPanel.fPosY = toPercent(xPanelNode.attributes().value(CLayout::sAttributePosY, "0"), 0.0);
    tPanel.fWidth = toPercent(xPanelNode.attributes().value(CLayout::sAttributeWidth, "100"), 100.0);
    tPanel.fHeight = toPercent(xPanelNode.attributes().value(CLayout::sAttributeHeight, "100"), 100.0);
    const QStringList lKnownTabIds = CLayout::knownTabIds();
    CXMLNodeList lTabNodes = xPanelNode.getNodesByTagName(CLayout::sTagTab);
    for (const CXMLNode& xTabNode : lTabNodes)
    {
        QString sTabId = xTabNode.attributes().value(CLayout::sAttributeId, "");
        if (sTabId.isEmpty())
            continue;
        if (lKnownTabIds.contains(sTabId) == false)
        {
            qWarning() << QString("CLayout: unknown tab identifier [%1] ignored").arg(sTabId);
            continue;
        }
        tPanel.lTabIds << sTabId;
    }
    return tPanel;
}

/************************************************************************/
/*!
    \fn static QVector<SLayoutDefinition> parseLayoutNodes(const CXMLNode& xRoot)
    Parses layout definitions from an already loaded JSON root node.
*/
static QVector<SLayoutDefinition> parseLayoutNodes(const CXMLNode& xRoot)
{
    QVector<SLayoutDefinition> vLayouts;
    CXMLNodeList lLayoutNodes = xRoot.getNodesByTagName(CLayout::sTagLayout);
    for (const CXMLNode& xLayoutNode : lLayoutNodes)
    {
        SLayoutDefinition tLayout;
        tLayout.sName = xLayoutNode.attributes().value(CLayout::sAttributeName, "");
        if (tLayout.sName.isEmpty())
            continue;
        CXMLNodeList lPanelNodes = xLayoutNode.getNodesByTagName(CLayout::sTagPanel);
        for (const CXMLNode& xPanelNode : lPanelNodes)
        {
            SLayoutPanel tPanel = toPanel(xPanelNode);
            if (tPanel.lTabIds.isEmpty())
                continue;
            tLayout.vPanels << tPanel;
        }
        if (tLayout.vPanels.isEmpty() == false)
            vLayouts << tLayout;
    }
    return vLayouts;
}

/************************************************************************/
/*!
    \fn static SLayoutPanel createPanel(const QString& sId, double fPosX, double fPosY, double fWidth, double fHeight, const QStringList& lTabIds)
    Builds a single panel without repeating member assignments at every call site.
*/
static SLayoutPanel createPanel(const QString& sId, double fPosX, double fPosY, double fWidth, double fHeight, const QStringList& lTabIds)
{
    SLayoutPanel tPanel;
    tPanel.sId = sId;
    tPanel.fPosX = fPosX;
    tPanel.fPosY = fPosY;
    tPanel.fWidth = fWidth;
    tPanel.fHeight = fHeight;
    tPanel.lTabIds = lTabIds;
    return tPanel;
}

/************************************************************************/
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
    tFallback.vPanels << createPanel("Fallback", 0.0, 0.0, 100.0, 100.0, CLayout::knownTabIds());
    return QVector<SLayoutDefinition>() << tFallback;
}

/************************************************************************/
QVector<SLayoutDefinition> CLayout::loadLayouts(const QString& sFileName, QString* pErrorString)
{
    QString sError;
    CXMLNode xRoot = CXMLNode::loadJSONFromFile(sFileName, &sError);

    if (xRoot.isEmpty())
    {
        if (pErrorString != nullptr)
            *pErrorString = sError;
        return defaultLayouts();
    }

    QVector<SLayoutDefinition> vLayouts = parseLayoutNodes(xRoot);

    if (vLayouts.isEmpty())
    {
        if (pErrorString != nullptr)
            *pErrorString = QString("No valid layout found in %1, using defaults").arg(sFileName);
        return defaultLayouts();
    }

    return vLayouts;
}

/************************************************************************/
QVector<SLayoutDefinition> CLayout::defaultLayouts()
{
    QString sError;
    CXMLNode xRoot = CXMLNode::loadJSONFromFile(sResourceFileName, &sError);
    if (xRoot.isEmpty() == false)
    {
        QVector<SLayoutDefinition> vLayouts = parseLayoutNodes(xRoot);
        if (vLayouts.isEmpty() == false)
            return vLayouts;
        qWarning() << QString("CLayout: embedded layouts invalid, using minimal fallback (%1)").arg(sError);
    }
    else
    {
        qWarning() << QString("CLayout: embedded layouts not found, using minimal fallback (%1)").arg(sError);
    }

    return minimalFallbackLayouts();
}

/************************************************************************/
QVariantList CLayout::toVariantPanels(const SLayoutDefinition& tLayout)
{
    QVariantList lPanels;
    for (const SLayoutPanel& tPanel : tLayout.vPanels)
    {
        QVariantMap mPanel;
        mPanel[sKeyPanelId] = tPanel.sId;
        mPanel[sKeyPosX] = tPanel.fPosX;
        mPanel[sKeyPosY] = tPanel.fPosY;
        mPanel[sKeyWidth] = tPanel.fWidth;
        mPanel[sKeyHeight] = tPanel.fHeight;
        QVariantList lTabs;
        for (const QString& sTabId : tPanel.lTabIds)
        {
            QVariantMap mTab;
            mTab[sKeyTabId] = sTabId;
            lTabs << mTab;
        }
        mPanel[sKeyTabs] = lTabs;
        lPanels << mPanel;
    }
    return lPanels;
}

/************************************************************************/
QStringList CLayout::knownTabIds()
{
    static const QStringList lTabIds = QStringList()
        << "Files" << "Branches" << "Tags" << "BranchLog" << "Graph" << "RefLog"
        << "Diff" << "FileLog" << "Blame" << "Output";
    return lTabIds;
}
