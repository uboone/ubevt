////////////////////////////////////////////////////////////////////////
/// \file   Baseline.cc
/// \author T. Usher
////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "ubevt/CalData/DeconTools/IBaseline.h"
#include "art/Utilities/ToolMacros.h"
#include "art/Framework/Principal/Handle.h"
#include "art_root_io/TFileService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib_except/exception.h"
#include "ubevt/Utilities/SignalShapingServiceMicroBooNE.h"
#include "larcore/CoreUtils/ServiceUtil.h" // lar::providerFrom<>()

#include <fstream>
#include <algorithm> // std::minmax_element()

namespace uboone_tool
{

class BaselineMostProbAve : IBaseline
{
public:
    explicit BaselineMostProbAve(const fhicl::ParameterSet& pset);
    
    ~BaselineMostProbAve();
    
    void configure(const fhicl::ParameterSet& pset)                                      override;
    void outputHistograms(art::TFileDirectory&)                                    const override;
    
    float GetBaseline(const std::vector<float>&, raw::ChannelID_t, size_t, size_t) const override;
    
private:
    std::pair<float,int> GetBaseline(const std::vector<float>&, int, size_t, size_t) const;

    art::ServiceHandle<util::SignalShapingServiceMicroBooNE> fSignalShaping;
};
    
//----------------------------------------------------------------------
// Constructor.
BaselineMostProbAve::BaselineMostProbAve(const fhicl::ParameterSet& pset)
{
    configure(pset);
}
    
BaselineMostProbAve::~BaselineMostProbAve()
{
}
    
void BaselineMostProbAve::configure(const fhicl::ParameterSet& pset)
{
    // Get signal shaping service.
    fSignalShaping = art::ServiceHandle<util::SignalShapingServiceMicroBooNE>();
    
    return;
}

    
float BaselineMostProbAve::GetBaseline(const std::vector<float>& holder,
                                       raw::ChannelID_t          channel,
                                       size_t                    roiStart,
                                       size_t                    roiLen) const
{
    float base(0.);

    if (roiLen > 1)
    {
        // Recover the expected electronics noise on this channel
        float  deconNoise = 1.26491 * fSignalShaping->GetDeconNoise(channel);
        int    binRange   = std::max(1, int(deconNoise));
        size_t halfLen    = std::min(size_t(100),roiLen/2);
        size_t roiStop    = roiStart + roiLen;
        
        std::pair<float,int> baseFront = GetBaseline(holder, binRange, roiStart,          roiStop);
        std::pair<float,int> baseBack  = GetBaseline(holder, binRange, roiStop - halfLen, roiStop);
        
        if (std::fabs(baseFront.first - baseBack.first) > deconNoise)
        {
            if      (baseFront.second > 3 * baseBack.second  / 2) base = baseFront.first;
            else if (baseBack.second  > 3 * baseFront.second / 2) base = baseBack.first;
            else                                                  base = std::max(baseFront.first,baseBack.first);
        }
        else
            base = (baseFront.first*baseFront.second + baseBack.first*baseBack.second)/float(baseFront.second+baseBack.second);
    }
    
    return base;
}
    
std::pair<float,int> BaselineMostProbAve::GetBaseline(const std::vector<float>& holder,
                                                      int                       binRange,
                                                      size_t                    roiStart,
                                                      size_t                    roiStop) const
{
    std::pair<float,int> base(0.,1);
    
    if (roiStop > roiStart)
    {
        // Basic idea is to find the most probable value in the ROI presented to us
        // From that we can develop an average of the true baseline of the ROI.
        // To do that we employ a map based scheme
        std::map<int,int> frequencyMap;
        int               mpCount(0);
        int               mpVal(0);
        
        for(size_t idx = roiStart; idx < roiStop; idx++)
        {
            int intVal = std::round(2.*holder.at(idx));
            
            int binCount = ++frequencyMap[intVal];
            
            if (binCount > mpCount)
            {
                mpCount = binCount;
                mpVal   = intVal;
            }
        }
        
        // Safety check...
        if (mpCount > 0)
        {
            // take a weighted average of two neighbor bins
            int meanCnt  = 0;
            int meanSum  = 0;
        
            for(int idx = -binRange; idx <= binRange; idx++)
            {
                std::map<int,int>::iterator neighborItr = frequencyMap.find(mpVal+idx);
            
                if (neighborItr != frequencyMap.end() && 5 * neighborItr->second > mpCount)
                {
                    meanSum += neighborItr->first * neighborItr->second;
                    meanCnt += neighborItr->second;
                }
            }

            if (meanCnt > 0)
            {
                base.first  = 0.5 * float(meanSum) / float(meanCnt);
                base.second = meanCnt;
            }
        }
    }
    
    return base;
}
    
void BaselineMostProbAve::outputHistograms(art::TFileDirectory& histDir) const
{
    // It is assumed that the input TFileDirectory has been set up to group histograms into a common
    // folder at the calling routine's level. Here we create one more level of indirection to keep
    // histograms made by this tool separate.
/*
    std::string dirName = "BaselinePlane_" + std::to_string(fPlane);
    
    art::TFileDirectory dir = histDir.mkdir(dirName.c_str());
    
    auto const* detprop      = lar::providerFrom<detinfo::DetectorPropertiesService>();
    double      samplingRate = detprop->SamplingRate();
    double      numBins      = fBaselineVec.size();
    double      maxFreq      = 500. / samplingRate;
    std::string histName     = "BaselinePlane_" + std::to_string(fPlane);
    
    TH1D*       hist         = dir.make<TH1D>(histName.c_str(), "Baseline;Frequency(MHz)", numBins, 0., maxFreq);
    
    for(int bin = 0; bin < numBins; bin++)
    {
        double freq = maxFreq * double(bin + 0.5) / double(numBins);
        
        hist->Fill(freq, fBaselineVec.at(bin).Re());
    }
*/
    
    return;
}
    
DEFINE_ART_CLASS_TOOL(BaselineMostProbAve)
}
