
// Qt
#include <QObject>
#include <QString>

#pragma once

//-------------------------------------------------------------------------------------------------

class Strings : public QObject
{
    Q_OBJECT

public:

    static const QString s_sSomeObjectNameOpened;
    static const QString s_sSomeObjectNameClosed;
    static const QString s_sSomeObjectNameIsNotARepository;
    static const QString s_sSomeObjectDoesNotExist;

    static const QString s_sPushingToGerrit;
    static const QString s_sUnableToGetBranch;

    static const QString s_sCommitStarted;
    static const QString s_sCommitFinished;
    static const QString s_sCommitFailed;
    static const QString s_sAmendStarted;
    static const QString s_sAmendFinished;
    static const QString s_sAmendFailed;
    static const QString s_sPushStarted;
    static const QString s_sPushFinished;
    static const QString s_sPushFailed;
    static const QString s_sPullStarted;
    static const QString s_sPullFinished;
    static const QString s_sPullFailed;
    static const QString s_sFetchStarted;
    static const QString s_sFetchFinished;
    static const QString s_sFetchFailed;
};
