/**
 * @file   PMTRemapProvider.cxx
 * @brief  MicroBooNE service provider that provides a map to correct PMT OpChannel numbers
 * @author Brandon Eberly (eberly@slac.stanford.edu)
 */

#include "ubevt/Utilities/PMTRemapProvider.h"
#include "larcore/Geometry/Geometry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

namespace util {

  PMTRemapProvider::PMTRemapProvider(fhicl::ParameterSet const& pset) {

    std::cout << "Initializing PMT Remap Provider" << std::endl;
  
    //hard-code the map
    fOriginal_to_corrected_map[26] = 27;
    fOriginal_to_corrected_map[27] = 28;
    fOriginal_to_corrected_map[28] = 29;
    fOriginal_to_corrected_map[29] = 30;
    fOriginal_to_corrected_map[30] = 31;
    fOriginal_to_corrected_map[31] = 26;

    // Initialize OpDet vs. OpChannel maps.

    art::ServiceHandle<geo::Geometry> geom;

    // Loop over OpChannels.

    for(unsigned int opch=0; opch<100; ++opch) {
      if(geom->IsValidOpChannel(opch)) {
	unsigned int opdet = geom->OpDetFromOpChannel(opch);
	std::cout << "OpChannel " << opch << ", OpDet " << opdet << std::endl;
	fOpDetToOpChannel[opdet] = opch;
	fOpChannelToOpDet[opch] = opdet;
      }
    }
    
  }
  
  unsigned int PMTRemapProvider::CorrectedOpChannel(unsigned int orig) const {
    
    auto it = fOriginal_to_corrected_map.find(orig%100);
    unsigned int hundreds = orig/100;
    
    if ( it == fOriginal_to_corrected_map.end() ) {
      return orig;
    }
    else return hundreds*100 + it->second;
  }
  
  unsigned int PMTRemapProvider::OriginalOpChannel(unsigned int corr) const {
    
    unsigned int mod = corr%100;
    unsigned int hundreds = corr/100;
    
    for (auto it = fOriginal_to_corrected_map.begin(); it != fOriginal_to_corrected_map.end(); ++it) {
      if (it->second == mod) {
        return hundreds*100 + it->first;
      }
    }
    
    return corr;
  }

  unsigned int PMTRemapProvider::CorrectedOpDet( unsigned int original_opdet ) const
  {
    unsigned int original_opch = fOpDetToOpChannel.at(original_opdet);
    unsigned int corrected_opch = CorrectedOpChannel(original_opch);
    unsigned int corrected_opdet = fOpChannelToOpDet.at(corrected_opch);
    return corrected_opdet;
  }

  unsigned int PMTRemapProvider::OriginalOpDet( unsigned int corrected_opdet ) const
  {
    unsigned int corrected_opch = fOpDetToOpChannel.at(corrected_opdet);
    unsigned int original_opch = OriginalOpChannel(corrected_opch);
    unsigned int original_opdet = fOpChannelToOpDet.at(original_opch);
    return original_opdet;
  }

} //end namespace util
      
      
