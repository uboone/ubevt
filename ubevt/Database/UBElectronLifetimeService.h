#ifndef UBELECTRONLIFETIMESERVICE_H
#define UBELECTRONLIFETIMESERVICE_H

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "larcore/CoreUtils/ServiceUtil.h"

//forward declarations
namespace lariov {
  class UBElectronLifetimeProvider;
}

namespace lariov {

  class UBElectronLifetimeService {
    
    public:
      using provider_type = UBElectronLifetimeProvider;
      
      virtual ~UBElectronLifetimeService() = default;
      
      //retrieve provider
      const UBElectronLifetimeProvider& GetProvider() const {
        return this->DoGetProvider();
      }
      
      UBElectronLifetimeProvider const* provider() const {
        return &DoGetProvider();
      }
      
    private:
    
      virtual const UBElectronLifetimeProvider& DoGetProvider() const = 0;
  };
}//end namespace lariov

DECLARE_ART_SERVICE_INTERFACE(lariov::UBElectronLifetimeService, LEGACY)

#endif
