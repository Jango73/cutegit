
#pragma once

// Qt
#include <QString>

//-------------------------------------------------------------------------------------------------

class CDiffConstants
{
public:

    //-------------------------------------------------------------------------------------------------
    // Hunk header pattern shared by diff parsing and patch building
    //-------------------------------------------------------------------------------------------------

    static const QString sHunkHeaderPattern;

    static const int iHunkOldStartCapture;
    static const int iHunkOldCountWithCommaCapture;
    static const int iHunkOldCountCapture;
    static const int iHunkNewStartCapture;
    static const int iHunkNewCountWithCommaCapture;
    static const int iHunkNewCountCapture;
    static const int iHunkSectionCapture;

    //-------------------------------------------------------------------------------------------------
    // Diff line prefixes shared by diff parsing
    //-------------------------------------------------------------------------------------------------

    static const QString sDiffFilePrefix;
    static const QString sDiffHunkPrefix;
    static const QString sDiffAddedPrefix;
    static const QString sDiffDeletedPrefix;
    static const QString sDiffAddedHeaderPrefix;
    static const QString sDiffDeletedHeaderPrefix;
    static const QString sDiffIndexPrefix;
    static const QString sDiffNoNewlinePrefix;

    //-------------------------------------------------------------------------------------------------
    // Diff file name pattern capture
    //-------------------------------------------------------------------------------------------------

    static const int iDiffFilePathCapture;

    //-------------------------------------------------------------------------------------------------
    // Sentinels shared by diff parsing and patch building
    //-------------------------------------------------------------------------------------------------

    static const int iInvalidHunkId;
    static const int iNoLineNumber;
};
