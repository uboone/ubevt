#ifndef UBOONELIGHTYIELDSERVICE_CC
#define UBOONELIGHTYIELDSERVICE_CC

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "fhiclcpp/ParameterSet.h"
#include "LightYieldService.h"
#include "LightYieldProvider.h"
#include "UbooneLightYieldProvider.h"
#include "UbooneCalibrationServiceHelper.h"

namespace lariov{

  /**
     \class UbooneLightYieldService
     adding these comments is so tedious...
  */
  class UbooneLightYieldService : public LightYieldService {
  
    public:
    
      UbooneLightYieldService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
      
      void PreProcessEvent(const art::Event& evt, art::ScheduleContext);
     
    private:
    
      LightYieldProvider const& DoGetProvider() const override {
        return fProvider;
      }   
      
      LightYieldProvider const* DoGetProviderPtr() const override {
        return &fProvider; 
      }
    
      UbooneLightYieldProvider fProvider;
      UbooneCalibrationServiceHelper fHelper;
  };
}//end namespace lariov
      
DECLARE_ART_SERVICE_INTERFACE_IMPL(lariov::UbooneLightYieldService, lariov::LightYieldService, LEGACY)
      

namespace lariov{

  UbooneLightYieldService::UbooneLightYieldService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg) 
  : fProvider(pset.get<fhicl::ParameterSet>("LightYieldProvider")),
    fHelper(pset.get<fhicl::ParameterSet>("CalibrationHelper"))
  {
    //register callback to update local database cache before each event is processed
    reg.sPreProcessEvent.watch(this, &UbooneLightYieldService::PreProcessEvent);
  }
  
  
  void UbooneLightYieldService::PreProcessEvent(const art::Event& evt, art::ScheduleContext) {
    
    fProvider.UpdateTimeStamp( fHelper.GetTimeStamp(evt, "PMT Gain") );
  } 
  
}//end namespace lariov

DEFINE_ART_SERVICE_INTERFACE_IMPL(lariov::UbooneLightYieldService, lariov::LightYieldService)

#endif
