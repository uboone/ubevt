#ifndef RAWDIGITCORRELATEDCORRECTIONALG_H
#define RAWDIGITCORRELATEDCORRECTIONALG_H
////////////////////////////////////////////////////////////////////////
//
// Class:       RawDigitCorrelatedCorrectionAlg
// Module Type: algorithm
// File:        RawDigitCorrelatedCorrectionAlg.cxx
//
//              The intent of this algorithm is to perform "correlated noise"
//              correction across the input waveforms
//
// Configuration parameters:
//
// TheChoseWire          - Wire chosen for "example" hists
// MaxPedestalDiff       - Baseline difference to pedestal to flag
// SmoothCorrelatedNoise - Turns on the correlated noise suppression
// NumWiresToGroup       - When removing correlated noise, # wires to group
// FillHistograms        - Turn on histogram filling for diagnostics
// RunFFTInputWires      - FFT analyze the input RawDigits if true - diagnostics
// RunFFTCorrectedWires  - FFT analyze the output RawDigits if true - diagnostics
// TruncateTicks:        - Provide mechanism to truncate a readout window to a smaller size
// WindowSize:           - The desired size of the output window
// NumTicksToDropFront:  - The number ticks dropped off the front of the original window
//
//
// Created by Tracy Usher (usher@slac.stanford.edu) on January 6, 2016
// Based on work done by Brian Kirby, Mike Mooney and Jyoti Joshi
//
////////////////////////////////////////////////////////////////////////

#include "RawDigitNoiseFilterDefs.h"
#include "RawDigitFFTAlg.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art_root_io/TFileService.h"
#include "larcore/CoreUtils/ServiceUtil.h"
#include "larcore/Geometry/WireReadout.h"
namespace detinfo {
  class DetectorClocksData;
  class DetectorPropertiesData;
}

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

#include <set>

namespace caldata
{
class RawDigitCorrelatedCorrectionAlg
{
public:

    // Copnstructors, destructor.
    RawDigitCorrelatedCorrectionAlg(fhicl::ParameterSet const & pset);
    ~RawDigitCorrelatedCorrectionAlg();

    // Provide for initialization
    void reconfigure(fhicl::ParameterSet const & pset);
    void initializeHists(art::ServiceHandle<art::TFileService>&);
    
    void removeCorrelatedNoise(detinfo::DetectorClocksData const& clockData,
                               detinfo::DetectorPropertiesData const& detProp,
                               RawDigitAdcIdxPair& digitIdxPair,
                               raw::ChannelID_t    channel,
                               std::vector<float>& truncMeanWireVec,
                               std::vector<float>& truncRmsWireVec,
                               std::vector<short>& minMaxWireVec,
                               std::vector<short>& meanWireVec,
                               std::vector<float>& skewnessWireVec,
                               std::vector<float>& neighborRatioWireVec,
                               std::vector<float>& pedCorWireVec) const;
    
    void getTruncatedMeanRMS(const RawDigitVector& waveform, float& mean, float& rms) const;

private:
    
    void smoothCorrectionVec(std::vector<float>&, raw::ChannelID_t) const;
    
    template<class T> T getMedian(std::vector<T>&, T) const;
    
    float getMostProbable(const std::vector<float>&, float) const;
    
    void getErosionDilationAverageDifference(const RawDigitVector& inputWaveform,
                                             RawDigitVector&       erosionVec,
                                             RawDigitVector&       dilationVec,
                                             std::vector<float>&   averageVec,
                                             RawDigitVector&       differenceVec) const;
    
    void getErosionDilationAverageDifference(const std::vector<float>& inputWaveform,
                                             std::vector<float>&       erosionVec,
                                             std::vector<float>&       dilationVec,
                                             std::vector<float>&       averageVec,
                                             std::vector<float>&       differenceVec) const;
    
    void getTruncatedMeanRMS(const std::vector<float>& waveform, float& mean, float& rms) const;

    // Fcl parameters.
    float                fTruncMeanFraction;     ///< Fraction for truncated mean
    //bool                 fSmoothCorrelatedNoise; ///< Should we smooth the noise?
    bool                 fApplyCorSmoothing;     ///< Attempt to smooth the correlated noise correction?
    bool                 fApplyFFTCorrection;    ///< Use an FFT to get the correlated noise correction
    bool                 fFillFFTHistograms;     ///< Fill associated FFT histograms
    std::vector<size_t>  fFFTHistsWireGroup;     ///< Wire Group to pick on
    std::vector<size_t>  fFFTNumHists;           ///< Number of hists total per view
    std::vector<double>  fFFTHistsStartTick;     ///< Starting tick for histograms
    std::vector<double>  fFFTMinPowerThreshold;  ///< Threshold for trimming FFT power spectrum
    std::vector<size_t>  fNumWiresToGroup;       ///< If smoothing, the number of wires to look at
    bool                 fFillHistograms;        ///< if true then will fill diagnostic hists
    bool                 fRunFFTCorrected;       ///< Should we run FFT's on corrected wires?
    std::vector<float>   fNumRmsToSmoothVec;     ///< # "sigma" to smooth correlated correction vec
    
    // Pointers to the histograms we'll create for monitoring what is happening
    TProfile*            fFFTHist[3];
    TProfile*            fFFTHistLow[3];
    TProfile*            fFFTHistCor[3];
    
    TProfile*            fCorValHist[3];
    TProfile*            fFFTCorValHist[3];
    TProfile*            fFFTCorHist[3];
    
    std::vector<std::vector<TProfile*>> fInputWaveHists;
    std::vector<std::vector<TProfile*>> fOutputWaveHists;
    std::vector<std::vector<TProfile*>> fStdCorWaveHists;
    
    std::vector<std::vector<TH1D*>>     fWaveformProfHists;    
    std::vector<std::vector<TProfile*>> fWaveCorHists;
    
    TProfile2D*          fFFTvsMBProf[3];
    
    std::vector<std::map<int,TH1D*>> fCorrectionHistVec;
    std::vector<std::map<int,TH1D*>> fCorrFixedHistVec;
    std::vector<std::map<int,TH1D*>> fErosionHistVec;
    std::vector<std::map<int,TH1D*>> fDilationHistVec;
    std::vector<std::map<int,TH1D*>> fAverageHistVec;
    std::vector<std::map<int,TH1D*>> fDiffHistVec;
    int                              fCorMaxHists;
    
    std::vector<std::set<size_t>> fBadWiresbyViewAndWire;
    
    RawDigitFFTAlg fFFTAlg;
    
    // Useful services, keep copies for now (we can update during begin run periods)
    geo::WireReadoutGeom const* fWireReadoutGeom = &art::ServiceHandle<geo::WireReadout const>()->Get();
};
    
} // end caldata namespace

#endif
