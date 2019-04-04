////////////////////////////////////////////////////////////////////////
// Class:       G4InfoMixerLocal
// Module Type: producer
// File:        G4InfoMixerLocal_module.cc
//
// This borrows a lot from the Mu2e mixing module:
//      EventMixing/src/MixMCEvents_module.cc
//
// The purpose of this filter module is to copy in g4-level 
// information into a data file.
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

#include "nusimdata/SimulationBase/MCParticle.h"
#include "lardataobj/Simulation/SimPhotons.h"
#include "lardataobj/Simulation/SimChannel.h"
#include "lardataobj/Simulation/AuxDetSimChannel.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"

namespace mix {
  class G4InfoMixerLocal;
}

class mix::G4InfoMixerLocal : public boost::noncopyable {
public:

  G4InfoMixerLocal(fhicl::ParameterSet const& p,
		    art::MixHelper &helper);

  size_t nSecondaries() { return fEventsToMix; } 

  // Mixing Functions

  //for MC collections we want just a simple copies of the collections...
  template<typename T>
  bool MixSimpleCopy( std::vector< std::vector<T> const*> const& inputs,
		      std::vector< T > & output,
		      art::PtrRemapper const &);

private:

  // Declare member data here.
  fhicl::ParameterSet           fpset;
  std::vector<art::InputTag>    fMCParticleInputModuleLabels;
  std::vector<art::InputTag>    fSimEnergyDepositInputModuleLabels;
  std::vector<art::InputTag>    fAuxDetSimChannelInputModuleLabels;
  std::vector<art::InputTag>    fSimChannelInputModuleLabels;
  std::vector<art::InputTag>    fSimPhotonsInputModuleLabels;
  size_t                        fEventsToMix;  
};


mix::G4InfoMixerLocal::G4InfoMixerLocal(fhicl::ParameterSet const& p,
					  art::MixHelper &helper)
  :
  fpset(p.get<fhicl::ParameterSet>("detail")),
  fMCParticleInputModuleLabels(fpset.get<std::vector<art::InputTag>>
			       ("MCParticleInputModuleLabels",std::vector<art::InputTag>())),
  fSimEnergyDepositInputModuleLabels(fpset.get<std::vector<art::InputTag>>
				     ("SimEnergyDepositInputModuleLabels",std::vector<art::InputTag>())),
  fAuxDetSimChannelInputModuleLabels(fpset.get<std::vector<art::InputTag>>
				     ("AuxDetSimChannelInputModuleLabels",std::vector<art::InputTag>())),
  fSimChannelInputModuleLabels(fpset.get<std::vector<art::InputTag>>
			       ("SimChannelInputModuleLabels",std::vector<art::InputTag>())),
  fSimPhotonsInputModuleLabels(fpset.get<std::vector<art::InputTag>>
			       ("SimPhotonsInputModuleLabels",std::vector<art::InputTag>())),
  fEventsToMix(fpset.get<size_t>("EventsToMix",1))
{
  
  if(fEventsToMix!=1){
    std::stringstream err_str;
    err_str << "ERROR! Really sorry, but we can only do mixing for one collection right now! ";
    err_str << "\nYep. We're gonna throw an exception now. You should change your fcl to set 'EventsToMix' to 1";
    throw cet::exception("G4InfoMixerLocal") << err_str.str() << std::endl;;
  }

  //MC generator info is a simple copy
  for(auto label : fMCParticleInputModuleLabels)
    helper.declareMixOp( label,
			 &G4InfoMixerLocal::MixSimpleCopy<simb::MCParticle>,
			 *this );
  for(auto label : fSimEnergyDepositInputModuleLabels)
    helper.declareMixOp( label,
			 &G4InfoMixerLocal::MixSimpleCopy<sim::SimEnergyDeposit>,
			 *this );
  for(auto label : fAuxDetSimChannelInputModuleLabels)
    helper.declareMixOp( label,
			 &G4InfoMixerLocal::MixSimpleCopy<sim::AuxDetSimChannel>,
			 *this );
  for(auto label : fSimChannelInputModuleLabels)
    helper.declareMixOp( label,
			 &G4InfoMixerLocal::MixSimpleCopy<sim::SimChannel>,
			 *this );
  for(auto label : fSimPhotonsInputModuleLabels)
    helper.declareMixOp( label,
			 &G4InfoMixerLocal::MixSimpleCopy<sim::SimPhotons>,
			 *this );

  //If it produces something on its own, declare it here
  //helper.produces< std::vector<mix::EventMixingSummary> >();

}

/*
//Initialize for each event
void mix::G4InfoMixerLocal::startEvent(const art::Event& event) {

  if(!( (event.isRealData() && fInputFileIsData) || (!event.isRealData() && !fInputFileIsData)))
    throw cet::exception("OverlayRawDataMicroBooNE") << "Input file claimed to be data/not data, but it's not." << std::endl;;

  fEventMixingSummary.reset(new std::vector<mix::EventMixingSummary>);
}

//For each of the mixed in events...bookkepping for event IDs
void mix::G4InfoMixerLocal::processEventAuxiliaries(art::EventAuxiliarySequence const & seq){
  for (auto const& ev : seq)
    fEventMixingSummary->emplace_back(ev.id().event(),ev.id().subRun(),ev.id().run(),ev.time());
}

//End each event
void mix::G4InfoMixerLocal::finalizeEvent(art::Event& event) {
  event.put(std::move(fEventMixingSummary));
}
*/

template<typename T>
bool mix::G4InfoMixerLocal::MixSimpleCopy( std::vector< std::vector<T> const*> const& inputs,
				       std::vector< T > & output,
				       art::PtrRemapper const &){
  art::flattenCollections(inputs,output);
  return true;
}

using Module_t = art::MixFilter<mix::G4InfoMixerLocal,art::RootIOPolicy>;
DEFINE_ART_MODULE(Module_t)
