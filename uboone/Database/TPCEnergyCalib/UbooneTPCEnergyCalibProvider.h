/**
 * \file UbooneTPCEnergyCalibProvider.h
 * 
 * \brief Class def header for a class UbooneTPCEnergyCalibProvider
 *
 * @author eberly@slac.stanford.edu
 */

#ifndef UBOONETPCENERGYCALIBPROVIDER_H
#define UBOONETPCENERGYCALIBPROVIDER_H

#include "larevt/CalibrationDBI/IOVData/IOVDataConstants.h"
#include "Uboonedqdx1DCorrectionProvider.h"
#include "TPCEnergyCalibProvider.h"

namespace lariov {

  /**
   * @brief Retrieves information: electronics calibrations, specifically gain and shaping time
   * 
   * Configuration parameters
   * =========================
   * 
   * - *DatabaseRetrievalAlg* (parameter set, mandatory): configuration for the
   *   database; see lariov::DatabaseRetrievalAlg
   * - *UseDB* (boolean, default: false): retrieve information from the database
   * - *UseFile* (boolean, default: false): retrieve information from a file;
   *   not implemented yet
   * - *DefaultGain* (real, default: ): Gain returned 
   *   when /UseDB/ and /UseFile/ parameters are false
   * - *DefaultGainErr* (real, default: ): Gain uncertainty returned
   *   when /UseDB/ and /UseFile/ parameters are false
   * - *DefaultShapingTime* (real, default: ): Shaping Time returned 
   *   when /UseDB/ and /UseFile/ parameters are false
   * - *DefaultShapingTimeErr* (real, default: ): Shaping Time uncertainty returned
   *   when /UseDB/ and /UseFile/ parameters are false
   */
  class UbooneTPCEnergyCalibProvider : public TPCEnergyCalibProvider {
  
    public:
    
      /// Constructors
      UbooneTPCEnergyCalibProvider(fhicl::ParameterSet const& p);
      
      /// Reconfigure function called by fhicl constructor
      void Reconfigure(fhicl::ParameterSet const& p);
      
      /// Update each source provider
      bool Update(DBTimeStamp_t ts);
      
      /// Retrieve TPC dq/dx and dE/dx calibration information     
      float YZdqdxCorrection(float y, float z) const override;
      float YZdqdxCorrectionErr(float y, float z) const override;
      float DriftdqdxCorrection(float drift_coord) const override;
      float DriftdqdxCorrectionErr(float drift_coord) const override;
      
      float XdqdxCorrection(float x) const override;
      float XdqdxCorrectionErr(float x) const override;
      float YdqdxCorrection(float y) const override;
      float YdqdxCorrectionErr(float y) const override;
      float ZdqdxCorrection(float z) const override;
      float ZdqdxCorrectionErr(float z) const override;
      
      float ThetadqdxCorrection(float theta) const override;
      float ThetadqdxCorrectionErr(float theta) const override;
      float PhidqdxCorrection(float phi) const override;
      float PhidqdxCorrectionErr(float phi) const override;
      
      /// total dqdx correction
      float TotaldqdxCorrection(float x, float y, float z, float theta, float phi=0.0) const override;
      float TotaldqdxCorrectionErr(float x, float y, float z, float theta, float phi=0.0) const override;
      
      /// dEdx correction as a function of plane number
      float dEdxCorrection(int plane) const override;
      float dEdxCorrectionErr(int plane) const override;
      
      
    private:
      
      //Uboonedqdx2DCorrectionProvider    fYZProvider;
      Uboonedqdx1DCorrectionProvider     fXProvider;
      //Uboonedqdx1DCorrectionProvider fThetaProvider;
      //UboonedEdxCorrectionProvider      fdEdxProvider;
  };
}//end namespace lariov

#endif
