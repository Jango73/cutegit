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
const char* CLayout::sTagLayout = "Layout";
const char* CLayout::sTagPanel = "Panel";
const char* CLayout::sTagTab = "Tab";
const char* CLayout::sAttributeName = "Name";
const char* CLayout::sAttributeId = "Id";
const char* CLayout::sAttributePosX = "X";
const char* CLayout::sAttributePosY = "Y";
const char* CLayout::sAttributeWidth = "Width";
const char* CLayout::sAttributeHeight = "Height";

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
    CXMLNodeList lTabNodes = xPanelNode.getNodesByTagName(CLayout::sTagTab);
    for (const CXMLNode& xTabNode : lTabNodes)
    {
        QString sTabId = xTabNode.attributes().value(CLayout::sAttributeId, "");
        if (sTabId.isEmpty() == false && CLayout::knownTabIds().contains(sTabId))
            tPanel.lTabIds << sTabId;
    }
    return tPanel;
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

    CXMLNodeList lLayoutNodes = xRoot.getNodesByTagName(sTagLayout);
    QVector<SLayoutDefinition> vLayouts;

    for (const CXMLNode& xLayoutNode : lLayoutNodes)
    {
        SLayoutDefinition tLayout;
        tLayout.sName = xLayoutNode.attributes().value(sAttributeName, "");
        if (tLayout.sName.isEmpty())
            continue;
        CXMLNodeList lPanelNodes = xLayoutNode.getNodesByTagName(sTagPanel);
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
    QVector<SLayoutDefinition> vLayouts;

    SLayoutDefinition tClassic;
    tClassic.sName = "Classic";
    SLayoutPanel tFiles;
    tFiles.sId = "Files";
    tFiles.fPosX = 0.0;
    tFiles.fPosY = 0.0;
    tFiles.fWidth = 40.0;
    tFiles.fHeight = 60.0;
    tFiles.lTabIds << "Files";
    tClassic.vPanels << tFiles;
    SLayoutPanel tTools;
    tTools.sId = "Tools";
    tTools.fPosX = 40.0;
    tTools.fPosY = 0.0;
    tTools.fWidth = 40.0;
    tTools.fHeight = 60.0;
    tTools.lTabIds << "Diff" << "FileLog" << "Blame";
    tClassic.vPanels << tTools;
    SLayoutPanel tBranches;
    tBranches.sId = "Branches";
    tBranches.fPosX = 80.0;
    tBranches.fPosY = 0.0;
    tBranches.fWidth = 20.0;
    tBranches.fHeight = 60.0;
    tBranches.lTabIds << "Branches" << "Tags";
    tClassic.vPanels << tBranches;
    SLayoutPanel tLogs;
    tLogs.sId = "Logs";
    tLogs.fPosX = 0.0;
    tLogs.fPosY = 60.0;
    tLogs.fWidth = 60.0;
    tLogs.fHeight = 40.0;
    tLogs.lTabIds << "BranchLog" << "Graph" << "RefLog";
    tClassic.vPanels << tLogs;
    SLayoutPanel tOutput;
    tOutput.sId = "Output";
    tOutput.fPosX = 60.0;
    tOutput.fPosY = 60.0;
    tOutput.fWidth = 40.0;
    tOutput.fHeight = 40.0;
    tOutput.lTabIds << "Output";
    tClassic.vPanels << tOutput;
    vLayouts << tClassic;

    SLayoutDefinition tSide;
    tSide.sName = "BranchesLeft";
    SLayoutPanel tSideBranches;
    tSideBranches.sId = "Branches";
    tSideBranches.fPosX = 0.0;
    tSideBranches.fPosY = 0.0;
    tSideBranches.fWidth = 20.0;
    tSideBranches.fHeight = 100.0;
    tSideBranches.lTabIds << "Branches" << "Tags";
    tSide.vPanels << tSideBranches;
    SLayoutPanel tSideCenter;
    tSideCenter.sId = "Center";
    tSideCenter.fPosX = 20.0;
    tSideCenter.fPosY = 0.0;
    tSideCenter.fWidth = 40.0;
    tSideCenter.fHeight = 100.0;
    tSideCenter.lTabIds << "Files" << "BranchLog" << "Graph" << "RefLog";
    tSide.vPanels << tSideCenter;
    SLayoutPanel tSideRight;
    tSideRight.sId = "Right";
    tSideRight.fPosX = 60.0;
    tSideRight.fPosY = 0.0;
    tSideRight.fWidth = 40.0;
    tSideRight.fHeight = 100.0;
    tSideRight.lTabIds << "Diff" << "FileLog" << "Blame" << "Output";
    tSide.vPanels << tSideRight;
    vLayouts << tSide;

    return vLayouts;
}

/************************************************************************/
QVariantList CLayout::toVariantPanels(const SLayoutDefinition& tLayout)
{
    QVariantList lPanels;
    for (const SLayoutPanel& tPanel : tLayout.vPanels)
    {
        QVariantMap mPanel;
        mPanel["panelId"] = tPanel.sId;
        mPanel["posX"] = tPanel.fPosX;
        mPanel["posY"] = tPanel.fPosY;
        mPanel["width"] = tPanel.fWidth;
        mPanel["height"] = tPanel.fHeight;
        QVariantList lTabs;
        for (const QString& sTabId : tPanel.lTabIds)
        {
            QVariantMap mTab;
            mTab["tabId"] = sTabId;
            lTabs << mTab;
        }
        mPanel["tabs"] = lTabs;
        lPanels << mPanel;
    }
    return lPanels;
}

/************************************************************************/
QStringList CLayout::knownTabIds()
{
    QStringList lTabIds;
    lTabIds << "Files" << "Branches" << "Tags" << "BranchLog" << "Graph" << "RefLog";
    lTabIds << "Diff" << "FileLog" << "Blame" << "Output";
    return lTabIds;
}
