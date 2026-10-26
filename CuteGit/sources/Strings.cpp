
// Application
#include "Strings.h"

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::objectOpened(const QString& sObjectName)
//! Returns "%1 opened." with the object name applied.
//! Translated at call time so the active translator applies.
QString Strings::objectOpened(const QString& sObjectName)
{
    return tr("%1 opened.").arg(sObjectName);
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::objectClosed(const QString& sObjectName)
//! Returns "%1 closed." with the object name applied.
//! Translated at call time so the active translator applies.
QString Strings::objectClosed(const QString& sObjectName)
{
    return tr("%1 closed.").arg(sObjectName);
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::objectIsNotARepository(const QString& sObjectPath)
//! Returns "%1 is not a repository..." with the path applied.
//! Translated at call time so the active translator applies.
QString Strings::objectIsNotARepository(const QString& sObjectPath)
{
    return tr("%1 is not a repository.\nPlease select a folder containing a repository.").arg(sObjectPath);
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::objectDoesNotExist(const QString& sObjectPath)
//! Returns "%1 does not exist..." with the path applied.
//! Translated at call time so the active translator applies.
QString Strings::objectDoesNotExist(const QString& sObjectPath)
{
    return tr("%1 does not exist. Ignoring.").arg(sObjectPath);
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::pushingToGerrit()
//! Returns "Pushing to gerrit...".
//! Translated at call time so the active translator applies.
QString Strings::pushingToGerrit()
{
    return tr("Pushing to gerrit...");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::unableToGetBranch()
//! Returns "Unable to get current branch name...".
//! Translated at call time so the active translator applies.
QString Strings::unableToGetBranch()
{
    return tr("Unable to get current branch name...");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::commitStarted()
//! Returns "Commit started...".
//! Translated at call time so the active translator applies.
QString Strings::commitStarted()
{
    return tr("Commit started...");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::commitFinished()
//! Returns "Commit finished.".
//! Translated at call time so the active translator applies.
QString Strings::commitFinished()
{
    return tr("Commit finished.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::commitFailed()
//! Returns "Commit failed.".
//! Translated at call time so the active translator applies.
QString Strings::commitFailed()
{
    return tr("Commit failed.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::amendStarted()
//! Returns "Amend started...".
//! Translated at call time so the active translator applies.
QString Strings::amendStarted()
{
    return tr("Amend started...");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::amendFinished()
//! Returns "Amend finished.".
//! Translated at call time so the active translator applies.
QString Strings::amendFinished()
{
    return tr("Amend finished.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::amendFailed()
//! Returns "Amend failed.".
//! Translated at call time so the active translator applies.
QString Strings::amendFailed()
{
    return tr("Amend failed.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::pushStarted()
//! Returns "Push started...".
//! Translated at call time so the active translator applies.
QString Strings::pushStarted()
{
    return tr("Push started...");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::pushFinished()
//! Returns "Push finished.".
//! Translated at call time so the active translator applies.
QString Strings::pushFinished()
{
    return tr("Push finished.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::pushFailed()
//! Returns "Push failed.".
//! Translated at call time so the active translator applies.
QString Strings::pushFailed()
{
    return tr("Push failed.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::pullStarted()
//! Returns "Pull started...".
//! Translated at call time so the active translator applies.
QString Strings::pullStarted()
{
    return tr("Pull started...");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::pullFinished()
//! Returns "Pull finished.".
//! Translated at call time so the active translator applies.
QString Strings::pullFinished()
{
    return tr("Pull finished.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::pullFailed()
//! Returns "Pull failed.".
//! Translated at call time so the active translator applies.
QString Strings::pullFailed()
{
    return tr("Pull failed.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::fetchStarted()
//! Returns "Fetch started...".
//! Translated at call time so the active translator applies.
QString Strings::fetchStarted()
{
    return tr("Fetch started...");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::fetchFinished()
//! Returns "Fetch finished.".
//! Translated at call time so the active translator applies.
QString Strings::fetchFinished()
{
    return tr("Fetch finished.");
}

//-------------------------------------------------------------------------------------------------

//!
//! \fn QString Strings::fetchFailed()
//! Returns "Fetch failed.".
//! Translated at call time so the active translator applies.
QString Strings::fetchFailed()
{
    return tr("Fetch failed.");
}
