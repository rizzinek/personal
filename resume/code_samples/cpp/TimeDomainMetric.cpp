#include "TimeDomainMetric.h"

CTimeDomainMetric::CTimeDomainMetric(
        unsigned uNoiseDetectionDelay
        , double dblThreshold)
    : m_uNoiseDetectionDelay(uNoiseDetectionDelay)
    , m_dblThreshold(dblThreshold)
{
    ResetNoiseBlockCounter();
}

/**
 * @brief CTimeDomainMetric::operator () calculates the metric
 *  , checks the noise detection delay and
    using these two parameters, makes a decision on whether the block
    is speech or not
 * @param vecData block samples converted to double
 * @return true if the block should be considered speech
 */
bool CTimeDomainMetric::operator ()(const std::vector<double>& vecData)
{
    double dblVal = CalculateMetric(vecData);

    if(dblVal < m_dblThreshold)
    {
        ResetNoiseBlockCounter();
        return true;
    }

    ++m_uNoiseBlocksCounter;

    return (m_uNoiseBlocksCounter < m_uNoiseDetectionDelay);
}

/**
 * @brief CTimeDomainMetric::ResetNoiseBlockCounter
 *  resets the noise block counter each time a speech block is detected
 */
void CTimeDomainMetric::ResetNoiseBlockCounter()
{
    m_uNoiseBlocksCounter = 0;
}
