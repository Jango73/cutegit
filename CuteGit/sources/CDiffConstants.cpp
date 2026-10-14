
// Application
#include "CDiffConstants.h"

//-------------------------------------------------------------------------------------------------

const QString CDiffConstants::sHunkHeaderPattern = "@@\\s+-([0-9]+)(,([0-9]+))?\\s+\\+([0-9]+)(,([0-9]+))?\\s+@@(.*)";

const int CDiffConstants::iHunkOldStartCapture = 1;
const int CDiffConstants::iHunkOldCountWithCommaCapture = 2;
const int CDiffConstants::iHunkOldCountCapture = 3;
const int CDiffConstants::iHunkNewStartCapture = 4;
const int CDiffConstants::iHunkNewCountWithCommaCapture = 5;
const int CDiffConstants::iHunkNewCountCapture = 6;
const int CDiffConstants::iHunkSectionCapture = 7;

const QString CDiffConstants::sDiffFilePrefix = "diff";
const QString CDiffConstants::sDiffHunkPrefix = "@@";
const QString CDiffConstants::sDiffAddedPrefix = "+";
const QString CDiffConstants::sDiffDeletedPrefix = "-";
const QString CDiffConstants::sDiffAddedHeaderPrefix = "+++";
const QString CDiffConstants::sDiffDeletedHeaderPrefix = "---";
const QString CDiffConstants::sDiffIndexPrefix = "index";
const QString CDiffConstants::sDiffNoNewlinePrefix = "\\";
const QString CDiffConstants::sDiffContextPrefix = " ";

const QString CDiffConstants::sNullFileName = "/dev/null";
const QString CDiffConstants::sPatchOldSideFormat = "a/%1";
const QString CDiffConstants::sPatchNewSideFormat = "b/%1";
const QString CDiffConstants::sPatchDiffHeaderFormat = "diff --git a/%1 b/%1";
const QString CDiffConstants::sPatchOldHeaderFormat = "--- %1";
const QString CDiffConstants::sPatchNewHeaderFormat = "+++ %1";
const QString CDiffConstants::sPatchHunkHeaderFormat = "@@ -%1,%2 +%3,%4 @@%5";
const QString CDiffConstants::sStagePatchFileTemplate = "%1/CuteGitStageXXXXXX.patch";

const int CDiffConstants::iDiffFilePathCapture = 3;

const int CDiffConstants::iInvalidHunkId = -1;
const int CDiffConstants::iNoLineNumber = -1;
// Length of the single-character prefix of diff lines (" ", "+", "-")
const int CDiffConstants::iDiffPrefixLength = 1;
// First column of single-column diff models
const int CDiffConstants::iFirstColumn = 0;
