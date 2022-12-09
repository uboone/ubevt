////////////////////////////////////////////////////////////////////////
// Class:       ChannelStatusAna
// Plugin Type: analyzer (art v3_01_02)
// File:        ChannelStatusAna_module.cc
//
// Purpose: Dump channel status from database and wire cell.
//
// Generated at Fri Dec  2 11:56:51 2022 by Herbert Greenlee using cetskelgen
// from cetlib version v3_05_01.
////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"
#include "TH1F.h"

class ChannelStatusAna;


class ChannelStatusAna : public art::EDAnalyzer {
public:
  explicit ChannelStatusAna(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ChannelStatusAna(ChannelStatusAna const&) = delete;
  ChannelStatusAna(ChannelStatusAna&&) = delete;
  ChannelStatusAna& operator=(ChannelStatusAna const&) = delete;
  ChannelStatusAna& operator=(ChannelStatusAna&&) = delete;

  // Overrides.
  void analyze(art::Event const& e) override;
  void beginJob() override;

private:

  // Declare member data here.

  bool fFromService;   // Flag to dump channel status from channel status service.
  bool fFromWireCell;  // Flag to dump wire cell live channel status.
  bool fDump;          // Dump flag.
  std::string fBadChannelModuleLabel;  // Bad channel module label.

  // Histograms.

  TH1F* hS;       // Number of channel status service bad channels.
  TH1F* hWC;      // Number of wire cell bad channels.
  TH1F* hBoth;    // Number of both bad channels.
  TH1F* hSonly;   // Number of channel status only bad channels.
  TH1F* hWConly;  // Number of wire cell only bad channels.
};


ChannelStatusAna::ChannelStatusAna(fhicl::ParameterSet const& p)
  : EDAnalyzer{p},
  fFromService(p.get<bool>("FromService", true)),
  fFromWireCell(p.get<bool>("FromWireCell", true)),
  fDump(p.get<bool>("Dump", true)),
  fBadChannelModuleLabel(p.get<std::string>("BadChannelModuleLabel", "nfspl1"))
{
  std::cout << "\nChannelStatusAna module initialized\n"
            << "Dump from service: " << fFromService << "\n"
            << "Dump from wire cell: " << fFromWireCell << "\n"
            << "Bad channel module label: " << fBadChannelModuleLabel << std::endl;
}

void ChannelStatusAna::analyze(art::Event const& e)
{
  // Print event header information.

  if(fDump) {
    std::cout << "\nChannelStatusAna for run " << e.run()
              << ", subrun " << e.subRun()
              << ", event " << e.event() << std::endl;
  }

  // Define collections (std::sets).

  lariov::ChannelStatusProvider::ChannelSet_t service_good_channels;
  lariov::ChannelStatusProvider::ChannelSet_t service_bad_channels;
  lariov::ChannelStatusProvider::ChannelSet_t service_noisy_channels;

  lariov::ChannelStatusProvider::ChannelSet_t wc_bad_channels;

  if(fFromService) {

    // Get channel status service provider.

    if(fDump)
      std::cout << "\nChannel status from ChannelStatusService:\n" << std::endl;

    const lariov::ChannelStatusProvider& chanFilt = art::ServiceHandle<lariov::ChannelStatusService>()->GetProvider();

    service_good_channels = chanFilt.GoodChannels();
    service_bad_channels = chanFilt.BadChannels();
    service_noisy_channels = chanFilt.NoisyChannels();

    if(fDump) {
      std::cout << service_good_channels.size() << " good channels." << std::endl;
      std::cout << service_bad_channels.size() << " bad channels." << std::endl;
      std::cout << service_noisy_channels.size() << " noisy channels." << std::endl;
    }

    hS->Fill(service_bad_channels.size()); 
  }

  if(fFromWireCell) {

    // Get channel status from wire cell.

    if(fDump)
      std::cout << "\nChannel status from Wire Cell:\n" << std::endl;

    // Get bad channel data product.

    art::Handle<std::vector<int> > badh;
    e.getByLabel(fBadChannelModuleLabel, "badchannels", badh);
    if(badh.isValid()) {
      const std::vector<int>& badchannels = *badh;
      for(int ch : badchannels) {
        wc_bad_channels.insert(ch);
      }
      if(fDump)
        std::cout << wc_bad_channels.size() << " bad channels." << std::endl;
      hWC->Fill(wc_bad_channels.size()); 
    }
    else {
      std::cout << "Bad channel data product not found for label " << fBadChannelModuleLabel << std::endl;
    }
  }

  if(fFromService && fFromWireCell) {

    // Compare service and wire cell.

    if(fDump)
      std::cout << "\nComparing Channel Status Service and Wire Cell.\n" << std::endl;

    std::vector<int> both;
    std::set_intersection(service_bad_channels.begin(),
                          service_bad_channels.end(),
                          wc_bad_channels.begin(),
                          wc_bad_channels.end(),
                          std::back_inserter(both));
    if(fDump)
      std::cout << both.size() << " bad channels for both service and wire cell." << std::endl;
    hBoth->Fill(both.size());
    
    std::vector<int> service_only;
    std::set_difference(service_bad_channels.begin(),
                        service_bad_channels.end(),
                        wc_bad_channels.begin(),
                        wc_bad_channels.end(),
                        std::back_inserter(service_only));
    if(fDump)
      std::cout << service_only.size() << " bad channels for service only." << std::endl;
    hSonly->Fill(service_only.size());
    
    std::vector<int> wc_only;
    std::set_difference(wc_bad_channels.begin(),
                        wc_bad_channels.end(),
                        service_bad_channels.begin(),
                        service_bad_channels.end(),
                        std::back_inserter(wc_only));
    if(fDump)
      std::cout << wc_only.size() << " bad channels for wire cell only." << std::endl;
    hWConly->Fill(wc_only.size());
  }

  // Done.

  if(fDump)
    std::cout << "\nEnd of ChannelStatusAna\n" << std::endl;
}

void ChannelStatusAna::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs;
  hS = tfs->make<TH1F>("Database Bad Channels", "Database Bad Channels",
                       200, 0, 2000.);
  hWC = tfs->make<TH1F>("Wire Cell Bad Channels", "Wire Cell Bad Channels",
                        200, 0, 2000.);
  hBoth = tfs->make<TH1F>("Both Bad Channels", "Both Bad Channels",
                          200, 0, 2000.);
  hSonly = tfs->make<TH1F>("Database Only Bad Channels", "Database Only Bad Channels",
                           100, 0, 500.);
  hWConly = tfs->make<TH1F>("Wire Cell Only Bad Channels", "Wire Cell Only Bad Channels",
                            100, 0, 500.);

}


DEFINE_ART_MODULE(ChannelStatusAna)
