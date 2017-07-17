////////////////////////////////////////////////////////////////////////
/// \file   ROIFinderMorphological.cc
/// \author T. Usher
////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "uboone/CalData/DeconTools/IROIFinder.h"
#include "art/Utilities/ToolMacros.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "larcore/Geometry/Geometry.h"

#include "TH1D.h"

#include <fstream>

namespace uboone_tool
{

class ROIFinderMorphological : public IROIFinder
{
public:
    explicit ROIFinderMorphological(const fhicl::ParameterSet& pset);
    
    ~ROIFinderMorphological();
    
    void configure(const fhicl::ParameterSet& pset)                          override;
    void outputHistograms(art::TFileDirectory&)                        const override;
    
    void FindROIs(const Waveform&, size_t, double, CandidateROIVec&)   const override;
    
private:
    // The actual ROI finding algorithm
    void findROICandidatesDiff(const Waveform&,
                               const Waveform&,
                               const Waveform&,
                               const geo::PlaneID::PlaneID_t&,
                               int,
                               int,
                               float,
                               CandidateROIVec&) const;
    
    // Average the input waveform
    void getErosionDilationAverageDifference(const Waveform&, size_t, Waveform&, Waveform&, Waveform&, Waveform&) const;
    
    // grrrrr
    float fixTheFreakingWaveform(const Waveform&, Waveform&) const;
    
    // recover the rms
    float getTruncatedRMS(const Waveform&) const;
    
    // Member variables from the fhicl file
    std::vector<float>              fNumSigma;                   ///< "# sigma" rms noise for ROI threshold
    std::vector<size_t>             fMax2MinDistance;            ///< Maxmimum allow peak to peak distance
    std::vector<int>                fHalfWindowSize;             ///< 1/2 the window size
    std::vector<unsigned short>     fPreROIPad;                  ///< ROI padding
    std::vector<unsigned short>     fPostROIPad;                 ///< ROI padding
    float                           fTruncRMSMinFraction;        ///< or at least this fraction of time bins
    
    // Services
    const geo::GeometryCore*        fGeometry = lar::providerFrom<geo::Geometry>();
};
    
//----------------------------------------------------------------------
// Constructor.
ROIFinderMorphological::ROIFinderMorphological(const fhicl::ParameterSet& pset)
{
    configure(pset);
}
    
ROIFinderMorphological::~ROIFinderMorphological()
{
}
    
void ROIFinderMorphological::configure(const fhicl::ParameterSet& pset)
{
    // Start by recovering the parameters
    std::vector<unsigned short> uin;
    std::vector<unsigned short> vin;
    std::vector<unsigned short> zin;
    
    fNumSigma            = pset.get< std::vector<float>>         ("NumSigma"       );
    fMax2MinDistance     = pset.get< std::vector<size_t>>        ("Max2MinDistance");
    fHalfWindowSize      = pset.get< std::vector<int>>           ("HalfWindowSize" );
    uin                  = pset.get< std::vector<unsigned short>>("uPlaneROIPad"   );
    vin                  = pset.get< std::vector<unsigned short>>("vPlaneROIPad"   );
    zin                  = pset.get< std::vector<unsigned short>>("zPlaneROIPad"   );
    fTruncRMSMinFraction = pset.get< float >                     ("TruncRMSMinFraction", 0.6);
    
    if(uin.size() != 2 || vin.size() != 2 || zin.size() != 2) {
        throw art::Exception(art::errors::Configuration)
        << "u/v/z plane ROI pad size != 2";
    }
    
    fPreROIPad.resize(3);
    fPostROIPad.resize(3);
    
    // put the ROI pad sizes into more convenient vectors
    fPreROIPad[0]  = uin[0];
    fPostROIPad[0] = uin[1];
    fPreROIPad[1]  = vin[0];
    fPostROIPad[1] = vin[1];
    fPreROIPad[2]  = zin[0];
    fPostROIPad[2] = zin[1];
    
    return;
}

void ROIFinderMorphological::FindROIs(const Waveform& waveform, size_t channel, double rmsNoise, CandidateROIVec& roiVec) const
{
    // The idea here is to consider the input waveform - if an induction plane then it is already in differential form,
    // if a collection plane then we form the differential - then smooth and look for ROIs. The technique for actually
    // finding ROI's will be to compute the erosion and dilation vectors, get their average and then use these to determine
    // candidate ROI's
    
    // First up, determine what kind of wire we have
    std::vector<geo::WireID> wids    = fGeometry->ChannelToWire(channel);
    const geo::PlaneID&      planeID = wids[0].planeID();
//    geo::SigType_t           sigType = fGeometry->SignalType(planeID);
/*
    // Local copy of the input waveform
    Waveform waveformDeriv(waveform.size());
    
    // If we have a collection plane then take the derivative
    if (sigType == geo::kCollection)
    {
        waveformDeriv[0]                 = 0;
        waveformDeriv[waveform.size()-1] = 0;
        
        for(size_t idx = 1; idx < waveform.size()-1; idx++)
            waveformDeriv[idx] = 0.5 * (waveform.at(idx+1) - waveform.at(idx-1));
    }
    // Otherwise a straight copy since the bipolar pulses are, effectively, derivatives
    else std::copy(waveform.begin(),waveform.end(),waveformDeriv.begin());
*/
    // Do the averaging
    Waveform erosionVec;
    Waveform dilationVec;
    Waveform averageVec;
    Waveform differenceVec;
    
    //getErosionDilationAverage(waveformDeriv, planeID.Plane, erosionVec, dilationVec, averageVec);
    getErosionDilationAverageDifference(waveform, planeID.Plane, erosionVec, dilationVec, averageVec, differenceVec);
    
    // Baseline correct with the average
//    std::transform(waveformDeriv.begin(),waveformDeriv.end(),averageVec.begin(),waveformDeriv.begin(),[](const auto& val, const auto& cor){return val - cor;});
    
    // Scheme for finding a suitable threshold
//    float truncRMS = getTruncatedRMS(waveformDeriv);
    
    // Now find the ROI's
//    findROICandidates(waveformDeriv.begin(),waveformDeriv.end(),planeID.Plane,0,truncRMS,roiVec);
    
    // Use the average vector to find ROI's
    float truncRMS = getTruncatedRMS(differenceVec);
    
    findROICandidatesDiff(differenceVec, erosionVec, dilationVec, planeID.Plane, 0, averageVec.size(), truncRMS, roiVec);
    
    if (roiVec.empty()) return;
    
    // pad the ROIs
    for(auto& roi : roiVec)
    {
        // low ROI end
        roi.first  = std::max(int(roi.first - fPreROIPad[planeID.Plane]),0);
        // high ROI end
        roi.second = std::min(roi.second + fPostROIPad[planeID.Plane], waveform.size() - 1);
    }
    
    // merge overlapping (or touching) ROI's
    if(roiVec.size() > 1)
    {
        // temporary vector for merged ROIs
        CandidateROIVec tempRoiVec;
        
        // Loop through candidate roi's
        size_t startRoi = roiVec.front().first;
        size_t stopRoi  = startRoi;
        
        for(auto& roi : roiVec)
        {
            if (roi.first <= stopRoi + 50) stopRoi = roi.second;
            else
            {
                tempRoiVec.push_back(CandidateROI(startRoi,stopRoi));
                
                startRoi = roi.first;
                stopRoi  = roi.second;
            }
        }
        
        // Make sure to get the last one
        tempRoiVec.push_back(CandidateROI(startRoi,stopRoi));
        
        roiVec = tempRoiVec;
    }
    
    return;
}
    
void ROIFinderMorphological::findROICandidatesDiff(const Waveform&                differenceVec,
                                                   const Waveform&                erosionVec,
                                                   const Waveform&                dilationVec,
                                                   const geo::PlaneID::PlaneID_t& plane,
                                                   int                            startTick,
                                                   int                            stopTick,
                                                   float                          truncRMS,
                                                   CandidateROIVec&               roiCandidateVec) const
{
    int roiLength = stopTick - startTick;
    
    if (roiLength > 0)
    {
        // The idea here is to find the difference and use that as the seed for searching the
        // erosion and dilation vectors for the end points
        Waveform::const_iterator maxItr = std::max_element(differenceVec.begin()+startTick,differenceVec.begin()+stopTick,[](const auto& left, const auto& right){return std::fabs(left) < std::fabs(right);});
        
        int   maxTick       = std::distance(differenceVec.begin(),maxItr);
        float maxDifference = *maxItr;
        
        // Start by finding maximum range of the erosion vector at this extremum
        int   maxCandRoiTick = maxTick;
        
        // The test on the erosion vector takes care of special case of slowly rising pulses which are characteristic of
        // tracks running into the wire plane
        while(++maxCandRoiTick < stopTick)
        {
            float difference = differenceVec.at(maxCandRoiTick);
            float erosion    = erosionVec.at(maxCandRoiTick);
            
            if (difference < 1.5 * truncRMS && erosion < 1.5 * truncRMS) break;
        }
        
        maxCandRoiTick = std::min(maxCandRoiTick,stopTick);
        
        // Now do the dilation vector
        int   minCandRoiTick = maxTick;
        
        while(--minCandRoiTick >= startTick)
        {
            float difference = differenceVec.at(minCandRoiTick);
            float erosion    = erosionVec.at(minCandRoiTick);
            
            if (difference < 1.5 * truncRMS && erosion < 1.5 * truncRMS) break;
        }
        
        // make sure we have not gone under
        minCandRoiTick = std::max(minCandRoiTick, startTick);
        
        // Make sure we have a legitimate candidate
        if (maxCandRoiTick - minCandRoiTick > 0 && maxDifference > fNumSigma.at(plane) * truncRMS)
        {
            // Before saving this ROI, look for candidates preceeding this one
            // Note that preceeding snippet will reference to the current roiStartTick
            findROICandidatesDiff(differenceVec, erosionVec, dilationVec, plane, startTick, minCandRoiTick, truncRMS, roiCandidateVec);
            
            // Save this ROI
            roiCandidateVec.push_back(CandidateROI(minCandRoiTick, maxCandRoiTick));
            
            // Now look for candidate ROI's downstream of this one
            findROICandidatesDiff(differenceVec, erosionVec, dilationVec, plane, maxCandRoiTick+1, stopTick, truncRMS, roiCandidateVec);
        }
    }
    
    return;
}
    
void ROIFinderMorphological::getErosionDilationAverageDifference(const Waveform& inputWaveform,
                                                                 size_t          plane,
                                                                 Waveform&       erosionVec,
                                                                 Waveform&       dilationVec,
                                                                 Waveform&       averageVec,
                                                                 Waveform&       differenceVec) const
{
    // Set the window size
    int halfWindowSize(fHalfWindowSize.at(plane));
    
    // Define the "smallest" function
    //    auto smaller = [](const auto& left, const auto& right){return std::fabs(left) < std::fabs(right);};
    
    // Initialize min and max elements
    std::pair<Waveform::const_iterator,Waveform::const_iterator> minMaxItr = std::minmax_element(inputWaveform.begin(),inputWaveform.begin()+halfWindowSize);
    
    Waveform::const_iterator minElementItr = minMaxItr.first;
    Waveform::const_iterator maxElementItr = minMaxItr.second;
    
    // Initialize the erosion and dilation vectors
    erosionVec.resize(inputWaveform.size());
    dilationVec.resize(inputWaveform.size());
    averageVec.resize(inputWaveform.size());
    differenceVec.resize(inputWaveform.size());
    
    // Now loop through remaining elements and complete the vectors
    Waveform::iterator minItr = erosionVec.begin();
    Waveform::iterator maxItr = dilationVec.begin();
    Waveform::iterator aveItr = averageVec.begin();
    Waveform::iterator difItr = differenceVec.begin();
    
    for (std::vector<float>::const_iterator inputItr = inputWaveform.begin(); inputItr != inputWaveform.end(); inputItr++)
    {
        // There are two conditions to check:
        // 1) is the current min/max element outside the current window?
        // 2) is the new element smaller/larger than the current min/max?
        
        // Make sure we are not running off the end of the vector
        if (std::distance(inputItr,inputWaveform.end()) > halfWindowSize)
        {
            if (std::distance(minElementItr,inputItr) >= halfWindowSize)
                minElementItr = std::min_element(inputItr - halfWindowSize + 1, inputItr + halfWindowSize + 1);
            else if (*(inputItr + halfWindowSize) < *minElementItr)
                minElementItr = inputItr + halfWindowSize;
            
            if (std::distance(maxElementItr,inputItr) >= halfWindowSize)
                maxElementItr = std::max_element(inputItr - halfWindowSize + 1, inputItr + halfWindowSize + 1);
            else if (*(inputItr + halfWindowSize) > *maxElementItr)
                maxElementItr = inputItr + halfWindowSize;
        }
        
        // Update the vectors
        *minItr++ = *minElementItr;
        *maxItr++ = *maxElementItr;
        *aveItr++ = 0.5 * (*maxElementItr + *minElementItr);
        *difItr++ = *maxElementItr - *minElementItr;
    }
    
    return;
}
    
float ROIFinderMorphological::getTruncatedRMS(const Waveform& waveform) const
{
    // do rms calculation - the old fashioned way and over all adc values
    Waveform locWaveform = waveform;
    
    // sort in ascending order so we can truncate the sume
    std::sort(locWaveform.begin(), locWaveform.end(),[](const auto& left, const auto& right){return std::fabs(left) < std::fabs(right);});
    
    float localRMS = std::inner_product(locWaveform.begin(), locWaveform.begin() + locWaveform.size()/2, locWaveform.begin(), 0.);
    
    localRMS = std::sqrt(std::max(float(0.),localRMS / float(locWaveform.size()/2)));
    
    float threshold = 6. * localRMS;
    
    std::vector<float>::iterator threshItr = std::find_if(locWaveform.begin(),locWaveform.end(),[threshold](const auto& val){return std::fabs(val) > threshold;});
    
    int minNumBins = std::max(int(fTruncRMSMinFraction * locWaveform.size()),int(std::distance(locWaveform.begin(),threshItr)));
    
    // Get the truncated sum
    localRMS = std::inner_product(locWaveform.begin(), locWaveform.begin() + minNumBins, locWaveform.begin(), 0.);
    localRMS = std::sqrt(std::max(float(0.),localRMS / float(minNumBins)));
    
    return localRMS;
}
    
void ROIFinderMorphological::outputHistograms(art::TFileDirectory& histDir) const
{
    // It is assumed that the input TFileDirectory has been set up to group histograms into a common
    // folder at the calling routine's level. Here we create one more level of indirection to keep
    // histograms made by this tool separate.
/*
    std::string dirName = "ROIFinderPlane_" + std::to_string(fPlane);
    
    art::TFileDirectory dir = histDir.mkdir(dirName.c_str());
    
    auto const* detprop      = lar::providerFrom<detinfo::DetectorPropertiesService>();
    double      samplingRate = detprop->SamplingRate();
    double      numBins      = fROIFinderVec.size();
    double      maxFreq      = 500. / samplingRate;
    std::string histName     = "ROIFinderPlane_" + std::to_string(fPlane);
    
    TH1D*       hist         = dir.make<TH1D>(histName.c_str(), "ROIFinder;Frequency(MHz)", numBins, 0., maxFreq);
    
    for(int bin = 0; bin < numBins; bin++)
    {
        double freq = maxFreq * double(bin + 0.5) / double(numBins);
        
        hist->Fill(freq, fROIFinderVec.at(bin).Re());
    }
*/
    
    return;
}
    
DEFINE_ART_CLASS_TOOL(ROIFinderMorphological)
}
