/**
 * @file   PMTRemapProvider.h
 * @brief  MicroBooNE service provider that provides a map to correct PMT OpChannel numbers
 * @author Brandon Eberly (eberly@slac.stanford.edu)
 */

#ifndef PMTREMAPPROVIDER_H
#define PMTREMAPPROVIDER_H

#include "fhiclcpp/ParameterSet.h"

namespace util {

  class PMTRemapProvider {
  
    public:
    
      PMTRemapProvider(fhicl::ParameterSet const& pset);
      ~PMTRemapProvider() = default;
      
      unsigned int CorrectedOpChannel( unsigned int original_opchannel ) const;
      unsigned int OriginalOpChannel( unsigned int corrected_opchannel ) const;
      unsigned int CorrectedOpDet( unsigned int original_opdet ) const;
      unsigned int OriginalOpDet( unsigned int corrected_opdet ) const;
      
    private:
    
      std::map<unsigned int, unsigned int> fOriginal_to_corrected_map;

      // Forward and reverse OpDet <--> OpChannel maps.
      // Information for these is obtained from the geomerty service.

      std::map<unsigned int, unsigned int> fOpDetToOpChannel;
      std::map<unsigned int, unsigned int> fOpChannelToOpDet;
  };
 
}

#endif
