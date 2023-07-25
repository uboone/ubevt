#include "UbooneElectronLifetimeService.h"

namespace lariov{

  UbooneElectronLifetimeService::UbooneElectronLifetimeService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg) 
  : fProvider(pset.get<fhicl::ParameterSet>("ElectronLifetimeProvider")),
    fHelper(pset.get<fhicl::ParameterSet>("CalibrationHelper"))
  {
    //register callback to update local database cache before each event is processed
    reg.sPreProcessEvent.watch(this, &UbooneElectronLifetimeService::PreProcessEvent);
  }

}//end namespace lariov
