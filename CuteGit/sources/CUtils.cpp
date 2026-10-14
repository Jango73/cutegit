
// Qt
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QUrl>

// Application
#include "CUtils.h"

//-------------------------------------------------------------------------------------------------

const QString CUtils::sFileUrlSchemePrefix = "file:";

QString CUtils::getTextFileContents(const QString& sFileName)
{
    QFile file(sFileName);
    QString sInputText;

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        sInputText = stream.readAll();
        file.close();
    }

    return sInputText;
}

//-------------------------------------------------------------------------------------------------

void CUtils::setTextFileContents(const QString& sFileName, const QString& sText)
{
    QFile file(sFileName);

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << sText;
        file.close();
    }
}

//-------------------------------------------------------------------------------------------------

/*!
    \fn CUtils::localFilePath
    Returns the local file path for a file name possibly holding a file URL.
*/
QString CUtils::localFilePath(const QString& sFileName)
{
    if (sFileName.startsWith(sFileUrlSchemePrefix))
        return QUrl(sFileName).toLocalFile();

    return sFileName;
}

//-------------------------------------------------------------------------------------------------
