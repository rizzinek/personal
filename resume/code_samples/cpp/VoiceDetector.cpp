#include "VoiceDetector.h"

//prj
#include "BandMedianEnergyMetric.h"
#include "MinBandEnergyMetric.h"
#include "ZeroCrossingMetric.h"

//std
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

//boost
#include <boost/lambda/lambda.hpp>

namespace grn
{
    CVoiceDetector::CVoiceDetector()
    {
        m_vecMetrics.push_back(new CBandMedianEnergyMetric(
                                   CBandMedianEnergyMetric::c_uDefaultDelay
                                   , CBandMedianEnergyMetric::c_dblDefaultThreshold));
        m_vecMetrics.push_back(new CMinBandEnergyMetric(
                                   CMinBandEnergyMetric::c_uDefaultDelay
                                   , CMinBandEnergyMetric::c_dblDefaultThreshold));
        m_vecMetrics.push_back(new CZeroCrossingMetric(
                                   CZeroCrossingMetric::c_uDefaultDelay
                                   , CZeroCrossingMetric::c_dblDefaultThreshold));
    }

    /**
     * @brief CVoiceDetector::IsSpeechPresent determines whether the
     *  sound block should be considered speech or noise
     * @param pDataSrc sample buffer
     * @param uSize_bytes size of the buffer
     * @return true if the block is a speech block
     */
    bool CVoiceDetector::IsSpeechPresent(
            const SHORT* const pDataSrc
            , const unsigned int uSize_bytes)
    {
        unsigned int uNumSamples = uSize_bytes / sizeof(SHORT);

        //read the data
        m_vecData.clear();
        for(unsigned i = 0; i < uNumSamples; ++i)
            m_vecData.push_back(static_cast<double>(pDataSrc[i]));

        //normalize
        double dblMax = 0.0;
        for(unsigned i = 0; i < m_vecData.size(); ++i)
            if(fabs(m_vecData[i]) > dblMax)
                dblMax = fabs(m_vecData[i]);
        std::transform(m_vecData.begin(), m_vecData.end(), m_vecData.begin()
                       , boost::lambda::_1 / dblMax);

        //true if all are true
        //false if at least one of them is false
        //couldn't use std::all_of because of no C++0x :[
        for(unsigned i = 0; i < m_vecMetrics.size(); ++i)
        {
            if(!m_vecMetrics[i](m_vecData))
                return false;
        }

        return true;
    }
} //namespace grn
