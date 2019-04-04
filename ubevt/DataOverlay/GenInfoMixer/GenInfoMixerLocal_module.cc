////////////////////////////////////////////////////////////////////////
// Class:       GenInfoMixerLocal
// Module Type: producer
// File:        GenInfoMixerLocal_module.cc
//
// This borrows a lot from the Mu2e mixing module:
//      EventMixing/src/MixMCEvents_module.cc
//
// The purpose of this filter module is to copy in generator-level 
// information (from GENIE) into a data file. 
////////////////////////////////////////////////////////////////////////

#include "art_root_io/RootIOPolicy.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/MixFilter.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Persistency/Common/CollectionUtilities.h"
#include "canvas/Utilities/InputTag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <string>
#include <vector>
#include <exception>
#include <sstream>
#include <unistd.h>

#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "lardataobj/Simulation/BeamGateInfo.h"

namespace mix {
  class GenInfoMixerLocal;
}

class mix::GenInfoMixerLocal : public boost::noncopyable {
public:

  GenInfoMixerLocal(fhicl::ParameterSet const& p,
		    art::MixHelper &helper);
  //  ~GenInfoMixerLocal() {};

  //void startEvent(const art::Event&);  //called at the start of every event
  //void finalizeEvent(art::Event &);    //called at the end of every event
  
  size_t nSecondaries() { return fEventsToMix; } 

  //void processEventAuxiliaries(art::EventAuxiliarySequence const& seq); //bookkepping for event IDs

  // Mixing Functions

  //for MC collections we want just a simple copies of the collections...
  template<typename T>
  bool MixSimpleCopy( std::vector< std::vector<T> const*> const& inputs,
		      std::vector< T > & output,
		      art::PtrRemapper const &);

private:

  // Declare member data here.
  fhicl::ParameterSet fpset;
  art::InputTag       fGeneratorInputModuleLabel;
  size_t              fEventsToMix;  
};


mix::GenInfoMixerLocal::GenInfoMixerLocal(fhicl::ParameterSet const& p,
					  art::MixHelper &helper)
  :
  fpset(p.get<fhicl::ParameterSet>("detail")),
  fGeneratorInputModuleLabel(fpset.get<art::InputTag>("GeneratorInputModuleLabel")),
  fEventsToMix(fpset.get<size_t>("EventsToMix",1))
{
  
  if(fEventsToMix!=1){
    std::stringstream err_str;
    err_str << "ERROR! Really sorry, but we can only do mixing for one collection right now! ";
    err_str << "\nYep. We're gonna throw an exception now. You should change your fcl to set 'EventsToMix' to 1";
    throw cet::exception("GenInfoMixerLocal") << err_str.str() << std::endl;;
  }

  //MC generator info is a simple copy
  helper.declareMixOp( fGeneratorInputModuleLabel,
		       &GenInfoMixerLocal::MixSimpleCopy<simb::MCTruth>,
		       *this );
  helper.declareMixOp( fGeneratorInputModuleLabel,
		       &GenInfoMixerLocal::MixSimpleCopy<simb::GTruth>,
		       *this );
  helper.declareMixOp( fGeneratorInputModuleLabel,
		       &GenInfoMixerLocal::MixSimpleCopy<simb::MCFlux>,
		       *this );
  helper.declareMixOp( fGeneratorInputModuleLabel,
		       &GenInfoMixerLocal::MixSimpleCopy<sim::BeamGateInfo>,
		       *this );
  //If it produces something on its own, declare it here
  //helper.produces< std::vector<mix::EventMixingSummary> >();

}

/*
//Initialize for each event
void mix::GenInfoMixerLocal::startEvent(const art::Event& event) {

  if(!( (event.isRealData() && fInputFileIsData) || (!event.isRealData() && !fInputFileIsData)))
    throw cet::exception("OverlayRawDataMicroBooNE") << "Input file claimed to be data/not data, but it's not." << std::endl;;

  fEventMixingSummary.reset(new std::vector<mix::EventMixingSummary>);
}

//For each of the mixed in events...bookkepping for event IDs
void mix::GenInfoMixerLocal::processEventAuxiliaries(art::EventAuxiliarySequence const & seq){
  for (auto const& ev : seq)
    fEventMixingSummary->emplace_back(ev.id().event(),ev.id().subRun(),ev.id().run(),ev.time());
}

//End each event
void mix::GenInfoMixerLocal::finalizeEvent(art::Event& event) {
  event.put(std::move(fEventMixingSummary));
}
*/

template<typename T>
bool mix::GenInfoMixerLocal::MixSimpleCopy( std::vector< std::vector<T> const*> const& inputs,
				       std::vector< T > & output,
				       art::PtrRemapper const &){
  art::flattenCollections(inputs,output);
  return true;
}

/*
// Return next file to mix.
std::string mix::GenInfoMixerLocal::getMixFile()
{
  std::string result;

  if(!fSamProcessID.empty()) {

    // Get IFDH art service.

    art::ServiceHandle<ifdh_ns::IFDH> ifdh;

    // Update status of current file, if any, to "consumed."

    if(!fSamCurrentFileName.empty()) {
      ifdh->updateFileStatus(fSamProjectURI,
			     fSamProcessID,
			     fSamCurrentFileName,
			     "consumed");

      //std::cout << "Mix SAM: File " << fSamCurrentFileName << " status changed to consumed." << std::endl;
      fSamCurrentFileURI = std::string();
      fSamCurrentFileName = std::string();
    }

    // Get next file uri.

    fSamCurrentFileURI = fSamCurrentFileURI = ifdh->getNextFile(fSamProjectURI,	fSamProcessID);
    unsigned int n = fSamCurrentFileURI.find_last_of('/') + 1;
    fSamCurrentFileName = fSamCurrentFileURI.substr(n);
    //std::cout << "Mix SAM: Next file uri = " << fSamCurrentFileURI << std::endl;
    //std::cout << "Mix SAM: Next file name = " << fSamCurrentFileName << std::endl;
    mf::LogInfo("OverlayRawDigitMicroBooNE") << "Next mix file uri: " << fSamCurrentFileURI << "\n"
					     << "Next mix file name: " << fSamCurrentFileName;

    // Throw an exception if we didn't get a next file.

    if(fSamCurrentFileURI.empty() || fSamCurrentFileName.empty())
      throw cet::exception("OverlayRawDataMicroBooNE") << "Failed to get next mix file.";

    // Here is where we would copy the file to the local node, if that were necessary.
    // Since we are using schema "root" (i.e. xrootd) to stream files, copying the 
    // file is not necessary.
    // Note further that we should not update the file status to "transferred" for
    // streaming files, since that can in principle allow the file to be deleted from
    // disk cache.

    // Update metadata.

    art::ServiceHandle<art::FileCatalogMetadata> md;
    md->addMetadataString("mixparent", fSamCurrentFileName);

    // Done.

    result = fSamCurrentFileURI;
  }

  return result;
}
*/

DEFINE_ART_MODULE((art::MixFilter<mix::GenInfoMixerLocal,art::RootIOPolicy>))
