#ifndef _CVOICEDETECTOR_H
#define _CVOICEDETECTOR_H

/**
 * @brief The CVoiceDetector class
 *
 *  Voice Activity Detector for Granite's
 *  RecorderSnd4
 */

//prj
#include "Granite/Common/WndTypes.h"
#include "TimeDomainMetric.h"

//std
#include <vector>

//boost
#include <boost/ptr_container/ptr_vector.hpp>

namespace grn
{
    class CVoiceDetector
    {
    private:
        ///< container for samples
        std::vector<double> m_vecData;

        ///< metrics used for speech detection
        boost::ptr_vector<CTimeDomainMetric> m_vecMetrics;
    public:
        ///< threshold coefficient, equals 1 by default
        double m_dblThresholdCoefficient;

    public:
        CVoiceDetector();

        //true if pDataSrc contains a speech sound block
        bool IsSpeechPresent(
                const SHORT* const pDataSrc
                , const unsigned int uSize_bytes);
    };
} //namespace grn

#endif // _CVOICEDETECTOR_H
