
// Qt
#include <QObject>
#include <QString>

#pragma once

//-------------------------------------------------------------------------------------------------

class Strings : public QObject
{
    Q_OBJECT

public:

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //! Returns "%1 opened." with the object name applied, translated at call time
    static QString objectOpened(const QString& sObjectName);

    //! Returns "%1 closed." with the object name applied, translated at call time
    static QString objectClosed(const QString& sObjectName);

    //! Returns "%1 is not a repository..." with the path applied, translated at call time
    static QString objectIsNotARepository(const QString& sObjectPath);

    //! Returns "%1 does not exist..." with the path applied, translated at call time
    static QString objectDoesNotExist(const QString& sObjectPath);

    //! Returns "Pushing to gerrit...", translated at call time
    static QString pushingToGerrit();

    //! Returns "Unable to get current branch name...", translated at call time
    static QString unableToGetBranch();

    //! Returns "Commit started...", translated at call time
    static QString commitStarted();

    //! Returns "Commit finished.", translated at call time
    static QString commitFinished();

    //! Returns "Commit failed.", translated at call time
    static QString commitFailed();

    //! Returns "Amend started...", translated at call time
    static QString amendStarted();

    //! Returns "Amend finished.", translated at call time
    static QString amendFinished();

    //! Returns "Amend failed.", translated at call time
    static QString amendFailed();

    //! Returns "Push started...", translated at call time
    static QString pushStarted();

    //! Returns "Push finished.", translated at call time
    static QString pushFinished();

    //! Returns "Push failed.", translated at call time
    static QString pushFailed();

    //! Returns "Pull started...", translated at call time
    static QString pullStarted();

    //! Returns "Pull finished.", translated at call time
    static QString pullFinished();

    //! Returns "Pull failed.", translated at call time
    static QString pullFailed();

    //! Returns "Fetch started...", translated at call time
    static QString fetchStarted();

    //! Returns "Fetch finished.", translated at call time
    static QString fetchFinished();

    //! Returns "Fetch failed.", translated at call time
    static QString fetchFailed();
};
