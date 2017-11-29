////////////////////////////////////////////////////////////////////////
// Class:       FlashExt
// Module Type: analyzer
// File:        FlashExt_module.cc
//
// Generated at Thu Sep 14 11:16:14 2017 by David Lorca Galindo using artmod
// from cetpkgsupport v1_11_00.
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

#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/OpFlash.h"
#include "lardataobj/RecoBase/PFParticle.h"
#include "lardataobj/AnalysisBase/T0.h"
#include "lardataobj/AnalysisBase/CosmicTag.h"
#include "lardata/Utilities/AssociationUtil.h"

#include "bernfebdaq-core/Overlays/BernZMQFragment.hh"
#include "artdaq-core/Data/Fragments.hh"

#include "art/Framework/Services/Optional/TFileService.h"

#include "uboone/CRT/CRTProducts/CRTHit.hh"
#include "uboone/CRT/CRTProducts/CRTTrack.hh"
#include "uboone/CRT/CRTAuxFunctions.hh"

#include "uboone/RawData/utils/DAQHeaderTimeUBooNE.h"

#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3S.h"
#include "TProfile.h"
#include "TF1.h"
#include "TDatime.h"
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <typeinfo>


namespace crt {
  class FlashExt;
}

class crt::FlashExt : public art::EDAnalyzer {
public:
  explicit FlashExt(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  FlashExt(FlashExt const &) = delete;
  FlashExt(FlashExt &&) = delete;
  FlashExt & operator = (FlashExt const &) = delete;
  FlashExt & operator = (FlashExt &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

  // Selected optional functions.
  void beginJob() override;
  void endJob() override;

private:

  // Declare member data here.
  art::ServiceHandle<art::TFileService> tfs;

  uint32_t fEvtNum; //Number of current event                       
  uint32_t frunNum;                //Run Number taken from event  
  uint32_t fsubRunNum;             //Subrun Number taken from event 
  std::string  data_label_flash_;
  std::string  data_label_DAQHeader_;
  int verbose_;


  TTree*       fTree;
  TH1F* hFlashTimeDis;
  TH1F* htmstp_diff;
  TProfile* htmstp_diff_vs_event;
  
  //for Tree
  uint32_t fTriTim_sec;
  uint32_t fTriTim_nsec;
  double fY;
  double fZ;
  double fPEflash;
  double fTimFla;
  double fAbsTimFla;
  int fbeam;
  //for Tree

};


crt::FlashExt::FlashExt(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p),
  data_label_flash_(p.get<std::string>("data_label_flash_")),
  data_label_DAQHeader_(p.get<std::string>("data_label_DAQHeader_")),
  verbose_(p.get<int>("verbose"))//,
 // More initializers here.
{
  fTree = tfs->make<TTree>("flash_tree","Flash_Tree");
  fTree->Branch("event", &fEvtNum, "event/I");
  fTree->Branch("beam", &fbeam, "beam/I");
  fTree->Branch("trigger_ts", &fTriTim_sec, "Time (s)/I");
  fTree->Branch("trigger_tns", &fTriTim_nsec, "Time (ns)/I");
  fTree->Branch("Y_reco", &fY, "Y (cm)/D");
  fTree->Branch("Z_reco", &fZ, "Z (cm)/D");
  fTree->Branch("N_PE", &fPEflash, "Photoelectrons/D");
  fTree->Branch("T_flash", &fTimFla, "Flash time w.r.t trigger (us)/D");
  fTree->Branch("Abs_T_flash", &fAbsTimFla, "Absolute flash time (ns)/D");
}

void crt::FlashExt::analyze(art::Event const & evt)
{
  // Implementation of required member function here.

  frunNum    = evt.run();
  fsubRunNum = evt.subRun();
  fEvtNum = evt.event();

  art::Timestamp evtTime = evt.time();
  auto evt_time_sec = evtTime.timeHigh();
  auto evt_time_nsec = evtTime.timeLow();

  //get DAQ Header
  art::Handle< raw::DAQHeaderTimeUBooNE > rawHandle_DAQHeader;
  evt.getByLabel(data_label_DAQHeader_, rawHandle_DAQHeader);

  //check to make sure the data we asked for is valid                                                                                          
  if(!rawHandle_DAQHeader.isValid()){
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << evt.event() << " has zero"
              << " DAQHeaderTimeUBooNE  " << " in with label " << data_label_DAQHeader_ << std::endl;
    return;
  }  

  raw::DAQHeaderTimeUBooNE const& my_DAQHeader(*rawHandle_DAQHeader);

  art::Timestamp evtTimeGPS = my_DAQHeader.gps_time();  
  double evt_timeGPS_sec = evtTimeGPS.timeHigh();
  double evt_timeGPS_nsec = evtTimeGPS.timeLow();
  fTriTim_sec = evtTimeGPS.timeHigh();
  fTriTim_nsec = evtTimeGPS.timeLow();

  art::Timestamp evtTimeNTP = my_DAQHeader.ntp_time();  
  double evt_timeNTP_sec = evtTimeNTP.timeHigh();
  double evt_timeNTP_nsec = evtTimeNTP.timeLow();
  
  double timstp_diff = std::abs(evt_timeGPS_nsec - evt_timeNTP_nsec);
  htmstp_diff->Fill(timstp_diff);
  htmstp_diff_vs_event->Fill(fEvtNum,timstp_diff);

  if(verbose_==1){ 
  std::cout.precision(19);
  std::cout<<"  GPS time nano_second:  "<<evt_timeGPS_nsec<<std::endl; 
  std::cout<<"  NTP time nano_second:  "<<evt_timeNTP_nsec<<std::endl; 
  
  std::cout<<"  difference:  "<<evt_timeGPS_nsec - evt_timeNTP_nsec<<std::endl; 
  std::cout<<"  ABS difference:  "<<abs(evt_timeGPS_nsec - evt_timeNTP_nsec)<<std::endl; 
  std::cout<<"  timstp_diff:  "<<timstp_diff<<std::endl; 
  getchar();
  }
  
  if(verbose_==1){ 
    std::cout<<"  GPS time second:  "<<evt_timeGPS_sec<<std::endl; 
    std::cout<<"  GPS time nano_second:  "<<evt_timeGPS_nsec<<std::endl; 
    std::cout<<"  NTP time second:  "<<evt_timeNTP_sec<<std::endl; 
    std::cout<<"  NTP time nano_second:  "<<evt_timeNTP_nsec<<std::endl; 
    std::cout<<"  event time second:  "<<evt_time_sec<<std::endl; 
    std::cout<<"  event time nano_second:  "<<evt_time_nsec<<std::endl; 
    std::cout<<"  difference between GPS and NTP:  "<<evt_timeGPS_nsec - evt_timeNTP_nsec<<" ns"<<std::endl; 
    std::cout<<"  ABS difference between GPS and NTP:  "<<timstp_diff<<" ns"<<std::endl; 

    if( (evt_time_sec==evt_timeGPS_sec) && (evt_time_nsec==evt_timeGPS_nsec))  std::cout<<" Event time type is: GPS  "<<std::endl; 
    if( (evt_time_sec==evt_timeNTP_sec) && (evt_time_nsec==evt_timeNTP_nsec))  std::cout<<" Event time type is: NTP  "<<std::endl; 

    getchar();
  }


  
  //get Optical Flash
  art::Handle< std::vector<recob::OpFlash> > rawHandle_OpFlash;
  evt.getByLabel(data_label_flash_, rawHandle_OpFlash);
  
  std::vector<recob::OpFlash> const& OpFlashCollection(*rawHandle_OpFlash);
  if(verbose_==1){ 
    std::cout<<"  OpFlashCollection.size()  "<<OpFlashCollection.size()<<std::endl; 
    getchar();
  }
  //get Optical Flash

  for(std::vector<int>::size_type i = 0; i != OpFlashCollection.size(); i++) {//A

    recob::OpFlash my_OpFlash = OpFlashCollection[i];

    auto Timeflash = my_OpFlash.Time(); //in us from trigger time
    hFlashTimeDis->Fill(Timeflash);      
    
    fY = my_OpFlash.YCenter();
    fZ = my_OpFlash.ZCenter();
    fPEflash = my_OpFlash.TotalPE();
    fTimFla = my_OpFlash.Time();
    fAbsTimFla = (evt_timeGPS_sec*1e9) + evt_timeGPS_nsec + (Timeflash * 1000);
    fbeam = my_OpFlash.OnBeamTime();



    if(verbose_==1){ 
      std::cout.precision(19);
      std::cout<<" Run:  "<<frunNum<<std::endl; 
      std::cout<<" SubRun:  "<<fsubRunNum<<std::endl; 
      std::cout<<" Event:  "<<fEvtNum<<std::endl; 
      std::cout<<" Trigger second:  "<<fTriTim_sec<<std::endl; 
      std::cout<<" Trigger nanosecond:  "<<fTriTim_nsec<<std::endl; 
      std::cout<<" Flash time w.r.t Trigger (us):  "<<Timeflash<<std::endl; 
      std::cout<<" Flash time w.r.t Trigger (ns):  "<<Timeflash*1000<<std::endl; 
      std::cout<<" Absolute time of flash time (ns):  "<<fAbsTimFla<<std::endl; 
      std::cout<<" is beam::  "<<fbeam<<std::endl;
      std::cout<<" Total PE:  "<<fPEflash<<std::endl;  
      getchar();
    }
    
    fTree->Fill();
  }//A


}

void crt::FlashExt::beginJob()
{
  // Implementation of optional member function here.

  hFlashTimeDis = tfs->make<TH1F>("hFlashTimDis","hFlashTimDis",3500,-3500,3500);
  hFlashTimeDis->GetXaxis()->SetTitle("Flash Time w.r.t. trigger (us)");
  hFlashTimeDis->GetYaxis()->SetTitle("Entries/bin");

  htmstp_diff = tfs->make<TH1F>(" htmstp_diff"," htmstp_diff",20000,0,20000000);
  htmstp_diff->GetXaxis()->SetTitle("abs(NTP - GPS)  (ns)");
  htmstp_diff->GetYaxis()->SetTitle("Entries/bin");

  htmstp_diff_vs_event = tfs->make<TProfile>(" htmstp_diff_vs_event"," htmstp_diff_vs_event",17000,0,17000,"s");
  htmstp_diff_vs_event->GetXaxis()->SetTitle("Event ID");
  htmstp_diff_vs_event->GetYaxis()->SetTitle("abs(NTP - GPS)  (ns)");


}

void crt::FlashExt::endJob()
{
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(crt::FlashExt)
