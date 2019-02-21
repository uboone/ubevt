/**
 * \file ElectronLifetime.h
 *
 * \ingroup IOVData
 * 
 * \brief Class def header for a class ElectronLifetime
 *
 * @author eberly@slac.stanford.edu
 */

#ifndef UBOONEELECTRONLIFETIMECONTAINER_H
#define UBOONEELECTRONLIFETIMECONTAINER_H 1

#include "larevt/CalibrationDBI/IOVData/ChData.h"
#include "larevt/CalibrationDBI/IOVData/CalibrationExtraInfo.h"

namespace lariov {
  /**
     \class UbooneElectronLifetimeContainer
  */
  class UbooneElectronLifetimeContainer : public ChData {
    
    public:
    
      /// Constructor
      UbooneElectronLifetimeContainer(unsigned int ch) : 
        ChData(ch),
	fExtraInfo("ElectronLifetime") {}
      
      /// Default destructor
      ~UbooneElectronLifetimeContainer() {}
            
      
      float Lifetime()        const { return fLifetime; }
      float LifetimeErr()     const { return fLifetimeErr; }
      CalibrationExtraInfo const& ExtraInfo() const { return fExtraInfo; }
      
      void SetLifetime(float val)        { fLifetime        = val; }
      void SetLifetimeErr(float val)     { fLifetimeErr     = val; }
      void SetExtraInfo(CalibrationExtraInfo const& info) { fExtraInfo = info; }
      
    private:
      
      float fLifetime;
      float fLifetimeErr;
      CalibrationExtraInfo fExtraInfo;

      
  }; // end class
} // end namespace lariov

#endif
 
