////////////////////////////////////////////////////////////////////////
// Class:       OverlayRawDataDetailMicroBooNE
// Module Type: producer
// File:        OverlayRawDataDetailMicroBooNE_module.cc
//
// This borrows a lot from the Mu2e mixing module:
//      EventMixing/src/MixMCEvents_module.cc
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Persistency/Common/CollectionUtilities.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "canvas/Utilities/InputTag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "larcore/Geometry/Geometry.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <exception>
#include <sstream>
#include <unistd.h>

#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"
#include "ubevt/Utilities/PMTRemapService.h"
#include "ubevt/Utilities/PMTRemapProvider.h"
#include "larevt/CalibrationDBI/Interface/PmtGainService.h"
#include "larevt/CalibrationDBI/Interface/PmtGainProvider.h"

#include "ubevt/DataOverlay/DataOverlay/RawDigitMixer.h"
#include "ubevt/DataOverlay/DataOverlay/CRTMixer.h"
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RawData/TriggerData.h"
#include "larcoreobj/SimpleTypesAndConstants/RawTypes.h"

#include "ubobj/CRT/CRTSimData.hh"
#include "ubobj/CRT/CRTHit.hh"

#include "ubevt/DataOverlay/DataOverlay/OpDetWaveformMixer.h"
#include "lardataobj/RawData/OpDetWaveform.h"
#include "larevt/CalibrationDBI/Interface/PmtGainService.h"
#include "larevt/CalibrationDBI/Interface/PmtGainProvider.h"

namespace mix {

class OverlayRawDataMicroBooNE : public art::EDProducer {
  public:

    OverlayRawDataMicroBooNE(fhicl::ParameterSet const& p);
    ~OverlayRawDataMicroBooNE() {}

    void produce (art::Event& evt);

  private:

    // Declare member data here.
    RawDigitMixer              fRDMixer;
    OpDetWaveformMixer         fODMixer;
    CRTMixer                   fCRTMixer;

    short                fDefaultRawDigitSatPoint;
    short                fDefaultOpDetSatPoint;
    size_t               fOpDetMinSampleSize;

    std::string          fRawDigitDataModuleLabel;
    std::string          fOpDetDataModuleLabel;
    std::string          fTriggerDataModuleLabel;
    std::string          fRawDigitMCModuleLabel;
    std::string          fOpDetMCModuleLabel;
    std::string          fTriggerMCModuleLabel;
    std::string          fCRTMCModuleLabel;
    std::string          fCRTDataModuleLabel;

    float                fDefaultMCRawDigitScale;
    float                fDefaultMCOpDetScale;

    float                fOpDetConstantSimulatedGain;


    bool fDoTPCMixing;
    bool fDoPMTMixing;
    bool fDoCRTMixing;

    void GenerateMCRawDigitScaleMap(std::vector<raw::RawDigit> const&);
    std::unordered_map<raw::ChannelID_t,float> fMCRawDigitScaleMap;
    
    void GenerateMCOpDetGainScaleMap();
    std::unordered_map<raw::Channel_t,float> fMCOpDetGainScaleMap;

    void GenerateMCOpDetHighGainScaleMap(std::vector<raw::OpDetWaveform> const&);
    std::unordered_map<raw::Channel_t,float> fMCOpDetHighGainScaleMap;

    void GenerateMCOpDetLowGainScaleMap(std::vector<raw::OpDetWaveform> const&);
    std::unordered_map<raw::Channel_t,float> fMCOpDetLowGainScaleMap;

    bool MixRawDigits( const art::Event& evt, std::vector<raw::RawDigit> & output);

    bool MixCRTHits( const art::Event& evt, std::vector<crt::CRTHit> & output);
    
    bool MixTriggerData( const art::Event& evt, std::vector<raw::Trigger> & output);

    bool MixOpDetWaveforms_HighGain( const art::Event& evt, std::vector<raw::OpDetWaveform> & output);
    bool MixOpDetWaveforms_LowGain( const art::Event& evt, std::vector<raw::OpDetWaveform> & output);

    void remapPMT(const std::vector<raw::OpDetWaveform> & vODW, std::vector<raw::OpDetWaveform> & corrected_vODW);
  };
}//end namespace mix


mix::OverlayRawDataMicroBooNE::OverlayRawDataMicroBooNE(fhicl::ParameterSet const& p)
  : EDProducer{p},
  fRDMixer(false), //print warnings turned off
  fODMixer(false), //print warnings turned off
  fCRTMixer(), //print warnings turned off

  fDefaultRawDigitSatPoint(p.get<short>("DefaultRawDigitSaturationPoint",4096)),
  fDefaultOpDetSatPoint(p.get<short>("DefaultOpDetSaturationPoint",4096)),
  fOpDetMinSampleSize(p.get<size_t>("OpDetMinSampleSize",100)),
  fRawDigitDataModuleLabel(p.get<std::string>("RawDigitDataModuleLabel","")),
  fOpDetDataModuleLabel(p.get<std::string>("OpDetDataModuleLabel","")),
  fTriggerDataModuleLabel(p.get<std::string>("TriggerDataModuleLabel","")),
  fRawDigitMCModuleLabel(p.get<std::string>("RawDigitMCModuleLabel","")),
  fOpDetMCModuleLabel(p.get<std::string>("OpDetMCModuleLabel","")),
  fTriggerMCModuleLabel(p.get<std::string>("TriggerMCModuleLabel","")),
  fCRTMCModuleLabel(p.get<std::string>("CRTMCModuleLabel","")),
  fCRTDataModuleLabel(p.get<std::string>("CRTDataModuleLabel","")),
  fDefaultMCRawDigitScale(p.get<float>("DefaultMCRawDigitScale",1)),
  fDefaultMCOpDetScale(p.get<float>("DefaultMCOpDetScale",1)),
  fOpDetConstantSimulatedGain(p.get<float>("OpDetConstantSimulatedGain",1))
{
  fRDMixer.SetSaturationPoint(fDefaultRawDigitSatPoint);
  fODMixer.SetSaturationPoint(fDefaultOpDetSatPoint);
  fODMixer.SetMinSampleSize(fOpDetMinSampleSize);
  
  if(fRawDigitDataModuleLabel.size()==0 && fRawDigitMCModuleLabel.size()==0)
    fDoTPCMixing = false;
  else
    fDoTPCMixing = true;

  if(fOpDetDataModuleLabel.size()==0 && fTriggerDataModuleLabel.size()==0 &&
     fOpDetMCModuleLabel.size()==0 && fTriggerMCModuleLabel.size()==0)
    fDoPMTMixing = false;
  else 
    fDoPMTMixing = true;

  if(fCRTDataModuleLabel.size()==0 && fCRTDataModuleLabel.size()==0)
    fDoCRTMixing = false;
  else
    fDoCRTMixing = true;

  if(fDoTPCMixing) produces< std::vector<raw::RawDigit> >();
  if(fDoCRTMixing) produces< std::vector<crt::CRTHit> >();
  if(fDoPMTMixing){
    produces< std::vector<raw::OpDetWaveform> >("OpdetBeamHighGain");
    produces< std::vector<raw::OpDetWaveform> >("OpdetBeamLowGain");
    produces< std::vector<raw::Trigger> >();
  }
}

void mix::OverlayRawDataMicroBooNE::produce(art::Event& evt) {

  if(fDoTPCMixing){
    std::unique_ptr<std::vector<raw::RawDigit> >     rawdigits(new std::vector<raw::RawDigit>);
    MixRawDigits(evt, *rawdigits);
    evt.put(std::move(rawdigits));
  }

  if(fDoCRTMixing){
    std::unique_ptr<std::vector<crt::CRTHit> >       crthits(new std::vector<crt::CRTHit>);
    MixCRTHits(evt, *crthits);
    evt.put(std::move(crthits));
  }

  if(fDoPMTMixing){
    std::unique_ptr<std::vector<raw::OpDetWaveform> > opdet_hg(new std::vector<raw::OpDetWaveform>);
    std::unique_ptr<std::vector<raw::OpDetWaveform> > opdet_lg(new std::vector<raw::OpDetWaveform>);
    std::unique_ptr<std::vector<raw::Trigger> > triggerdata(new std::vector<raw::Trigger>);
    
    GenerateMCOpDetGainScaleMap(); // Right now the gain scale map is the same for low and high gain readout and it is obtained before mixing any of them
    MixOpDetWaveforms_HighGain(evt, *opdet_hg);
    MixOpDetWaveforms_LowGain(evt, *opdet_lg);
    MixTriggerData(evt, *triggerdata);
    
    evt.put(std::move(opdet_hg),"OpdetBeamHighGain");
    evt.put(std::move(opdet_lg),"OpdetBeamLowGain");
    evt.put(std::move(triggerdata));
  }
}


void mix::OverlayRawDataMicroBooNE::GenerateMCRawDigitScaleMap(std::vector<raw::RawDigit> const& dataDigitVector){
  //right now, assume the number of channels is the number in the collection
  //and, loop through the channels one by one to get the right channel number
  //note: we will put here access to the channel database to determine dead channels
  fMCRawDigitScaleMap.clear();

  ///Wes, 12 March 2019
  ///Look, we do this via wirecell now and the ZeroedOutChannels module and etc.
  ///So, remove the DB access and lookup here, and set the scale on everything to be the default (1.0!)

  //const lariov::ChannelStatusProvider& chanStatus = art::ServiceHandle<lariov::ChannelStatusService>()->GetProvider();
  
  for(auto const& d : dataDigitVector){
    //if(chanStatus.IsBad(d.Channel()))
    //fMCRawDigitScaleMap[d.Channel()] = 0.0;
    //else
      fMCRawDigitScaleMap[d.Channel()] = fDefaultMCRawDigitScale;
  }
  
}

bool mix::OverlayRawDataMicroBooNE::MixRawDigits( const art::Event& event, std::vector<raw::RawDigit> & output) {
  
  output.clear();
  
  art::Handle< std::vector<raw::RawDigit> > mcDigitHandle;
  event.getByLabel( fRawDigitMCModuleLabel,mcDigitHandle);
  
  art::Handle< std::vector<raw::RawDigit> > dataDigitHandle;
  event.getByLabel( fRawDigitDataModuleLabel,dataDigitHandle);


  GenerateMCRawDigitScaleMap(*dataDigitHandle);
  fRDMixer.DeclareData(*dataDigitHandle);
  fRDMixer.Mix(*mcDigitHandle,fMCRawDigitScaleMap);
  fRDMixer.FillRawDigitOutput(output);
  
  return true;
}

bool mix::OverlayRawDataMicroBooNE::MixCRTHits( const art::Event& event, std::vector<crt::CRTHit> & output) {

  output.clear();

  std::unique_ptr<std::vector<crt::CRTHit>> dummyInput(new std::vector<crt::CRTHit>);

  art::Handle< std::vector<crt::CRTHit> > mcCRTHandle;
  event.getByLabel( fCRTMCModuleLabel,mcCRTHandle);

  art::Handle< std::vector<crt::CRTHit> > dataCRTHandle;
  event.getByLabel( fCRTDataModuleLabel,dataCRTHandle);

  if(!dataCRTHandle.isValid()) {
    std::cout<<"NO CRT INFO in the input data file - NO CRT MIXING DONE!"<<std::endl;
    return false;
  }

  std::vector<crt::CRTHit> const& mcCRTInputVec = (mcCRTHandle.isValid())? *mcCRTHandle : *dummyInput;

  fCRTMixer.Mix(mcCRTInputVec,*dataCRTHandle,output);
  return true;
}

void mix::OverlayRawDataMicroBooNE::GenerateMCOpDetGainScaleMap(){
  //right now, same scales are given to the high and low gain readouts 
  //and, loop through the channels one by one to get the right channel number
  //note: we will put here access to the channel database to determine dead channels
  fMCOpDetGainScaleMap.clear();
  //art::ServiceHandle<geo::Geometry> geo;
  const lariov::PmtGainProvider& gain_provider = art::ServiceHandle<lariov::PmtGainService>()->GetProvider();
  //for (unsigned int i=0; i!= geo->NOpDets(); ++i) {
  for (unsigned int i=0; i<32 ; i++) {
    //if (geo->IsValidOpChannel(i) && i<32) {
        fMCOpDetGainScaleMap[i] = gain_provider.ExtraInfo(i).GetFloatData("amplitude_gain");
    //}
  }
}

void mix::OverlayRawDataMicroBooNE::GenerateMCOpDetHighGainScaleMap(std::vector<raw::OpDetWaveform> const& dataVector){
  //right now, assume the number of channels is the number in the collection
  //and, loop through the channels one by one to get the right channel number
  //note: we will put here access to the channel database to determine dead channels
  fMCOpDetHighGainScaleMap.clear();
  for(auto const& d : dataVector)
     fMCOpDetHighGainScaleMap[d.ChannelNumber()] = fDefaultMCOpDetScale;
}

void mix::OverlayRawDataMicroBooNE::GenerateMCOpDetLowGainScaleMap(std::vector<raw::OpDetWaveform> const& dataVector){
  //right now, assume the number of channels is the number in the collection
  //and, loop through the channels one by one to get the right channel number
  //note: we will put here access to the channel database to determine dead channels
  fMCOpDetLowGainScaleMap.clear();
  for(auto const& d : dataVector)
     fMCOpDetLowGainScaleMap[d.ChannelNumber()] = fDefaultMCOpDetScale;
}

bool mix::OverlayRawDataMicroBooNE::MixOpDetWaveforms_HighGain( const art::Event& event, std::vector<raw::OpDetWaveform> & output) {
  
  output.clear();
  
  art::Handle< std::vector<raw::OpDetWaveform> > mcOpDetHandle_HighGain;
  event.getByLabel(fOpDetMCModuleLabel,"OpdetBeamHighGain",mcOpDetHandle_HighGain);
  //remapping MC PMTs to match those from data
  std::unique_ptr<std::vector<raw::OpDetWaveform> > corr_mcOpDetHandle_HighGain(new std::vector<raw::OpDetWaveform>);
  remapPMT(*mcOpDetHandle_HighGain, *corr_mcOpDetHandle_HighGain);
  
  art::Handle< std::vector<raw::OpDetWaveform> > dataOpDetHandle_HighGain;
  event.getByLabel(fOpDetDataModuleLabel,"OpdetBeamHighGain",dataOpDetHandle_HighGain);  

  //GenerateMCOpDetHighGainScaleMap(*dataOpDetHandle_HighGain); 
  fODMixer.DeclareData(*dataOpDetHandle_HighGain,output);
  //fODMixer.Mix(*mcOpDetHandle_HighGain, fMCOpDetHighGainScaleMap, output);
  //fODMixer.Mix(*corr_mcOpDetHandle_HighGain, fMCOpDetHighGainScaleMap, output);
  fODMixer.Mix(*corr_mcOpDetHandle_HighGain, fMCOpDetGainScaleMap, output, fOpDetConstantSimulatedGain);
  
  return true;
}

bool mix::OverlayRawDataMicroBooNE::MixOpDetWaveforms_LowGain( const art::Event& event, std::vector<raw::OpDetWaveform> & output) {

  output.clear();

  art::Handle< std::vector<raw::OpDetWaveform> > mcOpDetHandle_LowGain;
  event.getByLabel(fOpDetMCModuleLabel,"OpdetBeamLowGain",mcOpDetHandle_LowGain);
  //remapping MC PMTs to match those from data
  std::unique_ptr<std::vector<raw::OpDetWaveform> > corr_mcOpDetHandle_LowGain(new std::vector<raw::OpDetWaveform>); 
  remapPMT(*mcOpDetHandle_LowGain, *corr_mcOpDetHandle_LowGain);
 
  art::Handle< std::vector<raw::OpDetWaveform> > dataOpDetHandle_LowGain;
  event.getByLabel(fOpDetDataModuleLabel,"OpdetBeamLowGain",dataOpDetHandle_LowGain);  

  //GenerateMCOpDetLowGainScaleMap(*dataOpDetHandle_LowGain); 
  fODMixer.DeclareData(*dataOpDetHandle_LowGain,output);
  //fODMixer.Mix(*mcOpDetHandle_LowGain, fMCOpDetLowGainScaleMap, output);
  //fODMixer.Mix(*corr_mcOpDetHandle_LowGain, fMCOpDetLowGainScaleMap, output);
  fODMixer.Mix(*corr_mcOpDetHandle_LowGain, fMCOpDetGainScaleMap, output, fOpDetConstantSimulatedGain);
  
  return true;
}

bool mix::OverlayRawDataMicroBooNE::MixTriggerData( const art::Event& event, std::vector<raw::Trigger> & output) {
  
  output.clear();
  
  art::Handle< std::vector<raw::Trigger> > mcTriggerHandle;
  event.getByLabel(fTriggerMCModuleLabel, mcTriggerHandle);
  
  art::Handle< std::vector<raw::Trigger> > dataTriggerHandle;
  event.getByLabel(fTriggerDataModuleLabel, dataTriggerHandle);
  
  unsigned int trig_num; double trig_time,gate_time; unsigned int trig_bits; 
  trig_num  = dataTriggerHandle->at(0).TriggerNumber();
  trig_time = dataTriggerHandle->at(0).TriggerTime();
  gate_time = dataTriggerHandle->at(0).BeamGateTime();
  trig_bits = dataTriggerHandle->at(0).TriggerBits() | mcTriggerHandle->at(0).TriggerBits();
  output.emplace_back(trig_num,trig_time,gate_time,trig_bits);
  
  return true;
}

void mix::OverlayRawDataMicroBooNE::remapPMT(const std::vector<raw::OpDetWaveform> & vODW, std::vector<raw::OpDetWaveform> & corrected_vODW) {
  const util::PMTRemapProvider& pmtremap 
	= art::ServiceHandle<util::PMTRemapService>()->GetProvider();
  
  for( unsigned int i = 0 ; i< vODW.size() ; i++) {
      raw::OpDetWaveform odw = vODW[i];
      raw::Channel_t correctedChannelNumber = pmtremap.OriginalOpChannel(odw.ChannelNumber ()); 
      corrected_vODW.push_back(odw);
      corrected_vODW[i].SetChannelNumber(correctedChannelNumber);
  }
}

DEFINE_ART_MODULE(mix::OverlayRawDataMicroBooNE)
