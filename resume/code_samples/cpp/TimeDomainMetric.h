#ifndef TIMEDOMAINMETRIC_H
#define TIMEDOMAINMETRIC_H

#include <vector>

/**
 * @brief The CTimeDomainMetric class
 *  base class for voice detection metrics in time domain
 */
class CTimeDomainMetric
{
private:
    ///only after this number of consecutive blocks are
    ///considered noise does the metric finally consider
    ///a block noise
    unsigned m_uNoiseDetectionDelay;
    ///threshold value for the metric
    ///isSpeech == metric value < threshold
    double m_dblThreshold;

    ///counter for the noise detection delay
    unsigned m_uNoiseBlocksCounter;
public:
    CTimeDomainMetric(unsigned uNoiseDetectionDelay, double dblThreshold);
    //calculates the metric, checks the noise detection delay and
    //using these two parameters, makes a decision on whether the block
    //is speech or not
    bool operator()(const std::vector<double>& vecData);

public:
    //internal function for metric calculation
    virtual double CalculateMetric(const std::vector<double>& vecData) = 0;

private:
    //resets the noise block counter each time a speech block is detected
    void ResetNoiseBlockCounter();
};

#endif // TIMEDOMAINMETRIC_H
