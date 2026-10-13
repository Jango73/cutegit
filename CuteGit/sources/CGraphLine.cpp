
// Application
#include "CGraphLine.h"
#include "CLabel.h"

//-------------------------------------------------------------------------------------------------

//! Builds one owned label from a single decoration token, or null when empty
static CLabel* labelFromToken(const QString& sToken)
{
    QString sTrimmedToken = sToken.trimmed();

    if (sTrimmedToken.isEmpty())
        return nullptr;

    CLabel* pLabel = new CLabel();

    if (sTrimmedToken.startsWith("HEAD -> "))
    {
        pLabel->setName(sTrimmedToken.mid(8).trimmed());
        pLabel->setType(CEnums::LocalBranchLabel);
    }
    else if (sTrimmedToken.startsWith("tag: "))
    {
        pLabel->setName(sTrimmedToken.mid(5).trimmed());
        pLabel->setType(CEnums::TagLabel);
    }
    else if (sTrimmedToken == "HEAD")
    {
        pLabel->setName(sTrimmedToken);
        pLabel->setType(CEnums::LocalBranchLabel);
    }
    else if (sTrimmedToken.startsWith("origin/") || sTrimmedToken.startsWith("remotes/"))
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
