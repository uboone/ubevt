#ifndef WIRECELLCHANNELSTATUSPROVIDER_H
#define WIRECELLCHANNELSTATUSPROVIDER_H

//*********************************************************************************
//
// Name: WireCellChannelStatusProvider
//
// Purpose: This class is an implementation of the larsoft provider interface
//          lariov::ChannelStatusProvider.
//
//          This class owns a list of bad channels, which is updated from 
//          the associated service.
//
//          Channel status returned by this class is only "good" (kGOOD=4) or
//          "bad" (kDEAD=1).  Refer to larevt/CalibrationDBI/IOVData/ChannelStatus.h.
//
//          This class does not have any configuration parameters.
//
// Created: 2-Oct-2018, H. Greenlee
//
// 
//*********************************************************************************

#include <vector>
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"

namespace lariov {

  class WireCellChannelStatusProvider : public ChannelStatusProvider
  {
  public:

    // Constructors, destructor.

    WireCellChannelStatusProvider();                                       // Default constructor.
    WireCellChannelStatusProvider(ChannelStatusProvider const&) = delete;  // Uncopyable.
    WireCellChannelStatusProvider(ChannelStatusProvider&&) = delete;       // Unmovable.
    ~WireCellChannelStatusProvider();                                      // Destructor.

    // Operators.

    WireCellChannelStatusProvider& operator = (ChannelStatusProvider const&) = delete;
    WireCellChannelStatusProvider& operator = (ChannelStatusProvider&&) = delete;

    // Overrides.

    bool IsPresent(raw::ChannelID_t channel) const;
    bool IsBad(raw::ChannelID_t channel) const;
    bool IsNoisy(raw::ChannelID_t channel) const;
    Status_t Status(raw::ChannelID_t channel) const;
    ChannelSet_t GoodChannels() const;
    ChannelSet_t BadChannels() const;
    ChannelSet_t NoisyChannels() const;

    // Methods.

    void updateNumChannels(unsigned int nchannels);
    void clearBadChannels();
    void updateBadChannels(const std::vector<int> bad_channels);

  private:

    // Data members.

    unsigned int fNumChannels;
    ChannelSet_t fBadChannels;

  };
} // namespace lariov

#endif
