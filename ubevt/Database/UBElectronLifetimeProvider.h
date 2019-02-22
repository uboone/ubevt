/**
 * \file ElectronLifetimeProvider
 * 
 * \brief Class def header for a class ElectronLifetimeProvider
 *
 * @author eberly@slac.stanford.edu
 */

#ifndef UBELECTRONLIFETIMEPROVIDER_H
#define UBELECTRONLIFETIMEPROVIDER_H

#include "larcorealg/CoreUtils/UncopiableAndUnmovableClass.h"

namespace lariov {

  /**
     \class UBElectronLifetimeProvider
     Pure abstract interface class for retrieving electron lifetimes.
     
  */
  class UBElectronLifetimeProvider: private lar::UncopiableAndUnmovableClass {
  
    public:
    
      virtual ~UBElectronLifetimeProvider() = default;
       
      /// Retrieve pedestal information     
      virtual float Lifetime() const = 0;
      virtual float LifetimeErr() const = 0;
      
 
  };
}//end namespace lariov

#endif
