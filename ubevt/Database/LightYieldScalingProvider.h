/**
 * \file LightYieldScalingProvider.h
 *
 * \ingroup WebDBI
 * 
 * \brief Class def header for a class LightYieldScalingProvider
 *
 * @author ralitsa.sharankova@tufts.edu
 */

#ifndef LIGHTYIELDSCALINGPROVIDER_H
#define LIGHTYIELDSCALINGPROVIDER_H

#include "larevt/CalibrationDBI/IOVData/PmtGain.h"
#include "larevt/CalibrationDBI/IOVData/Snapshot.h"
#include "larevt/CalibrationDBI/IOVData/IOVDataConstants.h"
#include "larevt/CalibrationDBI/Interface/PmtGainProvider.h"
#include "larevt/CalibrationDBI/Providers/DatabaseRetrievalAlg.h"

namespace lariov {

  /**
   * @brief Retrieves information: light yield scaling
   * 
   * Configuration parameters
   * =========================
   * 
   * - *DatabaseRetrievalAlg* (parameter set, mandatory): configuration for the
   *   database; see lariov::DatabaseRetrievalAlg
   * - *UseDB* (boolean, default: false): retrieve information from the database
   * - *UseFile* (boolean, default: false): retrieve information from a file;
   *   not implemented yet
   * - *DefaultLightYield* (real, default: ): Light Yield Scaling returned 
   *   when /UseDB/ and /UseFile/ parameters are false
   * - *DefaultGainErr* (real, default: ): Gain uncertainty returned
   *   when /UseDB/ and /UseFile/ parameters are false
   */
  class LightYieldscalingProvider : public DatabaseRetrievalAlg, public PmtGainProvider {
  
    public:
    
      /// Constructors
      LightYieldscalingProvider(fhicl::ParameterSet const& p);
      
      /// Reconfigure function called by fhicl constructor
      void Reconfigure(fhicl::ParameterSet const& p) override;
      
      /// Update event time stamp.
      void UpdateTimeStamp(DBTimeStamp_t ts);

      /// Update Snapshot and inherited DBFolder if using database.  Return true if updated
      bool Update(DBTimeStamp_t ts);
      
      /// Retrieve gain information
      const PmtGain& PmtGainObject(DBChannelID_t ch) const;      
      float LYScaling(DBChannelID_t ch) const override;
      float LYSalingErr(DBChannelID_t ch) const override;
      CalibrationExtraInfo const& ExtraInfo(DBChannelID_t ch) const override;
      
    private:
    
      /// Do actual database updates.

      bool DBUpdate() const;                    // Uses current event time.
      bool DBUpdate(DBTimeStamp_t ts) const;

      // Time stamps.

      DBTimeStamp_t fEventTimeStamp;            // Most recently seen time stamp.
      mutable DBTimeStamp_t fCurrentTimeStamp;  // Time stamp of cached data.

      DataSource::ds fDataSource;
          
      mutable Snapshot<PmtGain> fData;
  };
}//end namespace lariov

#endif

