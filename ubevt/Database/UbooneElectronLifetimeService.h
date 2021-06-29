#ifndef UBOONEELECTRONLIFETIMESERVICE_H
#define UBOONEELECTRONLIFETIMESERVICE_H

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "fhiclcpp/ParameterSet.h"
#include "UBElectronLifetimeService.h"
#include "UbooneElectronLifetimeProvider.h"
#include "UbooneCalibrationServiceHelper.h"

namespace lariov{

  /**
     \class UbooneElectronLifetimeService
     art service provider for electron lifetime.  Implements 
     an electron lifetime retrieval service for database scheme in which 
     all elements in a database folder share a common interval of validity
  */
  class UbooneElectronLifetimeService : public UBElectronLifetimeService {
  
    public:
    
      UbooneElectronLifetimeService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
      
      void PreProcessEvent(const art::Event& evt, art::ScheduleContext) {
        fProvider.UpdateTimeStamp( fHelper.GetTimeStamp(evt, "Electron Lifetime" ) );
      }
     
    private:
    
      const UBElectronLifetimeProvider& DoGetProvider() const override {
        return fProvider;
      }

      UbooneElectronLifetimeProvider fProvider;
      UbooneCalibrationServiceHelper fHelper;
  };
}//end namespace lariov
      
DECLARE_ART_SERVICE_INTERFACE_IMPL(lariov::UbooneElectronLifetimeService, lariov::UBElectronLifetimeService,LEGACY)

#endif
