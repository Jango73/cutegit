
// Qt
#include <QDebug>
#include <QDir>
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
    A leading tilde is expanded to the user home folder: shells do this,
    QProcess and QDir do not, so an unexpanded tilde creates literal
    folders named "~" instead of the intended home-relative path.
*/
QString CUtils::localFilePath(const QString& sFileName)
{
    QString sLocalPath = sFileName;

    if (sLocalPath.startsWith(sFileUrlSchemePrefix))
        sLocalPath = QUrl(sLocalPath).toLocalFile();

    if (sLocalPath == "~")
        return QDir::homePath();

    if (sLocalPath.startsWith("~/"))
        return QDir::homePath() + sLocalPath.mid(1);

    return sLocalPath;
}

//-------------------------------------------------------------------------------------------------
