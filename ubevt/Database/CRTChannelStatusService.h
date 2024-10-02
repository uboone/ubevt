/**
 * @file   CRTChannelStatusService.h
 * @brief  Interface for experiment-specific service for channel quality info
 *
 * The schema is the same as for WireReadout in Geometry library
 * (larcore repository).
 * The implementations of this interface can be directly used as art services.
 */

#ifndef CRTCHANNELSTATUSSERVICE_H
#define CRTCHANNELSTATUSSERVICE_H

// LArSoft libraries
#include "larcore/CoreUtils/ServiceUtil.h" // ServiceRequirementsChecker<>
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"

// Framework libraries
#include "art/Framework/Services/Registry/ServiceMacros.h"

//forward declarations
namespace lariov {
  class ChannelStatusProvider;
}

namespace lariov {

  class CRTChannelStatusService {
    
    public:
      
      using provider_type = ChannelStatusProvider;
      
      /// Destructor
      virtual ~CRTChannelStatusService() = default;

      //
      // Actual interface here
      //

      //@{
      /// Returns a reference to the service provider
      ChannelStatusProvider const& GetProvider() const
        { return DoGetProvider(); }
      // will be deprecated:
      ChannelStatusProvider const& GetFilter() const { return GetProvider(); }
      //@}

      //@{
      /// Returns a pointer to the service provider
      ChannelStatusProvider const* GetProviderPtr() const
        { return DoGetProviderPtr(); }
      // will be deprecated:
      ChannelStatusProvider const* GetFilterPtr() const
        { return GetProviderPtr(); }
      //@}
      
      
      ChannelStatusProvider const* provider() const
        { return GetProviderPtr(); }
      
      //
      // end of interface
      //
    
    private:
    
      /// Returns a pointer to the service provider 
      virtual ChannelStatusProvider const* DoGetProviderPtr() const = 0;

      /// Returns a reference to the service provider
      virtual ChannelStatusProvider const& DoGetProvider() const = 0;
    
    
    
  }; // class CRTChannelStatusService
  
} // namespace lariov


DECLARE_ART_SERVICE_INTERFACE(lariov::CRTChannelStatusService, LEGACY)


// check that the requirements for lariov::CRTChannelStatusService are satisfied
template struct lar::details::ServiceRequirementsChecker<lariov::CRTChannelStatusService>;


#endif // CRTCHANNELSTATUSSERVICE_H
