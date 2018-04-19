////////////////////////////////////////////////////////////////////////
// Class:       ShiftEdepSCE
// Plugin Type: producer (art v2_05_01)
// File:        ShiftEdepSCE_module.cc
//
// Generated at Thu Apr 19 00:41:18 2018 by Wesley Ketchum using cetskelgen
// from cetlib version v1_21_00.
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

#include <memory>

#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "larevt/SpaceChargeServices/SpaceChargeService.h"

namespace spacecharge {
  class ShiftEdepSCE;
}


class spacecharge::ShiftEdepSCE : public art::EDProducer {
public:
  explicit ShiftEdepSCE(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ShiftEdepSCE(ShiftEdepSCE const &) = delete;
  ShiftEdepSCE(ShiftEdepSCE &&) = delete;
  ShiftEdepSCE & operator = (ShiftEdepSCE const &) = delete;
  ShiftEdepSCE & operator = (ShiftEdepSCE &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

private:

  // Declare member data here.
  art::InputTag fEDepTag;

};


spacecharge::ShiftEdepSCE::ShiftEdepSCE(fhicl::ParameterSet const & p)
  : fEDepTag(p.get<art::InputTag>("EDepTag"))
{
  produces< std::vector<sim::SimEnergyDeposit> >();
}

void spacecharge::ShiftEdepSCE::produce(art::Event & e)
{
  auto sce = lar::providerFrom<spacecharge::SpaceChargeService>();

  std::uniqute_ptr< std::vector<sim::SimEnergyDeposit> > 
    outEdepVecPtr(new std::vector<sim::SimEnergyDeposit>() );
  auto & outEdepVec(*outEdepVecPtr);

  art::Handle< std::vector<sim::SimEnergyDeposit> > inEdepHandle;
  e.getByLabel(fEdepTag,inEdepHandle);
  auto const& inEdepVec(*inEdepHandle);
  
  outEdepVec.reserve(inEdepVec.size());
  
  std::vector<double> posOffsets{0.0,0.0,0.0};
  for(auto const& edep : inEdepVec){
    if(sce->EnableSimSpatialSCE())
      posOffsets = sce->GetPosOffsets(edep.X(),edep.Y(),edep.Z());
    outEdepVec.emplace_back(edep.NumPhotons(),
			    edep.NumElectrons(),
			    edep.Energy(),
			    {edep.StartX()+posOffsets[0],edep.StartY+posOffsets[1],edep.StartZ()+posOffsets[2]},
			    {edep.EndX()+posOffsets[0],edep.EndY+posOffsets[1],edep.EndZ()+posOffsets[2]},
			    edep.StartT(),
			    edep.EndT(),
			    edep.TrackID(),
			    edep.PdgCode());
  }

  e.put(std::move(outEdepVecPtr));
}

DEFINE_ART_MODULE(spacecharge::ShiftEdepSCE)
