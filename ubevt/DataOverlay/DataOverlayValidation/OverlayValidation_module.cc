////////////////////////////////////////////////////////////////////////
// Class:       OverlayValidation
// Plugin Type: analyzer (art v2_11_03)
// File:        OverlayValidation_module.cc
//
// Generated at Sun Oct 28 19:46:22 2018 by Christopher Barnes using cetskelgen
// from cetlib version v3_03_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// Includes for RawDigits.
#include "lardataobj/RawData/RawDigit.h"

namespace lar {
  class OverlayValidation;
}


class lar::OverlayValidation : public art::EDAnalyzer {
public:
  explicit OverlayValidation(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  OverlayValidation(OverlayValidation const &) = delete;
  OverlayValidation(OverlayValidation &&) = delete;
  OverlayValidation & operator = (OverlayValidation const &) = delete;
  OverlayValidation & operator = (OverlayValidation &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

private:

  std::string fRawDigitProducer;

  reconfigure;
  // Declare member data here.

};


lar::OverlayValidation::OverlayValidation(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)  // ,
 // More initializers here.
{
  fRawDigitProducer = p.get<std::string>("RawDigitProducer");
}

void lar::OverlayValidation::analyze(art::Event const & e)
{
  // Implementation of required member function here.
  
  // Read in the RawDigits of the overlay.
  art::Handle<std::vector<raw::RawDigit> > rawdigit_h;
  e.getByLabel( fRawDigitProducer , rawdigit_h );

  // make sure tracks look good                                                                                                                                                                             
  if(!rawdigit_h.isValid()) {
    std::cerr<<"\033[93m[ERROR]\033[00m ... could not locate Raw Digits!"<<std::endl;
    throw std::exception();
  }
  
  // Loop over the raw digits.
  for ( size_t i = 0; i < rawdigit_h->size(); i++ ) {

    // Obtain the entry at each tick for this channel.
    std::vector< short > adc_counts = rawdigit_h->at( i ).ADCvector_t;

    // Loop through the entry at each time tick.
    for ( size_t j = 0; j < adc_counts.size(); j++ ) {

      short current_tick = adc_counts.at( j );

      if ( flag( i, j ) && current_tick > 0 ) 
	std::cout << "There is a bug in the overlay!" << std::endl;

    }

  }

}

DEFINE_ART_MODULE(lar::OverlayValidation)
