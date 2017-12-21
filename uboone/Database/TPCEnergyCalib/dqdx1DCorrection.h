/**
 * \file dqdx1DCorrection.h
 * 
 * \brief Class def header for a class dqdx1DCorrection
 *
 * @author eberly@slac.stanford.edu
 */

#ifndef IOVDATA_DQDX1DCORRECTION_H
#define IOVDATA_DQDX1DCORRECTION_H

#include "larevt/CalibrationDBI/IOVData/ChData.h"
#include "larevt/CalibrationDBI/IOVData/CalibrationExtraInfo.h"

namespace lariov {
  /**
     \class dqdx1DCorrection   
  *
  */
  class dqdx1DCorrection : public ChData {
    
    public:
    
      /// Constructor
      dqdx1DCorrection(unsigned int bin) : 
        ChData(bin),
	fExtraInfo("dqdx1DCorrection") {}
      
      /// Default destructor
      ~dqdx1DCorrection() {}
            
      float Correction()    const { return fCorrection; }
      float CorrectionErr() const { return fCorrectionErr; }
      CalibrationExtraInfo const& ExtraInfo() const { return fExtraInfo; }
      
      void SetCorrection(float v)    { fCorrection   = v; }
      void SetCorrectionErr(float v) { fCorrectionErr = v; }
      void SetExtraInfo(CalibrationExtraInfo const& info) { fExtraInfo = info; }
      
    private:
    
      float fCorrection;
      float fCorrectionErr;
      CalibrationExtraInfo fExtraInfo;
      
  }; // end class
} // end namespace lariov

#endif
