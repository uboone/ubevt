/**
 * \file LightYieldProvider
 * 
 * \brief Class def header for a class LightYieldProvider
 *
 * @author ralitsa.sharankova@tufts.edu
 */

#ifndef LIGHTYIELDPROVIDER_H
#define LIGHTYIELDPROVIDER_H

#include "larcoreobj/SimpleTypesAndConstants/RawTypes.h"
#include "larcorealg/CoreUtils/UncopiableAndUnmovableClass.h"
#include "larevt/CalibrationDBI/IOVData/CalibrationExtraInfo.h"

namespace lariov {

  /**
   \class LightYieldProvider
   * Currently, the class provides interface for the following information:
   * - light yield relative scaling and its error
   * - extra info describing x-dependence modeling of scaling etc.
   */
  class LightYieldProvider: private lar::UncopiableAndUnmovableClass {
  
    public:
    
      virtual ~LightYieldProvider() = default;
       
      /// Retrieve pmt gain information     
      virtual float LYScaling(raw::ChannelID_t ch) const = 0;
      virtual float LYScalingErr(raw::ChannelID_t ch) const = 0;
      
      virtual CalibrationExtraInfo const& ExtraInfo(raw::ChannelID_t ch) const = 0;
  };
}//end namespace lariov

#endif
