/**
 * \file TPCdEdxProvider
 * 
 * \brief Class def header for a class TPCdEdxProvider
 *
 * @author eberly@slac.stanford.edu
 */

#ifndef TPCENERGYCALIBPROVIDER_H
#define TPCENERGYCALIBPROVIDER_H

#include "larcoreobj/SimpleTypesAndConstants/RawTypes.h"

namespace lariov {

  /**
   \class TPCEnergyCalibProvider
   * Currently, the class provides interface for the following information:
   * - dq/dx correction factor as a function of YZ (plane orthogonal to drift direction) and its uncertainty
   * - dq/dx correction factor as a function of drift coordinate and its uncertainty
   * - dq/dx correction factor as a function of X, Y, and Z coordinates and its uncertainty
   *
   * - dq/dx correction factor as a function of theta (implementation defined) and its uncertainty
   * - dq/dx correction factor as a function of phi (implementation defined) and its uncertainty
   *
   * - dE/dx correction factor per plane number and its uncertainty
   *
   * Notes: This class design allows for the possibility that the total dq/dx correction can be factorized into separate dependencies.  
   * If this is not true for the implementation, then the relevant factorized retriever functions should 
   * be implemented to throw an error that directs the user to the correct dependencies or the TotaldqdxCorrection function
   */
  class TPCEnergyCalibProvider {
  
    public:
    
      virtual ~TPCEnergyCalibProvider() = default;
       
      /// dqdx corrections factorized into drift and drift-orthogonal components   
      virtual float YZdqdxCorrection(float y, float z) const = 0;
      virtual float YZdqdxCorrectionErr(float y, float z) const = 0;
      virtual float DriftdqdxCorrection(float drift_coord) const = 0;
      virtual float DriftdqdxCorrectionErr(float drift_coord) const = 0;
      
      /// dqdx corrections factorized into x, y, and z components
      virtual float XdqdxCorrection(float x) const = 0;
      virtual float XdqdxCorrectionErr(float x) const = 0;
      virtual float YdqdxCorrection(float y) const = 0;
      virtual float YdqdxCorrectionErr(float y) const = 0;
      virtual float ZdqdxCorrection(float z) const = 0;
      virtual float ZdqdxCorrectionErr(float z) const = 0;
      
      /// phi and theta dependence of the dqdx correction
      virtual float ThetadqdxCorrection(float theta) const = 0;
      virtual float ThetadqdxCorrectionErr(float theta) const = 0;
      virtual float PhidqdxCorrection(float phi) const = 0;
      virtual float PhidqdxCorrectionErr(float phi) const = 0;
      
      /// total dqdx correction
      virtual float TotaldqdxCorrection(float x, float y, float z, float theta, float phi) const = 0;
      virtual float TotaldqdxCorrectionErr(float x, float y, float z, float theta, float phi) const = 0;
      
      /// dEdx correction as a function of plane number
      virtual float dEdxCorrection(int plane) const = 0;
      virtual float dEdxCorrectionErr(int plane) const = 0;
      
  };
}//end namespace lariov

#endif
