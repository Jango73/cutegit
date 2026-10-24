
// Application
#include "CGraphLine.h"
#include "CLabel.h"

//-------------------------------------------------------------------------------------------------

//! Builds one owned label from a single git decoration token, or null when empty
// Type mapping: "HEAD -> <branch>" and bare names are local branches,
// "tag: <name>" are tags, "origin/..." and "remotes/..." are remote branches.
// A bare "HEAD" means a detached head with no branch name: it shows as a
// local-branch chip carrying the "HEAD" name. The "remotes/" prefix is
// stripped so the name matches the branch models, which store short names.
static CLabel* labelFromToken(const QString& sToken)
{
    static const QString sHeadPointsToPrefix = "HEAD -> ";
    static const QString sTagPrefix = "tag: ";
    static const QString sOriginPrefix = "origin/";
    static const QString sRemotesPrefix = "remotes/";

    QString sTrimmedToken = sToken.trimmed();

    if (sTrimmedToken.isEmpty())
        return nullptr;

    CLabel* pLabel = new CLabel();

    if (sTrimmedToken.startsWith(sHeadPointsToPrefix))
    {
        pLabel->setName(sTrimmedToken.mid(sHeadPointsToPrefix.size()).trimmed());
        pLabel->setType(CEnums::LocalBranchLabel);
    }
    else if (sTrimmedToken.startsWith(sTagPrefix))
    {
        pLabel->setName(sTrimmedToken.mid(sTagPrefix.size()).trimmed());
        pLabel->setType(CEnums::TagLabel);
    }
    else if (sTrimmedToken == "HEAD")
    {
        pLabel->setName(sTrimmedToken);
        pLabel->setType(CEnums::LocalBranchLabel);
    }
    else if (sTrimmedToken.startsWith(sRemotesPrefix))
    {
        pLabel->setName(sTrimmedToken.mid(sRemotesPrefix.size()).trimmed());
        pLabel->setType(CEnums::RemoteBranchLabel);
    }
    else if (sTrimmedToken.startsWith(sOriginPrefix))
    {
        pLabel->setName(sTrimmedToken);
        pLabel->setType(CEnums::RemoteBranchLabel);
    }
    else
    {
        pLabel->setName(sTrimmedToken);
        pLabel->setType(CEnums::LocalBranchLabel);
    }

    return pLabel;
}

//-------------------------------------------------------------------------------------------------

/***************************************************************************/

/*! Builds owned labels from git decorations */
QList<CLabel*> CGraphLine::createLabels() const
{
    QList<CLabel*> lReturnValue;

    for (const QString& sToken : m_lDecorations)
    {
        CLabel* pLabel = labelFromToken(sToken);

        if (pLabel != nullptr)
            lReturnValue << pLabel;
    }

    return lReturnValue;
}
