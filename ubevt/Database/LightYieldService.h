/**
 * @file   LightYieldService.h
 * @brief  Interface for experiment-specific service for relative light yield info
 * @author Ralitsa Sharankova (ralitsa.sharankova@tufts.edu)
 * @date   June 19, 2019
 */

#ifndef LIGHTYIELDSERVICE_H
#define LIGHTYIELDSERVICE_H

// Framework libraries
#include "art/Framework/Services/Registry/ServiceMacros.h"

//forward declarations
namespace lariov {
  class LightYieldProvider;
}

namespace lariov {

  /**
   \class LightYieldService
   This service provides only a simple interface to a provider class
   */
  class LightYieldService {
      
    public:
      using provider_type = LightYieldProvider;
   
      /// Destructor
      virtual ~LightYieldService() = default;

      //retrieve provider
      LightYieldProvider const& GetProvider() const
      { return DoGetProvider(); }
	
      LightYieldProvider const* GetProviderPtr() const
      { return DoGetProviderPtr(); }
    
    private:

      /// Returns a reference to the service provider
      virtual LightYieldProvider const& DoGetProvider() const = 0;
      
      virtual LightYieldProvider const* DoGetProviderPtr() const = 0;
    
    
    
  }; // class LightYieldService
} // namespace lariov


DECLARE_ART_SERVICE_INTERFACE(lariov::LightYieldService, LEGACY)

#endif
