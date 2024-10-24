/**
 * \file UbooneElectronLifetimeProvider.h
 *
 * \ingroup WebDBI
 * 
 * \brief Class def header for a class UbooneElectronLifetimeProvider
 *
 * @author eberly@slac.stanford.edu
 */

#ifndef UBOONEELECTRONLIFETIMEPROVIDER_H
#define UBOONEELECTRONLIFETIMEPROVIDER_H

// C/C++ standard libraries
#include <string>
#include <array>

// LArSoft libraries
#include "larevt/CalibrationDBI/IOVData/Snapshot.h"
#include "larevt/CalibrationDBI/IOVData/IOVDataConstants.h"
#include "larevt/CalibrationDBI/Providers/DatabaseRetrievalAlg.h"

#include "UBElectronLifetimeProvider.h"
#include "UbooneElectronLifetimeContainer.h"

namespace lariov {

  /**
   * @brief Retrieves channel information: pedestal and RMS
   * 
   * Configuration parameters
   * =========================
   * 
   * - *DatabaseRetrievalAlg* (parameter set, mandatory): configuration for the
   *   database; see lariov::DatabaseRetrievalAlg
   * - *UseDB* (boolean, default: false): retrieve information from the database
   * - *UseFile* (boolean, default: false): retrieve information from a file;
   *   not implemented yet
   * - *DefaultExp* (real, default: 400.0): exponential parameter returned
   *   when /UseDB/ and /UseFile/ parameters are false
   * - *DefaultIndMean* (real, default: 2048.0): constant parameter returned
   *   when /UseDB/ and /UseFile/ parameters are false
   */
  class UbooneElectronLifetimeProvider : public DatabaseRetrievalAlg, public UBElectronLifetimeProvider {
  
    public:
    
      /// Constructors
      UbooneElectronLifetimeProvider(const std::string& foldername, 
      			      const std::string& url, 
			      const std::string& tag="");
	
      UbooneElectronLifetimeProvider(fhicl::ParameterSet const& p);
      
      /// Reconfigure function called by fhicl constructor
      void Reconfigure(fhicl::ParameterSet const& p) override;
      
      /// Update event time stamp.
      void UpdateTimeStamp(DBTimeStamp_t ts);

      /// Update Snapshot and inherited DBFolder if using database.  Return true if updated
      bool Update(DBTimeStamp_t ts);
      
      /// Retrieve lifetime information
      const UbooneElectronLifetimeContainer& LifetimeContainer() const;
      float Lifetime() const override;
      float LifetimeErr() const override;
      
      
           
      //hardcoded information about database folder - useful for debugging cross checks
      static constexpr unsigned int NCOLUMNS = 3;
      static constexpr const char* FIELD_NAMES[NCOLUMNS]
        = {"channel", "lifetime", "lifetime_err"};
      static constexpr const char* FIELD_TYPES[NCOLUMNS]
        = {"bigint", "real", "real"};
      
    private:
    
      /// Do actual database updates.

      bool DBUpdate() const;                    // Uses current event time.
      bool DBUpdate(DBTimeStamp_t ts) const;

      // Time stamps.

      DBTimeStamp_t fEventTimeStamp;            // Most recently seen time stamp.
      mutable DBTimeStamp_t fCurrentTimeStamp;  // Time stamp of cached data.

      DataSource::ds fDataSource;
          
      mutable Snapshot<UbooneElectronLifetimeContainer> fData;
      
      const unsigned int fLifetimeChannel = 0;
  };
}//end namespace lariov

#endif
