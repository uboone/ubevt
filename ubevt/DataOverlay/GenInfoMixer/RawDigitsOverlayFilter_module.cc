////////////////////////////////////////////////////////////////////////
// Class:       RawDigitsOverlayFilter
// Plugin Type: filter (art v2_11_03)
// File:        RawDigitsOverlayFilter_module.cc
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

#include "IFDH_service.h"

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

//raw products
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RawData/DAQHeader.h"


namespace mix {
  class RawDigitsOverlayFilter;
}


class mix::RawDigitsOverlayFilter : public art::EDFilter {
public:
  explicit RawDigitsOverlayFilter(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.
  ~RawDigitsOverlayFilter();
  
  // Plugins should not be copied or assigned.
  RawDigitsOverlayFilter(RawDigitsOverlayFilter const &) = delete;
  RawDigitsOverlayFilter(RawDigitsOverlayFilter &&) = delete;
  RawDigitsOverlayFilter & operator = (RawDigitsOverlayFilter const &) = delete;
  RawDigitsOverlayFilter & operator = (RawDigitsOverlayFilter &&) = delete;
  
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
  gallery::Event* gEvent;
  
  //input tag lists...
  //note, if these are empty, that's ok: then we won't produce them on the event
  
  std::vector<art::InputTag>    fRawDigitsInputModuleLabels;
  
  template<class T>
  using CollectionMap = std::unordered_map< std::string, std::unique_ptr<T> >;
  
  CollectionMap< std::vector<raw::RawDigit> >     fRawDigitsMap;
  
  //verbose
  int fVerbosity;

  //pot accounting

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

  std::map<int, std::map<int, std::map<int, long long>>> fRunSubrunEvtMap; 
  std::map<int, std::map<int, std::string>> fRunSubrunMap;
  std::string fCurrentFile;
  
  std::string fParentDefinition;
};



mix::RawDigitsOverlayFilter::RawDigitsOverlayFilter(fhicl::ParameterSet const & p)
  :
    gEvent(0),
  fVerbosity(p.get<int>("Verbosity",-1)),
  fCurrentFile(""),
  fParentDefinition(p.get<std::string>("ParentDefinition"))
{
  FillInputModuleLabels(p);
  DeclareProduces();
}

mix::RawDigitsOverlayFilter::~RawDigitsOverlayFilter() {
  if(gEvent) delete gEvent;
  gEvent = 0;
}


void mix::RawDigitsOverlayFilter::FillInputModuleLabels(fhicl::ParameterSet const & p,
						      std::string s,
						      std::vector<art::InputTag> & labels)
{
  labels = p.get< std::vector<art::InputTag> >(s,std::vector<art::InputTag>());
}

void mix::RawDigitsOverlayFilter::FillInputModuleLabels(fhicl::ParameterSet const & p)
{
  FillInputModuleLabels(p,"RawDigitsInputModuleLabels",fRawDigitsInputModuleLabels);
}

template<class T>
void mix::RawDigitsOverlayFilter::DeclareProduces(std::vector<art::InputTag> const& labels)
{
  for(auto label : labels)
    this->produces<T>(label.instance());   
}

void mix::RawDigitsOverlayFilter::DeclareProduces()
{
  DeclareProduces< std::vector<raw::RawDigit>     >(fRawDigitsInputModuleLabels);
}

template<class T>
void mix::RawDigitsOverlayFilter::InitializeCollectionMap(std::vector<art::InputTag> const& labels,
							CollectionMap<T> & colmap)
{
  for(auto const& label : labels)
    colmap[label.instance()] = std::make_unique<T>();
}

void mix::RawDigitsOverlayFilter::InitializeCollectionMaps()
{
  InitializeCollectionMap(fRawDigitsInputModuleLabels,fRawDigitsMap);

  if(fVerbosity>1)
    std::cout << "Finished initializing collection maps." << std::endl;
}

template<class T>
void mix::RawDigitsOverlayFilter::PutCollectionOntoEvent(art::Event & e,
						       CollectionMap<T> & colmap)
{
  for(auto & cp : colmap)
    e.put(std::move(cp.second),cp.first);
}

void mix::RawDigitsOverlayFilter::PutCollectionsOntoEvent(art::Event & e)
{
  PutCollectionOntoEvent(e,fRawDigitsMap);  

  if(fVerbosity>1)
    std::cout << "Finished putting collections onto the event." << std::endl;
}

template<class T>
std::map< std::pair<art::ProductID,size_t> , art::Ptr<T> >
mix::RawDigitsOverlayFilter::FillCollectionMap(std::vector<art::InputTag> const& labels,
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
    if(!gEvent->getByLabel< std::vector<T> >(label,handle)) {
      if(fVerbosity>3)
        std::cout << "No product with label "<<label<<" found"<<std::endl;
      continue;
    }

    //in the future, this will be the way we do it
    auto product_id = handle.id();

    //but, that's only available in gallery v1_10_00 and up.
    //For now, we gotta do it ourselves!
    
    /*
    if(label.process().size()==0)
      throw cet::exception("RawDigitsOverlayFilter")
	<< "ERROR! All InputTags must be FULLY specified until gallery v1_10_00 is available."
	<< " InputTag: " << label << " not fully specified!";
    auto canonical_product_name =  art::canonicalProductName(art::friendlyname::friendlyName(type_name),
							     label.label(),
							     label.instance(),
							     label.process());
    auto product_id = art::ProductID(canonical_product_name);

    if(fVerbosity>2)
      std::cout << "Reading gallery handle. Canonical name = " << canonical_product_name << std::endl;
    */

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


bool mix::RawDigitsOverlayFilter::filter(art::Event & e)
{
  InitializeCollectionMaps();

  int r = e.run();
  int s = e.subRun();
  int ev = e.event();

  if(fRunSubrunEvtMap.find(r) == fRunSubrunEvtMap.end()
      || fRunSubrunEvtMap[r].find(s) == fRunSubrunEvtMap[r].end()
      || fRunSubrunEvtMap[r][s].find(ev) == fRunSubrunEvtMap[r][s].end()) {
    // throws an exception
    std::cerr << " did not find RSE "<<r<<","<<s<<","<<ev<<" . EXITING WITH ERROR!"<<std::endl;
    gEvent->goToEntry(-1);
  }
  
  gEvent->goToEntry(fRunSubrunEvtMap[r][s][ev]);

  if(fVerbosity>1){
    std::cout << "Processing input event: "
	      << "Run " << e.run() << ", "
	      << "Event " << e.event() << std::endl;
    std::cout << "Processing simulation event: "
	      << "Run " << gEvent->eventAuxiliary().run() << ", "
	      << "Event " << gEvent->eventAuxiliary().event() << std::endl;      
  }


  auto mctruth_artptr_lookup = FillCollectionMap<raw::RawDigit>(fRawDigitsInputModuleLabels,
								fRawDigitsMap,
								"std::vector<raw::RawDigit>",
								e);

  //put onto event and loop the gallery event
  PutCollectionsOntoEvent(e);
  return true;
}

void mix::RawDigitsOverlayFilter::beginJob()
{
}

bool mix::RawDigitsOverlayFilter::beginRun(art::Run & r)
{
  return true;
}

bool mix::RawDigitsOverlayFilter::beginSubRun(art::SubRun & sr)
{
  int r = sr.run();
  int s = sr.subRun();
  if(fRunSubrunMap.find(r) == fRunSubrunMap.end() || fRunSubrunMap[r].find(s) == fRunSubrunMap[r].end()) {
    art::ServiceHandle<ifdh_ns::IFDH> ifdh;
    std::ostringstream dim;
    dim << "defname: " << fParentDefinition
	<< " and run_number " << r << "." << s;
    const std::vector<std::string> parents = ifdh->translateConstraints(dim.str());
    std::cerr << " found "<<parents.size() << " parents:"<<std::endl;
    for(auto& p : parents) std::cerr << "parent '" << p <<"'"<< std::endl;
    if(parents.size() != 1) {
      std::cerr << " did not find unique parent. EXITING WITH ERROR!"<<std::endl;
      throw 1;
    }
    const std::vector<std::string> paths = ifdh->locateFile(parents.front());
    std::cerr << " found "<<paths.size() <<" paths"<<std::endl;
    for(auto& path: paths)
      std::cerr <<"------   path = "<< path << std::endl;
    if(paths.size() != 1) {
      std::cerr << " did not find unique path. EXITING WITH ERROR!"<<std::endl;
      throw 1;
    }
    std::string path = paths[0];
    path.erase(path.find_last_of("("));
    path.replace(0,13,"root://fndca1.fnal.gov:1094/pnfs/fnal.gov/usr");
    path += "/" + parents.front();
    std::cerr << " new path: "<<path<<std::endl;
    TFile *f = TFile::Open(path.c_str());

    TTree *st = (TTree*)f->Get("SubRuns");
    if(st->GetEntries() < 1) {
      delete f;
      return true;
    }
    st->SetBranchStatus("*",0);
    st->SetBranchStatus("SubRunAuxiliary",1);
    art::SubRunAuxiliary* sra = 0;
    st->SetBranchAddress("SubRunAuxiliary",&sra);
    for(long long entry = 0; entry < st->GetEntries(); ++entry) {
      st->GetEntry(entry);
      int rr = sra->run();
      int ss = sra->subRun();
      fRunSubrunMap[rr][ss] = path;
    }
    
    TTree *t = (TTree*)f->Get("Events");
    if(t->GetEntries() < 1) {
      delete f;
      return true;
    }
    t->SetBranchStatus("*",0);
    t->SetBranchStatus("raw::DAQHeader_daq__Swizzler.*",1);
    art::Wrapper<raw::DAQHeader> *dqhd = 0;
    t->SetBranchAddress("raw::DAQHeader_daq__Swizzler.",&dqhd);
    for(long long entry =0; entry < t->GetEntries(); ++entry) {
      t->GetEntry(entry);
      int rr = dqhd->product()->GetRun();
      int ss = dqhd->product()->GetSubRun();
      int ev = dqhd->product()->GetEvent();
      //  dqhd->product()->GetEvent() is a 16-bit integer
      while(ev < ss*50) ev += 65536;
      fRunSubrunEvtMap[rr][ss][ev] = entry;
    }
    delete f;
  }
  if(fRunSubrunMap[r][s] != fCurrentFile) {
    if(gEvent) {
      delete gEvent;
      gEvent = 0; // in case new gallery::Event throws exception, we would have a double delete in the destructor
    }
    gEvent = new gallery::Event({fRunSubrunMap[r][s]});
    fCurrentFile = fRunSubrunMap[r][s];
  }
  return true;
}

void mix::RawDigitsOverlayFilter::endJob()
{
}

bool mix::RawDigitsOverlayFilter::endRun(art::Run & r)
{
  return true;
}

bool mix::RawDigitsOverlayFilter::endSubRun(art::SubRun & sr)
{
  return true;
}

void mix::RawDigitsOverlayFilter::respondToCloseInputFile(art::FileBlock const & fb)
{
}

void mix::RawDigitsOverlayFilter::respondToCloseOutputFiles(art::FileBlock const & fb)
{
}

void mix::RawDigitsOverlayFilter::respondToOpenInputFile(art::FileBlock const  &fb)
{
}

void mix::RawDigitsOverlayFilter::respondToOpenOutputFiles(art::FileBlock const & fb)
{
}

DEFINE_ART_MODULE(mix::RawDigitsOverlayFilter)
