//*********************************************************************************
//
// Name: WireCellChannelStatusProvider
//
// Purpose: Implementation for WireCellChannelStatusProvider.
//
// Created: 2-Oct-2018, H. Greenlee
//
// 
//*********************************************************************************

#include <iostream>
#include "WireCellChannelStatusProvider.h"
#include "larevt/CalibrationDBI/IOVData/ChannelStatus.h"
#include "larevt/CalibrationDBI/Providers/SIOVChannelStatusProvider.h"

namespace lariov {

  // Default Constructor.

  WireCellChannelStatusProvider::WireCellChannelStatusProvider()
  {}

  // Destructor.

  WireCellChannelStatusProvider::~WireCellChannelStatusProvider()
  {}

  // Overrides.

  // Is channel present?

  bool WireCellChannelStatusProvider::IsPresent(raw::ChannelID_t channel) const
  {
    bool result = (channel < fNumChannels);
    if(result && fDBProvider)
      result = fDBProvider->IsPresent(channel);
    return result;
  }

  // Is this a bad channel?

  bool WireCellChannelStatusProvider::IsBad(raw::ChannelID_t channel) const
  {
    bool result = (fBadChannels.count(channel) > 0);
    if(result && fDBProvider)
      result = fDBProvider->IsBad(channel);
    return result;
  }

  // Is this a noisy channel?
  // As far as this class is concerned, no channel is ever noisy (only bad).

  bool WireCellChannelStatusProvider::IsNoisy(raw::ChannelID_t channel) const
  {
    bool result = false;
    if(result && fDBProvider)
      result = fDBProvider->IsNoisy(channel);
    return result;
  }

  // Return channel status.
  // According to this class, channels are either good or dead.

  ChannelStatusProvider::Status_t
  WireCellChannelStatusProvider::Status(raw::ChannelID_t channel) const
  {
    ChannelStatusProvider::Status_t result = (IsBad(channel) ? lariov::kDEAD : lariov::kGOOD);
    if(fDBProvider) {
      ChannelStatusProvider::Status_t dbresult = fDBProvider->Status(channel);
      if(dbresult < result)
	result = dbresult;
    }
    return result;
  }

  // Return list of good channels.

  ChannelStatusProvider::ChannelSet_t
  WireCellChannelStatusProvider::GoodChannels() const
  {
    // Generate good channel list on the fly.

    ChannelStatusProvider::ChannelSet_t result;

    for(unsigned int c=0; c<fNumChannels; ++c) {
      if(IsPresent(c) && !IsBad(c) && !IsNoisy(c))
	result.insert(c);
    }
    return result;
  }

  // Return list of bad channels.

  ChannelStatusProvider::ChannelSet_t
  WireCellChannelStatusProvider::BadChannels() const
  {
    // Generate bad channel list on the fly.

    ChannelStatusProvider::ChannelSet_t result;

    for(unsigned int c=0; c<fNumChannels; ++c) {
      if(IsBad(c))
	result.insert(c);
    }
    return result;
  }

  // Return list of noisy channels.

  ChannelStatusProvider::ChannelSet_t
  WireCellChannelStatusProvider::NoisyChannels() const
  {
    // Generate noisy channel list on the fly.

    ChannelStatusProvider::ChannelSet_t result;

    for(unsigned int c=0; c<fNumChannels; ++c) {
      if(IsNoisy(c))
	result.insert(c);
    }
    return result;
  }

  WireCellChannelStatusProvider::ChannelMask
  WireCellChannelStatusProvider::StatusMask(raw::ChannelID_t channel) const
  {
    ChannelMask result;    // Default-constructed (invalid) mask.

    auto const& it = fBadMasks.find(channel);
    if(it != fBadMasks.end())
      result = it->second;

    // Done.

    return result;
  }

  const WireCellChannelStatusProvider::ChannelMap_t&
  WireCellChannelStatusProvider::BadMasks() const
  {
    return fBadMasks;
  }

  // Update the number of channels.

  void WireCellChannelStatusProvider::updateNumChannels(unsigned int nchannels)
  {
    fNumChannels = nchannels;
  }

  // Clear bad channel list.

  void WireCellChannelStatusProvider::clearBadMasks()
  {
    fBadChannels.clear();
    fBadMasks.clear();
  }

  // Update bad channel list.

  void WireCellChannelStatusProvider::updateBadMasks(const std::vector<int> badmasks)
  {
    // First clear bad channel list.

    clearBadMasks();

    // Add bad channels one by one.

    for(size_t i=0; i<badmasks.size()-2; i+=3) {
      raw::ChannelID_t c = badmasks[i];
      int first = badmasks[i+1];
      int last = badmasks[i+2];
      fBadChannels.insert(c);
      fBadMasks.emplace(c, ChannelMask(first, last));
      //std::cout << "Channel=" << c << " (" << first << ", " << last << ")" << std::endl;
    }
  }

  // Add a secondary database provider.

  void WireCellChannelStatusProvider::addDBProvider(fhicl::ParameterSet pset)
  {
    fDBProvider = std::unique_ptr<SIOVChannelStatusProvider>(new SIOVChannelStatusProvider(pset));
  }

} // namespace lariov
