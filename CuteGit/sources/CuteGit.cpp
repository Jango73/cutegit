
// Qt
#include <QDebug>
#include <QQuickStyle>
#include <QApplication>
#include <QCoreApplication>

// qt-plus
#include <CXMLNode.h>

// Application
#include "CuteGit.h"
#include "CEnums.h"
#include "CRepository.h"
#include "CDiffLine.h"

/*!
    \class CuteGit
    \inmodule CuteGit
    \section1 General
    This is the base class for all repository control interfaces.
    It allows execution of a process in background.

                .''
      ._.-.___.' (`\
     //(        ( `'
    '/ )\ ).__. ) 
    ' <' `\ ._/'\
       `   \     \
*/

//-------------------------------------------------------------------------------------------------

CuteGit::CuteGit(bool bMasterMode, const QString& sStubFileName, const QString& sSequenceFileName)
    : m_pController(nullptr)
    , m_pEngine(nullptr)
{
    QString sFinalStubFileName = sStubFileName;

    if (sFinalStubFileName.isEmpty())
    {
        sFinalStubFileName = QCoreApplication::applicationFilePath();
    }

    if (bMasterMode)
    {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps); //HiDPI pixmaps

//        qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");
//        qputenv("QT_SCALE_FACTOR", "1.5");

        qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");

        // Register types
        qRegisterMetaType<CXMLNode>();
        qRegisterMetaType<CProcessResult>();
        qRegisterMetaType<CRepoFileList>();
        qRegisterMetaType<CEnums::EProcessCommand>("CEnums::EProcessCommand");
        qRegisterMetaType<CEnums::ECapability>("CEnums::ECapability");
        qRegisterMetaType<CEnums::ERepositoryType>("CEnums::ERepositoryType");
        qRegisterMetaType<CEnums::ERepositoryStatus>("CEnums::ERepositoryStatus");
        qRegisterMetaType<CEnums::EDiffOperation>("CEnums::EDiffOperation");
        qRegisterMetaType<CEnums::ESortField>("CEnums::ESortField");

        qmlRegisterUncreatableType<CEnums>("CuteGit", 1, 0, "CEnums", "Cannot create a CEnums instance.");

        // Create controller and QML engine
        m_pController = new CController(sFinalStubFileName, this);
        m_pEngine = new QQmlApplicationEngine(this);

        // Set context properties
        m_pEngine->rootContext()->setContextProperty("controller", m_pController);

        QQuickStyle::setStyle("Material");

        // Load UI
        m_pEngine->load(QUrl(QStringLiteral("qrc:/qml/App.qml")));
    }
    else
    {
        m_pController = new CController(sStubFileName, sSequenceFileName, this);
    }
}

//-------------------------------------------------------------------------------------------------

CuteGit::~CuteGit()
{
    delete m_pEngine;
    delete m_pController;
}
