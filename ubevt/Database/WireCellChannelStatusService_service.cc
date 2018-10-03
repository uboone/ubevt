//*********************************************************************************
//
// Name: WireCellChannelStatusService
//
// Purpose: This class is an implementation of the larsoft service interface
//          lariov::ChannelStatusService.
//
//          This implementation does not access a database.  Rather, it obtains
//          channel status information from wire cell data product "badchannel."
//          Additional tick-dependent information is available from wire cell
//          data product "badmask," however, this service and its associated provider
//          only make use of badchannel.
//
//          The information available from the badchannel data product is only
//          "good" or "bad" (i.e. there is no additional information, such as 
//          noisiness).  The following status enums are used to represent good
//          and bad channels (refer to larevt/CalibrationDBI/IOVData/ChannelStatus.h)
//
//          good - kGOOD=4
//          bad  - kDEAD=1
//
//          This class uses the PreProcessEvent callback to update its state by
//          reading the badchannel data product.  It is not an error if there
//          is no badchannel data product when this function is called.  Modules
//          that update the badchannel list may call PreProcessEvent manually.
//
// FCL parameters:
//
//   BadChannelModuleLabel - Module label of badchannel data product.
//
// Created: 2-Oct-2018, H. Greenlee
//
// 
//*********************************************************************************

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
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
      
    void PreProcessEvent(const art::Event& evt); 
     
  private:

    // Overrides.
    
    const ChannelStatusProvider& DoGetProvider() const override {return fProvider;}    
    const ChannelStatusProvider* DoGetProviderPtr() const override {return &fProvider;}

    // Fcl parameters.

    std::string fBadChannelModuleLabel;

    // Provider.
      
    WireCellChannelStatusProvider fProvider;
  };
}//end namespace lariov
      
DECLARE_ART_SERVICE_INTERFACE_IMPL(lariov::WireCellChannelStatusService, lariov::ChannelStatusService, LEGACY)
      

namespace lariov{

  // Constructor.

  WireCellChannelStatusService::WireCellChannelStatusService(fhicl::ParameterSet const& pset,
							     art::ActivityRegistry& reg) :
    fBadChannelModuleLabel(pset.get<std::string>("BadChannelModuleLabel"))
  {
    // Register callback.

    reg.sPreProcessEvent.watch(this, &WireCellChannelStatusService::PreProcessEvent);    
  }
  
  // Callback.  

  void WireCellChannelStatusService::PreProcessEvent(const art::Event& evt) {

    // Update number of channels.

    art::ServiceHandle<geo::Geometry> geo;
    unsigned int nchannels = geo->Nchannels();
    fProvider.updateNumChannels(nchannels);

    // Clear bad channel list.

    fProvider.clearBadChannels();
    
    // Read badchannel data product, if any, and update bad channels.

    art::Handle<std::vector<int> > badh;
    evt.getByLabel(fBadChannelModuleLabel, badh);
    if(badh.isValid()) {

      const std::vector<int>& badchannels = *badh;
      fProvider.updateBadChannels(badchannels);
    }
    
  } 
  
  

}//end namespace lariov

DEFINE_ART_SERVICE_INTERFACE_IMPL(lariov::WireCellChannelStatusService, lariov::ChannelStatusService)
