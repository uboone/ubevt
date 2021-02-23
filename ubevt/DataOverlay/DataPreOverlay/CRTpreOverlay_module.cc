////////////////////////////////////////////////////////////////////////
// Class:       CRTpreOverlay
// Plugin Type: producer (art v2_11_03)
// File:        CRTpreOverlay_module.cc
//
// Generated at Fri Nov  2 13:48:18 2018 by Adi Ashkenazi 
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
#include "ubobj/RawData/DAQHeaderTimeUBooNE.h"

//#include "ubobj/CRT/CRTTrack.hh"
//#include "ubcrt/CRT/CRTAuxFunctions.hh"

#include <string>
#include <vector>
#include <memory>

namespace lar {
  class CRTpreOverlay;
}


class lar::CRTpreOverlay : public art::EDProducer {
public:
  explicit CRTpreOverlay(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CRTpreOverlay(CRTpreOverlay const &) = delete;
  CRTpreOverlay(CRTpreOverlay &&) = delete;
  CRTpreOverlay & operator = (CRTpreOverlay const &) = delete;
  CRTpreOverlay & operator = (CRTpreOverlay &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

private:
  
  // This is the name of the producer of the CRTHits.
  std::string fCRTHitLabel;
  std::string fCRTHitInstanceName;
  std::string fMaskedLabel;
  std::string fMaskedInstanceName;
  bool        verbose;
  std::string  data_label_DAQHeader_;

  int         event_counter;
  int         bad_adc_counter;
  int         fTimeZeroOffset;

};


lar::CRTpreOverlay::CRTpreOverlay(fhicl::ParameterSet const & p)
  : art::EDProducer(p)
// Initialize member data here.
{
  // Call appropriate produces<>() functions here.
  produces< std::vector<crt::CRTHit> >();

  fCRTHitLabel        = p.get<std::string>("CRTHitLabel");
  fCRTHitInstanceName = p.get<std::string>("CRTHitInstanceName");
  fMaskedLabel         = p.get<std::string>("MaskedLabel");
  fMaskedInstanceName  = p.get<std::string>("MaskedInstanceName");
  verbose               = p.get<bool>("verbose");
  data_label_DAQHeader_ = p.get<std::string>("data_label_DAQHeader_");
  fTimeZeroOffset       = p.get<int>("TimeZeroOffset");

  event_counter         = 0;
  bad_adc_counter       = 0;

}

void lar::CRTpreOverlay::produce(art::Event & e)
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

  // Declare the products that we will be outputing from this module.
  std::unique_ptr< std::vector<crt::CRTHit> > masked_crthits_v(new std::vector<crt::CRTHit>);

  // get DAQHeader for GPS time                                                                                             
  art::Handle< raw::DAQHeaderTimeUBooNE > rawHandle_DAQHeader;
  e.getByLabel(data_label_DAQHeader_, rawHandle_DAQHeader);
  // make sure DAQHeader for GPS time looks good
  if(!rawHandle_DAQHeader.isValid()){
    std::cout << "Run " << e.run() << ", subrun " << e.subRun()
              << ", event " << e.event() << " has zero"
              << " DAQHeaderTimeUBooNE  " << " in with label " << data_label_DAQHeader_ << std::endl;
    e.put(std::move(masked_crthits_v));
    return;
  }
  // get event GPS time 
  raw::DAQHeaderTimeUBooNE const& my_DAQHeader(*rawHandle_DAQHeader);
  art::Timestamp evtTimeGPS = my_DAQHeader.gps_time();
  auto evt_timeGPS_sec = evtTimeGPS.timeHigh();
  auto evt_timeGPS_nsec = evtTimeGPS.timeLow();
  if(verbose!=0){
    std::cout<<"Evt:GPS_Time: sec, nsec :"<<evt_timeGPS_sec <<" "<< evt_timeGPS_nsec <<std::endl;
  }
 
  std::unique_ptr<std::vector<crt::CRTHit>> dummyInput(new std::vector<crt::CRTHit>);
  std::vector<crt::CRTHit> const& crthits = (crthits_h.isValid())? *crthits_h : *dummyInput;

  // Loop through the old CRTHits and set the new CRTHits equal to all of those values.

  // Loop over the crt hits.                                                                                                                                                                         
  for(auto const& od : crthits) {
    // !!! The differnt CRT comissioning periods are hardcoded here and should ! 
    // !!! be placed in a future database and this piece of code be modified !!!  
    if (runnumber < 11049 ) continue; // before full comissioning of the CRT

    crt::CRTHit CRTHitevent;

    CRTHitevent.plane = od.plane; 
    CRTHitevent.x_pos = od.x_pos;
    CRTHitevent.x_err = od.x_err;
    CRTHitevent.y_pos = od.y_pos;
    CRTHitevent.y_err = od.y_err;
    CRTHitevent.z_pos = od.z_pos;
    CRTHitevent.z_err = od.z_err;
    CRTHitevent.ts0_s = od.ts0_s + evt_timeGPS_sec; 
    CRTHitevent.ts0_ns = od.ts1_ns + evt_timeGPS_nsec - float(fTimeZeroOffset); //<-- The only time that makes sense in simulation is t1
    //CRTHitevent.ts0_s_err = od.ts0_s_err;
    //CRTHitevent.ts0_ns_err = od.ts0_ns_err;
    CRTHitevent.ts1_ns = od.ts1_ns;
    //CRTHitevent.ts1_ns_err = od.ts1_ns_err;
    CRTHitevent.feb_id = od.feb_id; 
    CRTHitevent.pesmap = od.pesmap;
    CRTHitevent.peshit = od.peshit;

    masked_crthits_v->emplace_back( CRTHitevent );
    
    
  } // End of the loop over the CRTHits.

  // Add the new CRTHits to the event.
  e.put(std::move(masked_crthits_v));

  return;

}
DEFINE_ART_MODULE(lar::CRTpreOverlay)
