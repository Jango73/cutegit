
// Qt
#include <QDebug>

// Application
#include "CRepoFile.h"

//-------------------------------------------------------------------------------------------------

const QString CRepoFile::sTokenClean = " ";
const QString CRepoFile::sTokenAdded = "+";
const QString CRepoFile::sTokenModified = "M";
const QString CRepoFile::sTokenRenamed = "=";
const QString CRepoFile::sTokenDeleted = "-";
const QString CRepoFile::sTokenMissing = "???";
const QString CRepoFile::sTokenUntracked = "?";
const QString CRepoFile::sTokenIgnored = "!";

const QString CRepoFile::sTokenStaged = "X";
const QString CRepoFile::sTokenUnstaged = " ";

const QString CRepoFile::sRepositoryStatusClean = "";
const QString CRepoFile::sRepositoryStatusMerge = "Merge";
const QString CRepoFile::sRepositoryStatusRebase = "Rebase";
const QString CRepoFile::sRepositoryStatusInteractiveRebase = "Interactive rebase";

//-------------------------------------------------------------------------------------------------

CRepoFile::CRepoFile(QObject* parent)
    : QObject(parent)
    , m_eStatus(CEnums::eUntracked)
    , m_bStaged(false)
    , m_bChanging(false)
{
}

//-------------------------------------------------------------------------------------------------

CRepoFile::CRepoFile(const CRepoFile& target, QObject* parent)
    : QObject(parent)
    , m_sFullName(target.m_sFullName)
    , m_sFileName(target.m_sFileName)
    , m_sRelativeName(target.m_sRelativeName)
    , m_eStatus(target.m_eStatus)
    , m_bStaged(target.m_bStaged)
    , m_bChanging(false)
{
}

//-------------------------------------------------------------------------------------------------

CRepoFile::~CRepoFile()
{
}

//-------------------------------------------------------------------------------------------------

QString CRepoFile::statusToString() const
{
    switch (m_eStatus)
    {
    case CEnums::eClean:
        return sTokenClean;
    case CEnums::eAdded:
        return sTokenAdded;
    case CEnums::eModified:
        return sTokenModified;
    case CEnums::eRenamed:
        return sTokenRenamed;
    case CEnums::eDeleted:
        return sTokenDeleted;
    case CEnums::eMissing:
        return sTokenMissing;
    case CEnums::eUntracked:
        return sTokenUntracked;
    case CEnums::eIgnored:
        return sTokenIgnored;
    }

    return sTokenUntracked;
}

//-------------------------------------------------------------------------------------------------

QString CRepoFile::stagedToString() const
{
    return m_bStaged ? sTokenStaged : sTokenUnstaged;
}

//-------------------------------------------------------------------------------------------------

bool CRepoFile::isNotClean() const
{
    // Same rule as the modified-files flag in CRepository::onNewOutputListOfCRepoFile:
    // every status but clean and ignored counts as a change (untracked and
    // missing included), so the changing flag is set for those files as well
    return m_eStatus != CEnums::eClean && m_eStatus != CEnums::eIgnored;
}

//-------------------------------------------------------------------------------------------------

bool CRepoFile::operator < (const CRepoFile& target)
{
    return m_sFullName < target.m_sFullName;
}
