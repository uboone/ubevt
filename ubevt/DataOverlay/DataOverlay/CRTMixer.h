/**
 * \file CRTMixer.h
 *
 * \ingroup DataOverlay
 * 
 * \brief Mixer function for adding complete CRT hits from Data and MC  
 *
 */

/** \addtogroup DataOverlay

    @{*/
#ifndef OVERLAY_DATAOVERLAY_CRTMIXER_H
#define OVERLAY_DATAOVERLAY_CRTMIXER_H

#include <vector>
#include <string>
//#include <unordered_map>

//#include "lardataobj/RawData/OpDetWaveform.h"
//#include "RawDigitAdder_HardSaturate.h"

#include "ubobj/CRT/CRTSimData.hh"
#include "ubobj/CRT/CRTHit.hh"

/**
   \class CRTMixer
   Add two raw digit collections together.
   
*/

namespace mix {
  class CRTMixer;
}

class mix::CRTMixer{

public:

  /// Default constructor
  CRTMixer(bool p=false):
  _printWarnings(p){};

  void DeclareData(std::vector<crt::CRTHit> const& dataVector,
		   std::vector<crt::CRTHit> & outputVector);
  //void Mix(std::vector<crt::CRTHit> const& mcVector,
	   //std::unordered_map<raw::Channel_t,float> const& map,
	//   std::vector<crt::CRTHit> & outputVector);
  void Mix(std::vector<crt::CRTHit> const& mcVector,
           std::vector<crt::CRTHit> const& dataVector,
           std::vector<crt::CRTHit> & outputVector);  
  //void SetSaturationPoint(short x)
  //{ fRDAdderAlg.SetSaturationPoint(x); }
  
  //void SetMinSampleSize(size_t x)
  //{ fMinSampleSize = x; }

  /// Default destructor
  virtual ~CRTMixer(){};
  
  
 private:
  
  bool _printWarnings;

// std::unordered_map<raw::Channel_t,size_t> fChannelIndexMap;
  
  //RawDigitAdder_HardSaturate fRDAdderAlg;

  //size_t fMinSampleSize;
  
};

#endif
/** @} */ // end of doxygen group 

