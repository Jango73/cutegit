
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
    static const QString sDiffContextPrefix;
    static const QString sDiffNewFilePrefix;
    static const QString sDiffDeletedFilePrefix;
    static const QString sDiffOldModePrefix;
    static const QString sDiffNewModePrefix;
    static const QString sDiffSimilarityPrefix;
    static const QString sDiffDissimilarityPrefix;
    static const QString sDiffRenameFromPrefix;
    static const QString sDiffRenameToPrefix;
    static const QString sDiffCopyFromPrefix;
    static const QString sDiffCopyToPrefix;

    //-------------------------------------------------------------------------------------------------
    // Patch blocks shared by patch generation
    //-------------------------------------------------------------------------------------------------

    static const QString sNullFileName;
    static const QString sPatchOldSideFormat;
    static const QString sPatchNewSideFormat;
    static const QString sPatchDiffHeaderFormat;
    static const QString sPatchOldHeaderFormat;
    static const QString sPatchNewHeaderFormat;
    static const QString sPatchHunkHeaderFormat;
    static const QString sStagePatchFileTemplate;

    //-------------------------------------------------------------------------------------------------
    // Diff file name pattern capture
    //-------------------------------------------------------------------------------------------------

    static const int iDiffFilePathCapture;

    //-------------------------------------------------------------------------------------------------
    // Sentinels shared by diff parsing and patch building
    //-------------------------------------------------------------------------------------------------

    static const int iInvalidHunkId;
    static const int iNoLineNumber;
    static const int iDiffPrefixLength;
    static const int iFirstColumn;
};
