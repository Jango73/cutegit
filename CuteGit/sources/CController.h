
#pragma once

// Qt
#include <QObject>
#include <QTimer>
#include <QFileInfo>
#include <QStringListModel>
#include <QSharedMemory>
#include <QTranslator>

// qt-plus
#include "Macros.h"
#include "CXMLNode.h"

// Application
#include "CRepositoryModel.h"
#include "CRepository.h"

//-------------------------------------------------------------------------------------------------

#define CONFIG_FILE_NAME            "configuration.xml"
#define SHARED_MEMORY_MAX           65536
#define SHARED_MEMORY_TEXT_MAX      60000
#define SHARED_MEMORY_FILE_NAME_MAX 2000

//-------------------------------------------------------------------------------------------------

class CController : public QObject
{
    Q_OBJECT

    // Application version
    Q_FAST_PROPERTY(QString, s, version, Version)

    // History of the status text
    Q_FAST_PROPERTY(QStringListModel*, p, statusTextHistory, StatusTextHistory)

    // Models for repositories
    Q_FAST_PROPERTY(QStringListModel*, p, knownRepositoryModel, KnownRepositoryModel)
    Q_FAST_PROPERTY(CRepositoryModel*, p, openRepositoryModel, OpenRepositoryModel)
    Q_FAST_PROPERTY(CRepository*, p, currentRepository, CurrentRepository)

    // Models for languages
    Q_FAST_PROPERTY(QStringListModel*, p, langModel, LangModel)
    Q_FAST_PROPERTY_NO_SET_IMPL(QString, s, language, Language)

    // Repository history
    Q_FAST_PROPERTY(QString, s, lastBrowsedRepositoryURL, LastBrowsedRepositoryURL)
    Q_FAST_PROPERTY(QString, s, lastBrowsedRepositoryPath, LastBrowsedRepositoryPath)

    // Various information
    Q_FAST_PROPERTY(QString, s, statusText, StatusText)
    Q_FAST_PROPERTY(QString, s, theme, Theme)
    Q_FAST_PROPERTY(QString, s, stubFileName, StubFileName)

    Q_FAST_PROPERTY_NO_SET_IMPL(int, i, currentRepositoryIndex, CurrentRepositoryIndex)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showClean, ShowClean)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showAdded, ShowAdded)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showModified, ShowModified)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showRenamed, ShowRenamed)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showDeleted, ShowDeleted)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showMissing, ShowMissing)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showUntracked, ShowUntracked)
    Q_FAST_PROPERTY_NO_SET_IMPL(bool, b, showIgnored, ShowIgnored)

public:

    //-------------------------------------------------------------------------------------------------

    enum ESharedOperation
    {
        eSONone = 0,
        eSOSlaveRequestEdit,
        eSOMasterFinishedEdit
    };

    typedef struct
    {
        qint32              iMagic;
        ESharedOperation    eOperation;
        char                sSequenceFileName[SHARED_MEMORY_FILE_NAME_MAX];
        char                sText[SHARED_MEMORY_TEXT_MAX];
    } SMemoryStruct;

    //-------------------------------------------------------------------------------------------------
    // Constructor & destructor
    //-------------------------------------------------------------------------------------------------

    //! Master constructor
    CController(QString sStubFileName, QObject* parent = nullptr);

    //! Slave constructor
    CController(QString sStubFileName, QString sSequenceFileName, QObject* parent = nullptr);

    //! Destructor
    virtual ~CController();

    //-------------------------------------------------------------------------------------------------
    // Setters
    //-------------------------------------------------------------------------------------------------

    //!
    void setSharedOperation(ESharedOperation iOperation);

    //!
    void setSequenceFileName(const QString& sSequenceFileName);

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //!
    QString appDataPath() const;

    //!
    QString configFileName() const;

    //! Returns the interprocess operation running
    ESharedOperation sharedOperation();

    //! Returns the name of the Git sequence file
    QString sequenceFileName();

    //-------------------------------------------------------------------------------------------------
    // Control methods
    //-------------------------------------------------------------------------------------------------

    //! Saves configuration
    void saveConfiguration();

    //! Loads configuration
    void loadConfiguration();

    //! Clears the shared memory data
    void clearSharedMemory();

    //-------------------------------------------------------------------------------------------------
    // Invokables
    //-------------------------------------------------------------------------------------------------

    //! Quits the application
    Q_INVOKABLE void quit();

    //! Clones a repository
    Q_INVOKABLE void cloneRepository(QString sRepositoryURL, QString sRepositoryPath);

    //! Opens a repository
    Q_INVOKABLE void openRepository(QString sRepositoryPath);

    //!
    Q_INVOKABLE void removeRepository(int iRepositoryIndex);

    //!
    Q_INVOKABLE int currentRepositoryIndexToSet();

    //!
    Q_INVOKABLE QString repositoryNameFromPath(const QString& sPath) const;

    //-------------------------------------------------------------------------------------------------
    // Slots
    //-------------------------------------------------------------------------------------------------

protected slots:

    //!
    void onNewCloneOutput(CEnums::EProcessCommand eCommand, QString sOutput);

    //! Triggered for shared memory operation sync
    void onSharedTimerTick();

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

protected:

    bool                    m_bMasterMode;
    QSharedMemory           m_tShared;
    QTimer                  m_tSharedTimer;
    QString                 m_sCloneCommandsRepositoryPath;
    CCommands*              m_pCloneCommands;
    QTranslator*            m_pTranslator;

    QString                 m_sCurrentRepositoryPath;       // This is temporary, used for tab index

    static const QString    m_sSharedKey;
};
