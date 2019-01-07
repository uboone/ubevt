////////////////////////////////////////////////////////////////////////
// Class:       SimInfoOverlayFilter
// Plugin Type: filter (art v2_11_03)
// File:        SimInfoOverlayFilter_module.cc
//
// Generated at Sat Nov 24 16:09:21 2018 by Wesley Ketchum using cetskelgen
// from cetlib version v3_03_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "canvas/Utilities/FriendlyName.h"
#include "art/Persistency/Common/PtrMaker.h"

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>

//gallery...
#include "gallery/Event.h"
#include "gallery/Handle.h"
#include "gallery/ValidHandle.h"

//association includes
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindOne.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindOneP.h"
#include "canvas/Persistency/Common/Assns.h"

//generator products
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "lardataobj/Simulation/BeamGateInfo.h"

//G4 products
#include "nusimdata/SimulationBase/MCParticle.h"
#include "lardataobj/Simulation/SimPhotons.h"
#include "lardataobj/Simulation/SimChannel.h"
#include "lardataobj/Simulation/AuxDetSimChannel.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "lardataobj/Simulation/GeneratedParticleInfo.h"

namespace mix {
  class SimInfoOverlayFilter;
}


class mix::SimInfoOverlayFilter : public art::EDFilter {
public:
  explicit SimInfoOverlayFilter(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.
  
  // Plugins should not be copied or assigned.
  SimInfoOverlayFilter(SimInfoOverlayFilter const &) = delete;
  SimInfoOverlayFilter(SimInfoOverlayFilter &&) = delete;
  SimInfoOverlayFilter & operator = (SimInfoOverlayFilter const &) = delete;
  SimInfoOverlayFilter & operator = (SimInfoOverlayFilter &&) = delete;
  
  // Required functions.
  bool filter(art::Event & e) override;
  
  // Selected optional functions.
  void beginJob() override;
  bool beginRun(art::Run & r) override;
  bool beginSubRun(art::SubRun & sr) override;
  void endJob() override;
  bool endRun(art::Run & r) override;
  bool endSubRun(art::SubRun & sr) override;
  void respondToCloseInputFile(art::FileBlock const & fb) override;
  void respondToCloseOutputFiles(art::FileBlock const & fb) override;
  void respondToOpenInputFile(art::FileBlock const  &fb) override;
  void respondToOpenOutputFiles(art::FileBlock const & fb) override;
  
private:
  
  //Infrastructure to read the event...
  std::vector<std::string> fSimInputFileNames;
  gallery::Event gEvent;
  
  //input tag lists...
  //note, if these are empty, that's ok: then we won't produce them on the event
  
  std::vector<art::InputTag>    fMCTruthInputModuleLabels;
  std::vector<art::InputTag>    fMCFluxInputModuleLabels;
  std::vector<art::InputTag>    fGTruthInputModuleLabels;
  std::vector<art::InputTag>    fBeamGateInputModuleLabels;
  
  std::vector<art::InputTag>    fMCParticleInputModuleLabels;
  std::vector<art::InputTag>    fSimEnergyDepositInputModuleLabels;
  std::vector<art::InputTag>    fAuxDetSimChannelInputModuleLabels;
  std::vector<art::InputTag>    fSimChannelInputModuleLabels;
  std::vector<art::InputTag>    fSimPhotonsInputModuleLabels;
  
  //now for the associations...
  std::vector<art::InputTag>    fMCTruthMCFluxAssnsInputModuleLabels;
  std::vector<art::InputTag>    fMCTruthGTruthAssnsInputModuleLabels;
  std::vector<art::InputTag>    fMCTruthMCParticleAssnsInputModuleLabels;
  
  
  template<class T>
  using CollectionMap = std::unordered_map< std::string, std::unique_ptr<T> >;
  
  CollectionMap< std::vector<simb::MCTruth> >     fMCTruthMap;
  CollectionMap< std::vector<simb::MCFlux> >      fMCFluxMap;
  CollectionMap< std::vector<simb::GTruth> >      fGTruthMap;
  CollectionMap< std::vector<sim::BeamGateInfo> > fBeamGateInfoMap;
  
  CollectionMap< art::Assns<simb::MCTruth,simb::MCFlux,void> > fMCTruthMCFluxAssnsMap;
  CollectionMap< art::Assns<simb::MCTruth,simb::GTruth,void> > fMCTruthGTruthAssnsMap;
  
  CollectionMap< std::vector<simb::MCParticle> >      fMCParticleMap;
  CollectionMap< std::vector<sim::SimEnergyDeposit> > fSimEnergyDepositMap;
  CollectionMap< std::vector<sim::AuxDetSimChannel> > fAuxDetSimChannelMap;
  CollectionMap< std::vector<sim::SimChannel> >       fSimChannelMap;
  CollectionMap< std::vector<sim::SimPhotons> >       fSimPhotonsMap;
  
  CollectionMap< art::Assns<simb::MCTruth,simb::MCParticle,sim::GeneratedParticleInfo> > fMCTruthMCParticleAssnsMap;
  
  //verbose
  int fVerbosity;

  void FillInputModuleLabels(fhicl::ParameterSet const &,
			     std::string,
			     std::vector<art::InputTag> &);
  void FillInputModuleLabels(fhicl::ParameterSet const &);

  template<class T>
  void DeclareProduces(std::vector<art::InputTag> const&);
  void DeclareProduces();

  template<class T>
  void InitializeCollectionMap(std::vector<art::InputTag> const&,
			       CollectionMap<T> &);
  void InitializeCollectionMaps();

  template<class T>
  void PutCollectionOntoEvent(art::Event &,
			      CollectionMap<T> &);
  void PutCollectionsOntoEvent(art::Event &);
  
  template<class T>
  std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> >
  FillCollectionMap(std::vector<art::InputTag> const&,
		    CollectionMap< std::vector<T> > &,
		    std::string,
		    art::Event &);

  template< class T, class U >
  void FillAssnsCollectionMap(std::vector<art::InputTag> const&,
			      CollectionMap< art::Assns<T,U,void> > &,
			      std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> > const&,
			      std::map< std::pair<art::ProductID,size_t> , art::Ptr<U> > const&);
  template< class T, class U, class D>
  void FillAssnsCollectionMap(std::vector<art::InputTag> const&,
			      CollectionMap< art::Assns<T,U,D> > &,
			      std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> > const&,
			      std::map< std::pair<art::ProductID,size_t> , art::Ptr<U> > const&);
};



mix::SimInfoOverlayFilter::SimInfoOverlayFilter(fhicl::ParameterSet const & p)
  :
  fSimInputFileNames(p.get<std::vector<std::string>>("SimInputFileNames")),
  gEvent(fSimInputFileNames),
  fVerbosity(p.get<int>("Verbosity",-1))
{
  FillInputModuleLabels(p);
  DeclareProduces();
}

void mix::SimInfoOverlayFilter::FillInputModuleLabels(fhicl::ParameterSet const & p,
						      std::string s,
						      std::vector<art::InputTag> & labels)
{
  labels = p.get< std::vector<art::InputTag> >(s,std::vector<art::InputTag>());
}

void mix::SimInfoOverlayFilter::FillInputModuleLabels(fhicl::ParameterSet const & p)
{
  FillInputModuleLabels(p,"MCTruthInputModuleLabels",fMCTruthInputModuleLabels);
  FillInputModuleLabels(p,"MCFluxInputModuleLabels",fMCFluxInputModuleLabels);
  FillInputModuleLabels(p,"GTruthInputModuleLabels",fGTruthInputModuleLabels);
  FillInputModuleLabels(p,"BeamGateInputModuleLabels",fBeamGateInputModuleLabels);

  FillInputModuleLabels(p,"MCParticleInputModuleLabels",fMCParticleInputModuleLabels);
  FillInputModuleLabels(p,"SimEnergyDepositInputModuleLabels",fSimEnergyDepositInputModuleLabels);
  FillInputModuleLabels(p,"AuxDetSimChannelInputModuleLabels",fAuxDetSimChannelInputModuleLabels);
  FillInputModuleLabels(p,"SimChannelInputModuleLabels",fSimChannelInputModuleLabels);
  FillInputModuleLabels(p,"SimPhotonsInputModuleLabels",fSimPhotonsInputModuleLabels);

  FillInputModuleLabels(p,"MCTruthMCFluxAssnsInputModuleLabels",fMCTruthMCFluxAssnsInputModuleLabels);
  FillInputModuleLabels(p,"MCTruthGTruthAssnsInputModuleLabels",fMCTruthGTruthAssnsInputModuleLabels);
  FillInputModuleLabels(p,"MCTruthMCParticleAssnsInputModuleLabels",fMCTruthMCParticleAssnsInputModuleLabels);
}

template<class T>
void mix::SimInfoOverlayFilter::DeclareProduces(std::vector<art::InputTag> const& labels)
{
  for(auto label : labels)
    this->produces<T>(label.instance());   
}

void mix::SimInfoOverlayFilter::DeclareProduces()
{
  DeclareProduces< std::vector<simb::MCTruth>     >(fMCTruthInputModuleLabels);
  DeclareProduces< std::vector<simb::MCFlux>      >(fMCFluxInputModuleLabels);
  DeclareProduces< std::vector<simb::GTruth>      >(fGTruthInputModuleLabels);
  DeclareProduces< std::vector<sim::BeamGateInfo> >(fBeamGateInputModuleLabels);

  DeclareProduces< std::vector<simb::MCParticle>      >(fMCParticleInputModuleLabels);
  DeclareProduces< std::vector<sim::SimEnergyDeposit> >(fSimEnergyDepositInputModuleLabels);
  DeclareProduces< std::vector<sim::AuxDetSimChannel> >(fAuxDetSimChannelInputModuleLabels);
  DeclareProduces< std::vector<sim::SimChannel>       >(fSimChannelInputModuleLabels);
  DeclareProduces< std::vector<sim::SimPhotons>       >(fSimPhotonsInputModuleLabels);

  DeclareProduces< art::Assns<simb::MCTruth,simb::MCFlux,void> >
    (fMCTruthMCFluxAssnsInputModuleLabels);
  DeclareProduces< art::Assns<simb::MCTruth,simb::GTruth,void> >
    (fMCTruthGTruthAssnsInputModuleLabels);
  DeclareProduces< art::Assns<simb::MCTruth,simb::MCParticle,sim::GeneratedParticleInfo> >
    (fMCTruthMCParticleAssnsInputModuleLabels);
}

template<class T>
void mix::SimInfoOverlayFilter::InitializeCollectionMap(std::vector<art::InputTag> const& labels,
							CollectionMap<T> & colmap)
{
  for(auto const& label : labels)
    colmap[label.instance()] = std::make_unique<T>();
}

void mix::SimInfoOverlayFilter::InitializeCollectionMaps()
{
  InitializeCollectionMap(fMCTruthInputModuleLabels,fMCTruthMap);
  InitializeCollectionMap(fMCFluxInputModuleLabels,fMCFluxMap);
  InitializeCollectionMap(fGTruthInputModuleLabels,fGTruthMap);
  InitializeCollectionMap(fBeamGateInputModuleLabels,fBeamGateInfoMap);

  InitializeCollectionMap(fMCTruthMCFluxAssnsInputModuleLabels,fMCTruthMCFluxAssnsMap);
  InitializeCollectionMap(fMCTruthGTruthAssnsInputModuleLabels,fMCTruthGTruthAssnsMap);

  InitializeCollectionMap(fMCParticleInputModuleLabels,fMCParticleMap);
  InitializeCollectionMap(fSimEnergyDepositInputModuleLabels,fSimEnergyDepositMap);
  InitializeCollectionMap(fAuxDetSimChannelInputModuleLabels,fAuxDetSimChannelMap);
  InitializeCollectionMap(fSimChannelInputModuleLabels,fSimChannelMap);
  InitializeCollectionMap(fSimPhotonsInputModuleLabels,fSimPhotonsMap);

  InitializeCollectionMap(fMCTruthMCParticleAssnsInputModuleLabels,fMCTruthMCParticleAssnsMap);

  if(fVerbosity>1)
    std::cout << "Finished initializing collection maps." << std::endl;
}

template<class T>
void mix::SimInfoOverlayFilter::PutCollectionOntoEvent(art::Event & e,
						       CollectionMap<T> & colmap)
{
  for(auto & cp : colmap)
    e.put(std::move(cp.second),cp.first);
}

void mix::SimInfoOverlayFilter::PutCollectionsOntoEvent(art::Event & e)
{
  PutCollectionOntoEvent(e,fMCTruthMap);  
  PutCollectionOntoEvent(e,fMCFluxMap);
  PutCollectionOntoEvent(e,fGTruthMap);
  PutCollectionOntoEvent(e,fBeamGateInfoMap);

  PutCollectionOntoEvent(e,fMCTruthMCFluxAssnsMap);
  PutCollectionOntoEvent(e,fMCTruthGTruthAssnsMap);

  PutCollectionOntoEvent(e,fMCParticleMap);  
  PutCollectionOntoEvent(e,fSimEnergyDepositMap);  
  PutCollectionOntoEvent(e,fAuxDetSimChannelMap);  
  PutCollectionOntoEvent(e,fSimChannelMap);  
  PutCollectionOntoEvent(e,fSimPhotonsMap);  

  PutCollectionOntoEvent(e,fMCTruthMCParticleAssnsMap);

  if(fVerbosity>1)
    std::cout << "Finished putting collections onto the event." << std::endl;
}

template<class T>
std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> >
mix::SimInfoOverlayFilter::FillCollectionMap(std::vector<art::InputTag> const& labels,
					     CollectionMap< std::vector<T> > & colmap,
					     std::string type_name,
					     art::Event & e)
{
  std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> >
    orig_artptr_lookup;

  for(auto label : labels){
    auto & out_ptrvec = colmap[label.instance()];
    art::PtrMaker<T> makeArtPtr(e, label.instance());

    gallery::Handle< std::vector<T> > handle;
    if(!gEvent.getByLabel< std::vector<T> >(label,handle)) continue;

    //in the future, this will be the way we do it
    //auto product_id = handle.id();

    //but, that's only available in gallery v1_10_00 and up.
    //For now, we gotta do it ourselves!
    if(label.process().size()==0)
      throw cet::exception("SimInfoOverlayFilter")
	<< "ERROR! All InputTags must be FULLY specified until gallery v1_10_00 is available."
	<< " InputTag: " << label << " not fully specified!";
    auto canonical_product_name =  art::canonicalProductName(art::friendlyname::friendlyName(type_name),
							     label.label(),
							     label.instance(),
							     label.process());
    auto product_id = art::ProductID(canonical_product_name);

    if(fVerbosity>2)
      std::cout << "Reading gallery handle. Canonical name = " << canonical_product_name << std::endl;

    auto const& vec(*handle);
    for(size_t i_obj=0; i_obj<vec.size(); ++i_obj){
      out_ptrvec->emplace_back(vec[i_obj]);
      orig_artptr_lookup[std::make_pair(product_id,i_obj)] = makeArtPtr(out_ptrvec->size()-1);
    }
  }

  if(fVerbosity>2)
    std::cout << "\tArtPtr lookup map has " << orig_artptr_lookup.size() << " entries." << std::endl;

  return orig_artptr_lookup;

}

template< class T, class U >
void mix::SimInfoOverlayFilter::FillAssnsCollectionMap(std::vector<art::InputTag> const& labels,
						       CollectionMap< art::Assns<T,U,void> > & colmap,
						       std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> > const&
						       orig_artptr_lookup_left,
						       std::map< std::pair<art::ProductID,size_t> , art::Ptr<U> > const&
						       orig_artptr_lookup_right)						       
{

  for(auto label : labels){
    auto & out_ptrAssns = colmap[label.instance()];
    

    gallery::Handle< art::Assns<T,U,void> > handle;
    if(!gEvent.getByLabel< art::Assns<T,U,void> >(label,handle)) continue;

    auto const& assns(*handle);
    for(size_t i_assn=0; i_assn<assns.size(); ++i_assn){
      auto id_left  = std::make_pair(assns[i_assn].first.id(),assns[i_assn].first.key());
      auto id_right = std::make_pair(assns[i_assn].second.id(),assns[i_assn].second.key());

      auto newptr_left = orig_artptr_lookup_left.at(id_left);
      auto newptr_right = orig_artptr_lookup_right.at(id_right);

      out_ptrAssns->addSingle(newptr_left,newptr_right);
    }
  }

}

template< class T, class U, class D>
void mix::SimInfoOverlayFilter::FillAssnsCollectionMap(std::vector<art::InputTag> const& labels,
						       CollectionMap< art::Assns<T,U,D> > & colmap,
						       std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> > const&
						       orig_artptr_lookup_left,
						       std::map< std::pair<art::ProductID,size_t> , art::Ptr<U> > const&
						       orig_artptr_lookup_right)						       
{

  for(auto label : labels){
    auto & out_ptrAssns = colmap[label.instance()];
    

    gallery::Handle< art::Assns<T,U,D> > handle;
    if(!gEvent.getByLabel< art::Assns<T,U,D> >(label,handle)) continue;

    auto const& assns(*handle);
    for(size_t i_assn=0; i_assn<assns.size(); ++i_assn){
      auto id_left  = std::make_pair(assns[i_assn].first.id(),assns[i_assn].first.key());
      auto id_right = std::make_pair(assns[i_assn].second.id(),assns[i_assn].second.key());
      auto const& data = assns.data(i_assn);

      auto newptr_left = orig_artptr_lookup_left.at(id_left);
      auto newptr_right = orig_artptr_lookup_right.at(id_right);

      out_ptrAssns->addSingle(newptr_left,newptr_right,data);
    }

  }

}

bool mix::SimInfoOverlayFilter::filter(art::Event & e)
{
  InitializeCollectionMaps();

  //check if we have exhausted our simulation events. If so, we return false.
  if(gEvent.atEnd()) {
    if(fVerbosity>0) 
      std::cout << "We've reached the end of the simulation event stream. Returning false." << std::endl;

    PutCollectionsOntoEvent(e);
    return false;
  }

  if(fVerbosity>1){
    std::cout << "Processing input event: "
	      << "Run " << e.run() << ", "
	      << "Event " << e.event() << std::endl;
    std::cout << "Processing simulation event: "
	      << "Run " << gEvent.eventAuxiliary().run() << ", "
	      << "Event " << gEvent.eventAuxiliary().event() << std::endl;      
  }

  auto mctruth_artptr_lookup = FillCollectionMap<simb::MCTruth>(fMCTruthInputModuleLabels,
								fMCTruthMap,
								"std::vector<simb::MCTruth>",
								e);
  auto mcflux_artptr_lookup = FillCollectionMap<simb::MCFlux>(fMCFluxInputModuleLabels,
							      fMCFluxMap,
							      "std::vector<simb::MCFlux>",
							      e);
  auto gtruth_artptr_lookup = FillCollectionMap<simb::GTruth>(fGTruthInputModuleLabels,
							      fGTruthMap,
							      "std::vector<simb::GTruth>",
							      e);
  FillCollectionMap<sim::BeamGateInfo>(fBeamGateInputModuleLabels,
				       fBeamGateInfoMap,
				       "std::vector<sim::BeamGateInfo>",
				       e);
  
  FillAssnsCollectionMap<simb::MCTruth,simb::MCFlux>(fMCTruthMCFluxAssnsInputModuleLabels,
						     fMCTruthMCFluxAssnsMap,
						     mctruth_artptr_lookup,
						     mcflux_artptr_lookup);

  FillAssnsCollectionMap<simb::MCTruth,simb::GTruth>(fMCTruthGTruthAssnsInputModuleLabels,
						     fMCTruthGTruthAssnsMap,
						     mctruth_artptr_lookup,
						     gtruth_artptr_lookup);

  //put onto event and loop the gallery event
  PutCollectionsOntoEvent(e);
  gEvent.next();
  return true;
}

void mix::SimInfoOverlayFilter::beginJob()
{
}

bool mix::SimInfoOverlayFilter::beginRun(art::Run & r)
{
  return true;
}

bool mix::SimInfoOverlayFilter::beginSubRun(art::SubRun & sr)
{
  return true;
}

void mix::SimInfoOverlayFilter::endJob()
{
}

bool mix::SimInfoOverlayFilter::endRun(art::Run & r)
{
  return true;
}

bool mix::SimInfoOverlayFilter::endSubRun(art::SubRun & sr)
{
  return true;
}

void mix::SimInfoOverlayFilter::respondToCloseInputFile(art::FileBlock const & fb)
{
}

void mix::SimInfoOverlayFilter::respondToCloseOutputFiles(art::FileBlock const & fb)
{
}

void mix::SimInfoOverlayFilter::respondToOpenInputFile(art::FileBlock const  &fb)
{
}

void mix::SimInfoOverlayFilter::respondToOpenOutputFiles(art::FileBlock const & fb)
{
}

DEFINE_ART_MODULE(mix::SimInfoOverlayFilter)