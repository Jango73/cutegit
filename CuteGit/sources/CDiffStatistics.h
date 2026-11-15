
#pragma once

// Qt
#include <QList>

// Application
#include "CDiffLine.h"

//-------------------------------------------------------------------------------------------------

/*!
    \class CDiffStatistics
    \inmodule CuteGit
    \section1 General
    Counts the added and removed lines of a diff.
    Every place that summarizes diff lines uses this single computation: the
    commit details counters and the working tree line message always report
    the same numbers for the same lines.
*/

class CDiffStatistics
{
public:

    //-------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //-------------------------------------------------------------------------------------------------

    //! Builds empty statistics
    CDiffStatistics()
        : m_iAddedLineCount(0)
        , m_iRemovedLineCount(0)
    {
    }

    //! Builds statistics from a list of diff lines
    CDiffStatistics(const QList<CDiffLine*>& lLines)
        : m_iAddedLineCount(0)
        , m_iRemovedLineCount(0)
    {
        setLines(lLines);
    }

    //-------------------------------------------------------------------------------------------------
    // Setters
    //-------------------------------------------------------------------------------------------------

    //! Counts the added and removed lines of a list of diff lines
    void setLines(const QList<CDiffLine*>& lLines)
    {
        reset();

        for (const CDiffLine* pLine : lLines)
        {
            if (pLine == nullptr)
                continue;

            if (pLine->operation() == CEnums::DiffAdd)
                m_iAddedLineCount++;
            else if (pLine->operation() == CEnums::DiffDelete)
                m_iRemovedLineCount++;
        }
    }

    //! Resets both counters to zero
    void reset()
    {
        m_iAddedLineCount = 0;
        m_iRemovedLineCount = 0;
    }

    //-------------------------------------------------------------------------------------------------
    // Getters
    //-------------------------------------------------------------------------------------------------

    //! Returns the number of added lines
    int addedLineCount() const
    {
        return m_iAddedLineCount;
    }

    //! Returns the number of removed lines
    int removedLineCount() const
    {
        return m_iRemovedLineCount;
    }

    //! Returns true when no line was added nor removed
    bool isEmpty() const
    {
        return m_iAddedLineCount == 0 && m_iRemovedLineCount == 0;
    }

    //-------------------------------------------------------------------------------------------------
    // Operators
    //-------------------------------------------------------------------------------------------------

    //! Returns true when both statistics carry the same counters
    bool operator==(const CDiffStatistics& tOther) const
    {
        return m_iAddedLineCount == tOther.m_iAddedLineCount
                && m_iRemovedLineCount == tOther.m_iRemovedLineCount;
    }

    //! Returns true when the statistics differ
    bool operator!=(const CDiffStatistics& tOther) const
    {
        return not (*this == tOther);
    }

protected:

    //-------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------

    //! Number of added lines
    int m_iAddedLineCount;

    //! Number of removed lines
    int m_iRemovedLineCount;
};
