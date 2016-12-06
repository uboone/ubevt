
#include "uboone/CRT/CRTData.hh"
#include "uboone/CRT/CRTDetSim.hh"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/Simulation/AuxDetSimChannel.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "larcore/Geometry/AuxDetGeometry.h"

#include <vector>
#include <memory>
#include <sstream>
#include <string>


namespace crt{

  std::string __log_name__ = "CRTDetSim";

  CRTDetSim::CRTDetSim(const fhicl::ParameterSet& pSet){
    produces< CRTData >();
    mf::LogInfo(__log_name__)<<"In construction: ";
  }

  CRTDetSim::~CRTDetSim(){
    mf::LogInfo(__log_name__)<<"In destruction: ";
  }

  void CRTDetSim::produce(art::Event& evt){

    std::unique_ptr< std::vector<CRTData> > hits;
    mf::LogInfo(__log_name__)<<"In produce ";
    art::ServiceHandle<geo::AuxDetGeometry> geo;
    art::Handle< std::vector<sim::AuxDetSimChannel> > channels;
    evt.getByLabel("largeant",channels);
    if(!channels.isValid()){
      mf::LogWarning(__log_name__)<<"Cannot get the AuxDetChannels";
      return;
    }

    mf::LogInfo(__log_name__)<<" Number of Channels Hit: "<<channels->size();

    for(auto it = channels->begin(); it!= channels->end(); ++it){
      uint32_t id = it->AuxDetID();
      uint32_t sens_id = it->AuxDetSensitiveID();
      mf::LogInfo(__log_name__)<<"Found AuxDetData: "<<id<<" , "<<sens_id;
      std::vector< sim::AuxDetIDE > ides = it->AuxDetIDEs();

      for(auto ideIt = ides.begin(); ideIt!= ides.end(); ++it){
        float t0 = (uint32_t) ideIt->entryT;
        float t1 = (uint32_t) ideIt->exitT;
        float adc = (uint32_t) ideIt->energyDeposited; 
        CRTData dat(sens_id, t0, t1, adc);
        hits.append(dat);
      }
    }
    evt.put(std::move(hits));
    mf::LogWarning(__log_name__)<<"Hit the end of the event list";
  }

  DEFINE_ART_MODULE(CRTDetSim)
}
