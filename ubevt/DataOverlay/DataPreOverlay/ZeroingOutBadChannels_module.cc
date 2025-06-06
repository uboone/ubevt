////////////////////////////////////////////////////////////////////////
// Class:       ZeroingOutBadChannels
// Plugin Type: producer (art v2_11_03)
// File:        ZeroingOutBadChannels_module.cc
//
// Generated at Fri Nov  2 13:48:18 2018 by Christopher Barnes using cetskelgen
// Modified  at Mon Feb 18 by Adi Ashkenazi adding simulated pedestal 
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

//includes for RawDigits.
#include "lardataobj/RawData/RawDigit.h"

#include <memory>

namespace lar {
  class ZeroingOutBadChannels;
}


class lar::ZeroingOutBadChannels : public art::EDProducer {
public:
  explicit ZeroingOutBadChannels(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ZeroingOutBadChannels(ZeroingOutBadChannels const &) = delete;
  ZeroingOutBadChannels(ZeroingOutBadChannels &&) = delete;
  ZeroingOutBadChannels & operator = (ZeroingOutBadChannels const &) = delete;
  ZeroingOutBadChannels & operator = (ZeroingOutBadChannels &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

private:
  
  // This is the name of the producer of the RawDigits.
  std::string fRawDigitLabel;
  std::string fRawDigitInstanceName;
  std::string fBadMaskLabel;
  std::string fBadMaskInstanceName;
  bool        verbose;

  int         event_counter;
  int         bad_adc_counter;

};


lar::ZeroingOutBadChannels::ZeroingOutBadChannels(fhicl::ParameterSet const & p)
  : EDProducer{p}
// Initialize member data here.
{
  // Call appropriate produces<>() functions here.
  produces<std::vector<raw::RawDigit>>();

  fRawDigitLabel        = p.get<std::string>("RawDigitLabel");
  fRawDigitInstanceName = p.get<std::string>("RawDigitInstanceName");
  fBadMaskLabel         = p.get<std::string>("BadMaskLabel");
  fBadMaskInstanceName  = p.get<std::string>("BadMaskInstanceName");
  verbose               = p.get<bool>("verbose");

  event_counter         = 0;
  bad_adc_counter       = 0;

}

void lar::ZeroingOutBadChannels::produce(art::Event & e)
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

  // Implementation of required member function here.
  // Read in the RawDigits of the overlay.                                                                                                                                                              
  art::Handle<std::vector<raw::RawDigit> > rawdigit_h;
  e.getByLabel( fRawDigitLabel, fRawDigitInstanceName , rawdigit_h );

  // make sure RawDigits look good.                                                                                                                                                                  
  if(!rawdigit_h.isValid()) {
    std::cerr<<"\033[93m[ERROR]\033[00m ... could not locate Raw Digits!"<<std::endl;
    throw std::exception();
  }

  // Read in a vector of the dead channels and the dead times.
  art::Handle< std::vector<int> > bad_channels_h;
  e.getByLabel(fBadMaskLabel, fBadMaskInstanceName, bad_channels_h);

  // make sure bad channels look good.
  if (!bad_channels_h.isValid()) {
    std::cerr<<"\033[93m[ERROR]\033[00m ... could not locate Bad Channels!"<<std::endl;
    throw std::exception();
  }

  // Declare the products that we will be outputing from this module.
  std::unique_ptr< std::vector<raw::RawDigit> > zeroed_RawDigit_v(new std::vector<raw::RawDigit>);

  // Append the raw digits to the event and return if the RawDigits are empty.
  if ( rawdigit_h->size() == 0 ) {

    e.put(std::move(zeroed_RawDigit_v));

    return;

  }
  
  // Declare a map between the location in raw digits and the channel value.
  // First entry: the channel.
  // Second entry: the entry in the 'rawdigit_h' vector.
  std::map<size_t,size_t> raw_digit_channel_map;
  raw_digit_channel_map.clear();

  // Loop through the old RawDigits and set the new RawDigits equal to all of those values.

  // Loop over the raw digits.                                                                                                                                                                         
  for ( size_t i = 0; i < rawdigit_h->size(); i++ ) {

    // Get all of the information for the old RawDigits.
    raw::ChannelID_t           channel           = rawdigit_h->at( i ).Channel();
    unsigned short             samples           = rawdigit_h->at( i ).Samples();
    raw::RawDigit::ADCvector_t adc_counts        = rawdigit_h->at( i ).ADCs();
    raw::Compress_t            compression       = rawdigit_h->at( i ).Compression();

    // Fill in the entry in the 'raw_digit_channel_map' vector.
    std::vector< size_t > raw_digit_single_channel_map;
    raw_digit_single_channel_map.clear();
    raw_digit_single_channel_map.push_back( size_t(channel) );
    raw_digit_single_channel_map.push_back( i );

    // Push back this vector to 'raw_digit_channel_map'.
    raw_digit_channel_map[channel] = i;

    // Use this to make a new RawDigits product.
    raw::RawDigit raw_digit( channel, samples, adc_counts, compression );
    raw_digit.SetPedestal (rawdigit_h->at( i ).GetPedestal());
    zeroed_RawDigit_v->emplace_back( raw_digit );

  } // End of the loop over the RawDigits.

  // Declare the variables that will be used in the following loop.
  size_t channel;
  int    starting_time_tick;
  int    ending_time_tick;

  // Loop through the 'bad_channels' data product to find which bad wires you should set to 0.
  for ( size_t bad_channel_iter = 0; bad_channel_iter < bad_channels_h->size(); bad_channel_iter += 3 ) {

    // Save the value of the wire, the starting time, and the ending time.
    channel            = size_t(bad_channels_h->at( bad_channel_iter ));
    starting_time_tick = bad_channels_h->at( bad_channel_iter + 1 );
    ending_time_tick   = bad_channels_h->at( bad_channel_iter + 2 );

    // Using our map, convert the channel value to the value of the index of the 'rawdigit_h' vector.
    auto m_iter = raw_digit_channel_map.find(channel);
    if(m_iter==raw_digit_channel_map.end()) continue;
    size_t rawdigit_h_idx = m_iter->second;

    // Unpack the values from this entry in the 'zeroed_RawDigit_v' vector using the 'rawdigit_h_idx'.
    // Get all of the information for the old RawDigits.                                                                                                                                                   
    unsigned short             samples           = rawdigit_h->at( rawdigit_h_idx ).Samples();
    raw::RawDigit::ADCvector_t adc_counts        = rawdigit_h->at( rawdigit_h_idx ).ADCs();
    raw::Compress_t            compression       = rawdigit_h->at( rawdigit_h_idx ).Compression();
    
    // Increment bad_adc_counter.
    bad_adc_counter++;

    // Loop through the time ticks to set the entries at the affected time ticks equal to 0.
    for ( size_t j = starting_time_tick; j < size_t(ending_time_tick); j++ ) {

      adc_counts.at( j ) = rawdigit_h->at( rawdigit_h_idx ).GetPedestal(); 
    
    } // End of the loop over setting the time ticks to 0.

    // Make a new raw::RawDigit product with the correct adc counts.
    raw::RawDigit new_raw_digit( channel, samples, adc_counts, compression );
    new_raw_digit.SetPedestal (rawdigit_h->at( rawdigit_h_idx ).GetPedestal());

    // Set the value of 'zeroed_RawDigit_v' at this entry equal to the new raw digit.
    zeroed_RawDigit_v->at( rawdigit_h_idx ) = new_raw_digit;

  } // End of the loop over the effected channels.

  // Add the new raw digits to the event.
  e.put(std::move(zeroed_RawDigit_v));

  return;

}
DEFINE_ART_MODULE(lar::ZeroingOutBadChannels)
