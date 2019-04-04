////////////////////////////////////////////////////////////////////////
// Class:       RawDigitOverlayMicroBooNEAna
// Module Type: analyzer
// File:        RawDigitOverlayMicroBooNEAna_module.cc
//
// Generated at Nov 20 13:06:09 2015 by Wesley Ketchum using artmod
// from cetpkgsupport v1_08_07.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art_root_io/TFileService.h"

#include "TH1S.h"
#include <sstream>

#include "DataOverlay/RawDigitAdderAna.hh"

namespace mix {
  class RawDigitOverlayMicroBooNEAna;
}

class mix::RawDigitOverlayMicroBooNEAna : public art::EDAnalyzer {
public:
  explicit RawDigitOverlayMicroBooNEAna(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  RawDigitOverlayMicroBooNEAna(RawDigitOverlayMicroBooNEAna const &) = delete;
  RawDigitOverlayMicroBooNEAna(RawDigitOverlayMicroBooNEAna &&) = delete;
  RawDigitOverlayMicroBooNEAna & operator = (RawDigitOverlayMicroBooNEAna const &) = delete;
  RawDigitOverlayMicroBooNEAna & operator = (RawDigitOverlayMicroBooNEAna &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

private:

  std::string                   fRawDigitModule1;
  std::string                   fRawDigitModule2;
  std::string                   fRawDigitModuleSum;
  size_t                        fChannelSampleInterval;
  std::vector<raw::ChannelID_t> fChannelsSpecial;  
  bool                          fPrintBadOverlays;
  std::string                   fInput1Label;
  std::string                   fInput2Label;
  std::string                   fSumLabel;

  mix::RawDigitAdderAna fAnaAlg;
  

};


mix::RawDigitOverlayMicroBooNEAna::RawDigitOverlayMicroBooNEAna(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p),
  fRawDigitModule1(p.get<std::string>("RawDigitModule1Label")),
  fRawDigitModule2(p.get<std::string>("RawDigitModule2Label")),
  fRawDigitModuleSum(p.get<std::string>("RawDigitModuleSumLabel")),
  fChannelSampleInterval(p.get<size_t>("ChannelSampleInterval",100)),
  fChannelsSpecial(p.get< std::vector<raw::ChannelID_t> >("ChannelsToPrint",std::vector<raw::ChannelID_t>())),
  fPrintBadOverlays(p.get<bool>("PrintBadOverlays",true)),
  fInput1Label(p.get<std::string>("Label1",fRawDigitModule1)),
  fInput2Label(p.get<std::string>("Label2",fRawDigitModule2)),
  fSumLabel(p.get<std::string>("LabelSum",fRawDigitModuleSum)),
  fAnaAlg(fChannelSampleInterval,fChannelsSpecial,fPrintBadOverlays,fInput1Label,fInput2Label,fSumLabel)
{
}

void mix::RawDigitOverlayMicroBooNEAna::analyze(art::Event const & e)
{
  art::ServiceHandle<art::TFileService> tfs;

  art::Handle<std::vector<raw::RawDigit> > waveform1Handle,waveform2Handle,waveformSumHandle;
  e.getByLabel(fRawDigitModule1,waveform1Handle);
  e.getByLabel(fRawDigitModule2,waveform2Handle);
  e.getByLabel(fRawDigitModuleSum,waveformSumHandle);
  if(!waveform1Handle.isValid() || !waveform2Handle.isValid() || !waveformSumHandle.isValid()) throw std::exception();
  std::vector<raw::RawDigit> const& waveform1Vector(*waveform1Handle); 
  std::vector<raw::RawDigit> const& waveform2Vector(*waveform2Handle); 
  std::vector<raw::RawDigit> const& waveformSumVector(*waveformSumHandle);

  size_t histos_to_make = fAnaAlg.CheckOverlay(waveform1Vector,waveform2Vector,waveformSumVector);

  std::vector<TH1S*> histograms(histos_to_make);
  for(size_t i_h=0; i_h<histograms.size(); i_h++){
    std::stringstream hname; hname << "h" << i_h;
    tfs->make<TH1S>(hname.str().c_str(),"GenericTitle",1,0,1);
  }
  
  fAnaAlg.CreateOutputHistograms(histograms,
				 waveform1Vector,waveform2Vector,waveformSumVector,
				 e.run(),e.event());
}

DEFINE_ART_MODULE(mix::RawDigitOverlayMicroBooNEAna)
