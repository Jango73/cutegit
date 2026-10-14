
#pragma once

// Qt
#include <QString>

//-------------------------------------------------------------------------------------------------

class CUtils
{
public:

    static const QString sFileUrlSchemePrefix;

    //! Returns the contents of a text file
    static QString getTextFileContents(const QString& sFileName);

    //! Writes text to a file
    static void setTextFileContents(const QString& sFileName, const QString& sText);

    //! Returns the local file path for a file name possibly holding a file URL
    static QString localFilePath(const QString& sFileName);
};
