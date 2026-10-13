
// Qt
#include <QApplication>
#include <QDebug>

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

int main(int argc, char *argv[])
{
    qInfo() << QString("CuteGit %1 built %2").arg(VERSION_STRING).arg(BUILD_TIMESTAMP);

    QApplication app(argc, argv);
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
