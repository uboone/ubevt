#ifndef UBOONECRTCHANNELSTATUSSERVICE_CC
#define UBOONECRTCHANNELSTATUSSERVICE_CC

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"
//#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "ubevt/Database/CRTChannelStatusService.h"
#include "larevt/CalibrationDBI/Providers/SIOVChannelStatusProvider.h"
#include "UbooneCalibrationServiceHelper.h"
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "larcore/Geometry/Geometry.h"

#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RawData/raw.h"

namespace lariov{

  /**
     \class CRTChannelStatusService
     art service implementation of CRTChannelStatusService.  Implements 
     a channel status retrieval service for database scheme in which 
     all elements in a database folder share a common interval of validity
  */
  class UbooneCRTChannelStatusService : public CRTChannelStatusService {
  
    public:
    
      UbooneCRTChannelStatusService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
      ~UbooneCRTChannelStatusService(){}
      
      void PreProcessEvent(const art::Event& evt, art::ScheduleContext);
     
    private:
    
      const ChannelStatusProvider& DoGetProvider() const override {
        return fProvider;
      }    
      
      const ChannelStatusProvider* DoGetProviderPtr() const override {
        return &fProvider;
      }
      
    
      SIOVChannelStatusProvider fProvider;
      UbooneCalibrationServiceHelper fHelper;
  };
}//end namespace lariov
      
DECLARE_ART_SERVICE_INTERFACE_IMPL(lariov::UbooneCRTChannelStatusService, lariov::CRTChannelStatusService, LEGACY)
      

namespace lariov{

  UbooneCRTChannelStatusService::UbooneCRTChannelStatusService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg) 
  : fProvider(pset.get<fhicl::ParameterSet>("ChannelStatusProvider")), 
    fHelper(pset.get<fhicl::ParameterSet>("CalibrationHelper"))
  {
        
    //register callback to update local database cache before each event is processed
    reg.sPreProcessEvent.watch(this, &UbooneCRTChannelStatusService::PreProcessEvent);
    
  }
  
  
  void UbooneCRTChannelStatusService::PreProcessEvent(const art::Event& evt, art::ScheduleContext) {
    
    fProvider.UpdateTimeStamp( fHelper.GetTimeStamp(evt, "CRT Channel Status") );

  } 
  
}//end namespace lariov

DEFINE_ART_SERVICE_INTERFACE_IMPL(lariov::UbooneCRTChannelStatusService, lariov::CRTChannelStatusService)

#endif
