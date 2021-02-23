#ifndef OVERLAY_DATAOVERLAY_CRTMIXER_CXX
#define OVERLAY_DATAOVERLAY_CRTMIXER_CXX

#include "CRTMixer.h"
#include <limits>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <TFile.h>
#include <TList.h>
#include <TH1D.h>

void mix::CRTMixer::DeclareData(std::vector<crt::CRTHit> const& dataVector,
				std::vector<crt::CRTHit> & outputVector){

  //fChannelIndexMap.clear();
  outputVector.reserve(dataVector.size());

  for(auto const& od : dataVector){

    outputVector.emplace_back(od);

  }
  
}

void mix::CRTMixer::Mix(std::vector<crt::CRTHit> const& mcVector,
			std::vector<crt::CRTHit> const& dataVector,
			std::vector<crt::CRTHit> & outputVector){

 outputVector.reserve(dataVector.size()+mcVector.size());

  for(auto const& od : dataVector){ outputVector.emplace_back(od);}
  for(auto const& od : mcVector)  { outputVector.emplace_back(od);}

}

bool mix::CRTMixer::Mask(crt::CRTHit simCRThit){
     return false;    
}
#endif
