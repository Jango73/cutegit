
// Qt
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QStandardPaths>

// Application
#include "CuteGit.h"

#if defined(__has_include)
#  if __has_include("BuildInfo.h")
#    include "BuildInfo.h"
#  endif
#endif

#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP "unknown"
#endif

// Available arguments
static const char* sSwitch_stub = "--stub";

//-------------------------------------------------------------------------------------------------
// Migrates settings saved under previous executable names (CuteGitApp,
// CuteGitApp-dbg) to the new stable location, if they have not been
// migrated yet. The original files are kept untouched.
//-------------------------------------------------------------------------------------------------

static void migrateLegacySettings(QApplication& app)
{
    const QString sNewDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(sNewDataPath);
    const QString sNewConfigPath = sNewDataPath + "/configuration.xml";

    if (QFile::exists(sNewConfigPath))
    {
        return;
    }

    const QStringList lOldApplicationNames = {"CuteGitApp", "CuteGitApp-dbg"};
    for (const QString& sOldApplicationName : lOldApplicationNames)
    {
        app.setApplicationName(sOldApplicationName);
        const QString sOldConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/configuration.xml";
        app.setApplicationName("CuteGit");

        if (QFile::exists(sOldConfigPath))
        {
            QFile::copy(sOldConfigPath, sNewConfigPath);
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    qInfo() << QString("CuteGit %1 built %2").arg(VERSION_STRING).arg(BUILD_TIMESTAMP);

    QApplication app(argc, argv);

    // Keep the application identity stable regardless of the executable file
    // name, so settings and data survive a rename of the binary.
    app.setApplicationName("CuteGit");

    // Migrate settings saved under previous executable names, if needed.
    migrateLegacySettings(app);

    // Embed the application icon so the running window shows it in the dock
    // (GNOME/Pop!_OS) instead of relying on the desktop file's Icon entry.
    app.setWindowIcon(QIcon(":/cutegit-icon.svg"));
    QStringList lArgList;
    QString sSequenceFile;
    QString sStubFileName;
    bool bMasterMode = true;

    // Check arguments
    if (argc > 1)
    {
        for (int iIndex = 1; iIndex < argc; iIndex++)
        {
            if (QString(argv[iIndex]).contains(sSwitch_stub))
            {
                if (iIndex < argc - 1)
                {
                    iIndex++;
                    sStubFileName = argv[iIndex];
                }
            }
            else
            {
                QFile file(argv[iIndex]);

                if (file.exists())
                {
                    bMasterMode = false;
                    sSequenceFile = argv[iIndex];
                }
            }
        }
    }

    // Build CuteGit instance
    CuteGit cuteGit(bMasterMode, sStubFileName, sSequenceFile);

    // Event loop
    return app.exec();
}
