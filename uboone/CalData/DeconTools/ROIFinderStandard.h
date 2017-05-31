////////////////////////////////////////////////////////////////////////
/// \file   ROIFinder.h
/// \author T. Usher
////////////////////////////////////////////////////////////////////////

#include "uboone/CalData/DeconTools/IROIFinder.h"
#include "canvas/Utilities/Exception.h"

namespace uboone_tool
{

class ROIFinderStandard : public IROIFinder
{
public:
    explicit ROIFinderStandard(const fhicl::ParameterSet& pset);
    
    ~ROIFinderStandard();
    
    void configure(const fhicl::ParameterSet& pset)                          override;
    void outputHistograms(art::TFileDirectory&)                        const override;
    
    void FindROIs(Waveform&, size_t, double, CandidateROIVec&)         const override;
    
private:
    // Member variables from the fhicl file
    unsigned short                fNumBinsHalf;                ///< Determines # bins in ROI running sum
    std::vector<unsigned short>   fThreshold;                  ///< abs(threshold) ADC counts for ROI
    std::vector<int>              fNumSigma;                   ///< "# sigma" rms noise for ROI threshold
    std::vector<unsigned short>   fPreROIPad;                  ///< ROI padding
    std::vector<unsigned short>   fPostROIPad;                 ///< ROI padding
};
    
}
