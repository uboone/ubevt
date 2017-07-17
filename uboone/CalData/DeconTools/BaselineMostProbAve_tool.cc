////////////////////////////////////////////////////////////////////////
/// \file   Baseline.cc
/// \author T. Usher
////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "uboone/CalData/DeconTools/IBaseline.h"
#include "art/Utilities/ToolMacros.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"
#include "uboone/Utilities/SignalShapingServiceMicroBooNE.h"

#include <fstream>

namespace uboone_tool
{

class BaselineMostProbAve : IBaseline
{
public:
    explicit BaselineMostProbAve(const fhicl::ParameterSet& pset);
    
    ~BaselineMostProbAve();
    
    void configure(const fhicl::ParameterSet& pset)                                override;
    void outputHistograms(art::TFileDirectory&)                              const override;
    
    float GetBaseline(std::vector<float>&, raw::ChannelID_t, size_t, size_t) const override;
    
private:
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

    
float BaselineMostProbAve::GetBaseline(std::vector<float>& holder,
                                    raw::ChannelID_t    channel,
                                    size_t              roiStart,
                                    size_t              roiLen) const
{
    float base=0;
    
    // Recover the expected electronics noise on this channel
    float deconNoise = 1.26491 * fSignalShaping->GetDeconNoise(channel);
    
    // Basic idea is to find the most probable value in the ROI presented to us
    // From that, get the average value within range of the expected noise and
    // return that as the ROI baseline.
    auto const minmax = std::minmax_element(holder.begin()+roiStart,holder.begin()+roiStart+roiLen);
    
    float min = *(minmax.first);
    float max = *(minmax.second);
    
    if (max > min)
    {
        // we are being generous and allow for one bin more,
        // which is actually needed in the rare case where (max-min) is an integer
        size_t nbin = 2 * std::ceil(max - min) + 1;
        
        // In principle this can't happen...
        if (nbin == 0) return base;
        
        std::vector<int> roiHistVec(nbin, 0);
        
        for(size_t binIdx = roiStart; binIdx < roiStart+roiLen; binIdx++)
        {
            // Provide overkill protection against possibility of a bad index...
            int    intIdx = std::floor(2. * (holder.at(binIdx) - min));
            size_t idx    = std::max(std::min(intIdx,int(nbin-1)),0);
            
            roiHistVec.at(idx)++;
        }
        
        std::vector<int>::const_iterator mpValItr = std::max_element(roiHistVec.cbegin(),roiHistVec.cend());
        
        // Really can't see how this can happen... but check just to be sure
        if (mpValItr != roiHistVec.end())
        {
            float mpVal   = min + 0.5 * std::distance(roiHistVec.cbegin(),mpValItr);
            int   baseCnt = 0;
            
            for(size_t binIdx = roiStart; binIdx < roiStart+roiLen; binIdx++)
            {
                if (std::fabs(holder.at(binIdx) - mpVal) < deconNoise)
                {
                    base += holder.at(binIdx);
                    baseCnt++;
                }
            }
            
            if (baseCnt > 0) base /= baseCnt;
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