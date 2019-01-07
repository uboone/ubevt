////////////////////////////////////////////////////////////////////////
// Class:       MaskingOutUnavailableCRT
// Plugin Type: producer (art v2_11_03)
// File:        MaskingOutUnavailableCRT_module.cc
//
// Generated at Fri Nov  2 13:48:18 2018 by Christopher Barnes using cetskelgen
// from cetlib version v3_03_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

//includes for CRTHits.
#include "ubobj/CRT/CRTSimData.hh"
#include "ubobj/CRT/CRTHit.hh"

#include <string>
#include <vector>
#include <memory>

namespace lar {
  class MaskingOutUnavailableCRT;
}


class lar::MaskingOutUnavailableCRT : public art::EDProducer {
public:
  explicit MaskingOutUnavailableCRT(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  MaskingOutUnavailableCRT(MaskingOutUnavailableCRT const &) = delete;
  MaskingOutUnavailableCRT(MaskingOutUnavailableCRT &&) = delete;
  MaskingOutUnavailableCRT & operator = (MaskingOutUnavailableCRT const &) = delete;
  MaskingOutUnavailableCRT & operator = (MaskingOutUnavailableCRT &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

private:
  
  // This is the name of the producer of the CRTHits.
  std::string fCRTHitLabel;
  std::string fCRTHitInstanceName;
  std::string fMaskedLabel;
  std::string fMaskedInstanceName;
  bool        verbose;

  int         event_counter;
  int         bad_adc_counter;

};


lar::MaskingOutUnavailableCRT::MaskingOutUnavailableCRT(fhicl::ParameterSet const & p)
// :
// Initialize member data here.
{
  // Call appropriate produces<>() functions here.
  produces< std::vector<crt::CRTHit> >();

  fCRTHitLabel        = p.get<std::string>("CRTHitLabel");
  fCRTHitInstanceName = p.get<std::string>("CRTHitInstanceName");
  fMaskedLabel         = p.get<std::string>("MaskedLabel");
  fMaskedInstanceName  = p.get<std::string>("MaskedInstanceName");
  verbose               = p.get<bool>("verbose");

  event_counter         = 0;
  bad_adc_counter       = 0;

}

void lar::MaskingOutUnavailableCRT::produce(art::Event & e)
{
  if ( verbose ) 
    std::cout << "Event #" << event_counter << std::endl;

  event_counter++;

  if ( verbose ) {
    // Print out the run, subrun, and event.
    std::cout << "run = " << e.run() << "." << std::endl;
    std::cout << "subrun = " << e.subRun() << "." << std::endl;
    std::cout << "event = " << e.event() << "." << std::endl;
  }

  int runnumber = e.run();
  // Implementation of required member function here.
  // Read in the CRTHits of the overlay.                                                                                                                                                              
  art::Handle<std::vector<crt::CRTHit> > crthits_h;
  e.getByLabel( fCRTHitLabel, crthits_h );

  // make sure CRTHits look good.                                                                                                                                                                  
  if(!crthits_h.isValid()) {
    std::cout<<"THERE ARE NO SIMULATED CRT HITS !"<<std::endl;
  }

  std::unique_ptr<std::vector<crt::CRTHit>> dummyInput(new std::vector<crt::CRTHit>);
  std::vector<crt::CRTHit> const& crthits = (crthits_h.isValid())? *crthits_h : *dummyInput;

  // Declare the products that we will be outputing from this module.
  std::unique_ptr< std::vector<crt::CRTHit> > masked_crthits_v(new std::vector<crt::CRTHit>);

  // Loop through the old CRTHits and set the new CRTHits equal to all of those values.

  // Loop over the crt hits.                                                                                                                                                                         
  for(auto const& od : crthits) {
    // The differnt CRT comissioning periods are hardcoded here and should. 
    // Infor should be placed in a future database and this piece of code be modified.  
    if (runnumber < 11049 ) continue; //first period - no CRT
    //else if (runnumber < 20000) {   //second period - only 0,1,2 planes
    //    if (od.plane > 3) masked_crthits_v->emplace_back( od );
    //}
                                      //third period - full comissioning 
    masked_crthits_v->emplace_back( od );
    
    
  } // End of the loop over the CRTHits.

  // Add the new CRTHits to the event.
  e.put(std::move(masked_crthits_v));

  return;

}
DEFINE_ART_MODULE(lar::MaskingOutUnavailableCRT)
