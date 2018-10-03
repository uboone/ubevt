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

#include "WireCellChannelStatusProvider.h"
#include "larevt/CalibrationDBI/IOVData/ChannelStatus.h"

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
    return channel < fNumChannels;
  }

  // Is this a bad channel?

  bool WireCellChannelStatusProvider::IsBad(raw::ChannelID_t channel) const
  {
    return fBadChannels.count(channel) > 0;
  }

  // Is this a noisy channel?
  // As far as this class is concerned, no channel is ever noisy (only bad).

  bool WireCellChannelStatusProvider::IsNoisy(raw::ChannelID_t channel) const
  {
    return false;
  }

  // Return channel status.
  // According to this class, channels are either good or dead.

  ChannelStatusProvider::Status_t
  WireCellChannelStatusProvider::Status(raw::ChannelID_t channel) const
  {
    return (IsBad(channel) ? lariov::kDEAD : lariov::kGOOD);
  }

  // Return list of good channels.

  ChannelStatusProvider::ChannelSet_t
  WireCellChannelStatusProvider::GoodChannels() const
  {
    // Generate good channel list on the fly.

    ChannelStatusProvider::ChannelSet_t result;

    for(unsigned int c=0; c<fNumChannels; ++c) {
      if(!IsBad(c))
	result.insert(c);
    }
    return result;
  }

  // Return list of bad channels.

  ChannelStatusProvider::ChannelSet_t
  WireCellChannelStatusProvider::BadChannels() const
  {
    return fBadChannels;
  }

  // Return list of noisy channels (empty set).

  ChannelStatusProvider::ChannelSet_t
  WireCellChannelStatusProvider::NoisyChannels() const
  {
    return ChannelStatusProvider::ChannelSet_t();
  }

  // Update the number of channels.

  void WireCellChannelStatusProvider::updateNumChannels(unsigned int nchannels)
  {
    fNumChannels = nchannels;
  }

  // Clear bad channel list.

  void WireCellChannelStatusProvider::clearBadChannels()
  {
    fBadChannels.clear();
  }

  // Update bad channel list.

  void WireCellChannelStatusProvider::updateBadChannels(const std::vector<int> bad_channels)
  {
    // First clear bad channel list.

    clearBadChannels();

    // Add bad channels one by one.

    for(int c : bad_channels)
      fBadChannels.insert(c);
  }

} // namespace lariov
