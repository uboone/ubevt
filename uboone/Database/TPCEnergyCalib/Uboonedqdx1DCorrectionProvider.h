/**
 * \file Uboonedqdx1DCorrectionProvider.h
 * 
 * \brief Class def header for a class Uboonedqdx1DCorrectionProvider
 *
 * @author eberly@slac.stanford.edu
 */

#ifndef UBOONEDQDX1DCORRECTIONPROVIDER_H
#define UBOONEDQDX1DCORRECTIONPROVIDER_H

#include "dqdx1DCorrection.h"
#include "larevt/CalibrationDBI/IOVData/Snapshot.h"
#include "larevt/CalibrationDBI/IOVData/IOVDataConstants.h"
#include "larevt/CalibrationDBI/Providers/DatabaseRetrievalAlg.h"

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
  class Uboonedqdx1DCorrectionProvider : public DatabaseRetrievalAlg {
  
    public:
    
      /// Constructors
      Uboonedqdx1DCorrectionProvider(fhicl::ParameterSet const& p);
      
      /// Reconfigure function called by fhicl constructor
      void Reconfigure(fhicl::ParameterSet const& p);
      
      /// Update Snapshot and inherited DBFolder if using database.  Return true if updated
      bool Update(DBTimeStamp_t ts);
      
      /// Retrieve calibration object
      const dqdx1DCorrection& Dqdx1DCorrectionObject(float coord) const;      
      
      /// Retrieve calibration info
      float DqdxCorrection(float coord) const;
      float DqdxCorrectionErr(float coord) const;
      
    private:
    
      int GetBinNumber(float coord) const;
    
      DataSource::ds fDataSource;
          
      Snapshot<dqdx1DCorrection> fData;
      
      //convert from input coordinate to bin number used by Snapshot
      float fCoord_min;
      float fCoord_max;
      bool fIsFixedBinSize;
      std::vector<float> fCoordToBinMap;
  };
}//end namespace lariov

#endif
