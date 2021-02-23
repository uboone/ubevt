////////////////////////////////////////////////////////////////////////
// Class:       ZeroingOutBadChannelsAna
// Plugin Type: analyzer (art v2_11_03)
// File:        ZeroingOutBadChannelsAna_module.cc
//
// Generated at Mon Nov 12 09:19:41 2018 by Christopher Barnes using cetskelgen
// from cetlib version v3_03_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "art_root_io/TFileService.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// Include a header for RawDigit.
#include "lardataobj/RawData/RawDigit.h"

// ROOT
#include "TTree.h"

namespace lar {
  class ZeroingOutBadChannelsAna;
}


class lar::ZeroingOutBadChannelsAna : public art::EDAnalyzer {
public:
  explicit ZeroingOutBadChannelsAna(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ZeroingOutBadChannelsAna(ZeroingOutBadChannelsAna const &) = delete;
  ZeroingOutBadChannelsAna(ZeroingOutBadChannelsAna &&) = delete;
  ZeroingOutBadChannelsAna & operator = (ZeroingOutBadChannelsAna const &) = delete;
  ZeroingOutBadChannelsAna & operator = (ZeroingOutBadChannelsAna &&) = delete;

  void beginJob() override;

  // Required functions.
  void analyze(art::Event const & e) override;

private:

  std::string    fRawDigitLabel;
  std::string    fRawDigitInstanceName;
  bool           verbose;

  TTree*         raw_digit_analysis_tree;

  unsigned int   channel;
  unsigned short samples;
  int            time_tick;
  short          adc_count;
  unsigned int   compression;

  int            event_count;

};


lar::ZeroingOutBadChannelsAna::ZeroingOutBadChannelsAna(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)  // ,
 // More initializers here.
{
  fRawDigitLabel               = p.get<std::string>("RawDigitLabel"    );
  fRawDigitInstanceName        = p.get<std::string>("RawDigitInstanceName"       );
  verbose                      = p.get<bool>("verbose"  );
}

void lar::ZeroingOutBadChannelsAna::beginJob() {

  // Declare a tree and place products inside of it.                                                                                                                                                
  art::ServiceHandle<art::TFileService> tfs;
  raw_digit_analysis_tree = tfs->make<TTree>("tree","ADC Count Tree");
  raw_digit_analysis_tree->Branch("channel",&channel,"channel/I");
  raw_digit_analysis_tree->Branch("samples",&samples,"samples/S");
  raw_digit_analysis_tree->Branch("time_tick",&time_tick,"time_tick/I");
  raw_digit_analysis_tree->Branch("adc_count",&adc_count,"adc_count/S");
  raw_digit_analysis_tree->Branch("compression",&compression, "compression/I");

  event_count = 0;
  
}

void lar::ZeroingOutBadChannelsAna::analyze(art::Event const & e)
{

  if ( verbose )
    std::cout << "Event #" << event_count << "." << std::endl;
  
  event_count++;

  if ( verbose ) {
    // Print out the run, subrun, and event.                                                                                                                                                      
    std::cout << "run = " << e.run() << "." << std::endl;
    std::cout << "subrun = " << e.subRun() << "." << std::endl;
    std::cout << "event = " << e.event() << "." << std::endl;
  }

  // Implementation of required member function here.
  // Read in the RawDigits of the overlay.                                                                                                                                                                
  art::Handle<std::vector<raw::RawDigit> > rawdigit_h;
  e.getByLabel( fRawDigitLabel, fRawDigitInstanceName , rawdigit_h );

  // make sure RawDigits look good.                                                                                                                                                              
  if(!rawdigit_h.isValid()) {
    std::cerr<<"\033[93m[ERROR]\033[00m ... could not locate Raw Digits!"<<std::endl;
    throw std::exception();
  }

  size_t raw_digit = 1; // (Placeholder for now.)

  // Pick out the raw digit that contains the dead channels you want to look at.
  // Get all of the information for the old RawDigits.                                                                                                                                                    
  channel                                      = rawdigit_h->at( raw_digit ).Channel();
  samples                                      = rawdigit_h->at( raw_digit ).Samples();
  raw::RawDigit::ADCvector_t adc_counts        = rawdigit_h->at( raw_digit ).ADCs();
  compression                                  = rawdigit_h->at( raw_digit ).Compression();

  // Loop through the sample and fill the tree.
  for ( time_tick = 0; time_tick < int( adc_counts.size() ); time_tick++ ) {

    // Set the value of 'adc_count'.
    adc_count = adc_counts.at( time_tick );

    raw_digit_analysis_tree->Fill();

  }

}

DEFINE_ART_MODULE(lar::ZeroingOutBadChannelsAna)
