//*********************************************************************************
//
// Name: WireCellChannelStatusService
//
// Purpose: This class is an implementation of the larsoft service interface
//          lariov::ChannelStatusService.
//
//          This implementation does not access a database.  Rather, it obtains
//          channel status information from wire cell data product "badmask."
//
//          The information available from the badmask data product is a list
//          of bad channels, plus a range of bad ticks.  The following status
//          enums are used to represent good and bad channels (refer to
//          larevt/CalibrationDBI/IOVData/ChannelStatus.h)
//
//          good - kGOOD=4
//          bad  - kDEAD=1
//
//          This class uses the PreProcessEvent callback to update its state by
//          reading the badmask data product.  It is not an error if there
//          is no badmask data product when this function is called.  Modules
//          that update the badmask data product may call PreProcessEvent manually.
//
// FCL parameters:
//
//   BadMaskModuleLabel - Module label of badmask data product.
//   UseDBProvider      - Use a secondary database service provider?
//   DBProvider         - FCL configuration of secondary provider.
//
// Created: 2-Oct-2018, H. Greenlee
//
// 
//*********************************************************************************

#include <iostream>
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/WireReadout.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"
#include "ubevt/Database/WireCellChannelStatusProvider.h"

namespace lariov{

  class WireCellChannelStatusService : public ChannelStatusService {
  
  public:

    // Constructors, destructor.
    
    WireCellChannelStatusService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
    ~WireCellChannelStatusService() {}

    // Callbacks.
      
    void PreProcessEvent(const art::Event& evt, art::ScheduleContext);
     
  private:

    // Overrides (base class provider accessors).
    
    const ChannelStatusProvider& DoGetProvider() const override {return fProvider;}    
    const ChannelStatusProvider* DoGetProviderPtr() const override {return &fProvider;}

    // Leaf class provider accessors.

    const WireCellChannelStatusProvider& DoGetWCProvider() const {return fProvider;}    
    const WireCellChannelStatusProvider* DoGetWCProviderPtr() const {return &fProvider;}

    // Fcl parameters.

    std::string fBadMaskModuleLabel;
    bool fUseDBProvider;
    fhicl::ParameterSet fDBProvider;

    // Provider.
      
    WireCellChannelStatusProvider fProvider;
  };
}//end namespace lariov
      
DECLARE_ART_SERVICE_INTERFACE_IMPL(lariov::WireCellChannelStatusService, lariov::ChannelStatusService, LEGACY)
      

namespace lariov{

  // Constructor.

  WireCellChannelStatusService::WireCellChannelStatusService(fhicl::ParameterSet const& pset,
							     art::ActivityRegistry& reg) :
    fBadMaskModuleLabel(pset.get<std::string>("BadMaskModuleLabel")),
    fUseDBProvider(pset.get<bool>("UseDBProvider"))
  {
    // Add secondary database provider?

    if(fUseDBProvider) {
      fDBProvider = pset.get<fhicl::ParameterSet>("DBProvider");
      fProvider.addDBProvider(fDBProvider);
    }

    // Register callback.

    reg.sPreProcessEvent.watch(this, &WireCellChannelStatusService::PreProcessEvent);    
  }
  
  // Callback.  

  void WireCellChannelStatusService::PreProcessEvent(const art::Event& evt, art::ScheduleContext) {

    // Update number of channels.

    unsigned int nchannels = art::ServiceHandle<geo::WireReadout const>()->Get().Nchannels();
    fProvider.updateNumChannels(nchannels);

    // Clear bad channel list.

    fProvider.clearBadMasks();
    
    // Read badmask data product, if any, and update bad channels.

    art::Handle<std::vector<int> > badh;
    evt.getByLabel(fBadMaskModuleLabel, badh);
    if(badh.isValid()) {
      const std::vector<int>& badmasks = *badh;
      fProvider.updateBadMasks(badmasks);
    }
    
  } 
  
  

}//end namespace lariov

DEFINE_ART_SERVICE_INTERFACE_IMPL(lariov::WireCellChannelStatusService, lariov::ChannelStatusService)
