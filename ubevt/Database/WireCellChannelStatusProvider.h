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
//          When accessed through the ChannelStatusProvider interface, channel
//          status returned by this class is only "good" (kGOOD=4) or
//          "bad" (kDEAD=1).  Refer to larevt/CalibrationDBI/IOVData/ChannelStatus.h.
//
//          In addition to inherited methods, this class provides a wider
//          interface that is able to identify good and back tick intervals
//          in particular channels.
//
//          This class does not have any configuration parameters (has only default
//          constructor).  However, this class does have per-event state information
//          that is updated from its owning service.
//
//          This class has the ability to own a secondary database provider.  If
//          it is used in this mode, the returned channel status is a combinarion
//          of the per-event status as read from the badmask data product, and the
//          database status (whichever is worse, defined as the smaller status enum).
//
// Created: 2-Oct-2018, H. Greenlee
//
// 
//*********************************************************************************

#include <vector>
#include <map>
#include "fhiclcpp/ParameterSet.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"

namespace lariov {

  class WireCellChannelStatusProvider : public ChannelStatusProvider
  {
  public:

    // Types.

    // Struct ChannelMask represents a tick interval for one channel.

    struct ChannelMask {
      int first;
      int last;
      bool valid;
    ChannelMask() : first(0), last(0), valid(false) {}
    ChannelMask(int f, int l) : first(f), last(l), valid(true) {}
    };

    typedef std::map<raw::ChannelID_t, ChannelMask> ChannelMap_t;

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

    // Additional accessors.

    ChannelMask StatusMask(raw::ChannelID_t channel) const;
    const ChannelMap_t& BadMasks() const;

    // Updating methods.

    void updateNumChannels(unsigned int nchannels);
    void clearBadMasks();
    void updateBadMasks(const std::vector<int> badmasks);
    void addDBProvider(fhicl::ParameterSet pset);

  private:

    // Data members.

    unsigned int fNumChannels;
    ChannelSet_t fBadChannels;
    ChannelMap_t fBadMasks;
    std::unique_ptr<ChannelStatusProvider> fDBProvider;
  };
} // namespace lariov

#endif
