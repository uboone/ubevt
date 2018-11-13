////////////////////////////////////////////////////////////////////////
// Class:       SCECorrReco
// Plugin Type: producer (art v2_11_02)
// File:        SCECorrReco_module.cc
//
// Generated at Thu Aug 23 18:00:55 2018 by Hannah Rogers using cetskelgen
// from cetlib version v3_03_01.
////////////////////////////////////////////////////////////////////////

//Framework includes
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "art/Framework/Services/Optional/TFileService.h" 
#include "art/Framework/Services/Optional/TFileDirectory.h"

// LArSoft includes
#include "lardataobj/RecoBase/Track.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "lardataalg/DetectorInfo/DetectorProperties.h"
//#include "larevt/SpaceCharge/SpaceCharge.h"

//ubevt includes
#include "ubevt/SpaceCharge/SpaceChargeMicroBooNE.h"
#include "ubevt/SpaceChargeServices/SpaceChargeServiceMicroBooNE.h"

// ROOT
#include "TH2.h"

// c++
#include <string>
#include <vector>

namespace spacecharge {
  class SCECorrReco;
}


class spacecharge::SCECorrReco : public art::EDProducer {
public:
  explicit SCECorrReco(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  SCECorrReco(SCECorrReco const &) = delete;
  SCECorrReco(SCECorrReco &&) = delete;
  SCECorrReco & operator = (SCECorrReco const &) = delete;
  SCECorrReco & operator = (SCECorrReco &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;
  void MakeDataProducts();

private:

	TH2F* hEmag;

};


spacecharge::SCECorrReco::SCECorrReco(fhicl::ParameterSet const & p)
 : hEmag(nullptr)
// Initialize member data here.
{
  produces< std::vector<recob::Track> >();
}

void spacecharge::SCECorrReco::produce(art::Event & e)
{
  auto const& track_handle = e.getValidHandle<std::vector<recob::Track>>("pandora");

  //auto const* sce = lar::providerFrom<spacecharge::SpaceChargeService>();
  //wes, 13Nov2018: we need to hack this as we use functions not available in the base class, but providerFrom only gives
  // us the base class part...
  spacecharge::SpaceChargeMicroBooNE const* sce = 
    reinterpret_cast<spacecharge::SpaceChargeMicroBooNE const*>(lar::providerFrom<spacecharge::SpaceChargeService>());


  std::unique_ptr< std::vector<recob::Track> > SCECorrPtr( new std::vector<recob::Track>() );
  auto & SCECorrTrack(*SCECorrPtr);
  
  if (sce->EnableCalSpatialSCE()){  
    for (auto const& track_obj : *track_handle) {
    
      std::vector<geo::Point_t> SCE_points;
      std::vector<geo::Vector_t> SCE_momenta;
      std::vector<recob::Track::PointFlags_t> SCE_flags;
      
      for (size_t ii = 0; ii < track_obj.NumberTrajectoryPoints(); ii++) {
        
        recob::tracking::TrajectoryPoint_t fTrack = track_obj.TrajectoryPoint(ii);
        geo::Point_t fTrackPos = fTrack.position;
        geo::Vector_t fPosOffsets = sce->GetCalPosOffsets(geo::Point_t{fTrackPos.X(),fTrackPos.Y(),fTrackPos.Z()});
        geo::Point_t fSCEPos = geo::Point_t{fTrackPos.X() - fPosOffsets.X(), fTrackPos.Y() + fPosOffsets.Y(), fTrackPos.Z() + fPosOffsets.Z()};
        
        SCE_points.push_back(fSCEPos);
        SCE_momenta.push_back(fTrack.momentum);
        SCE_flags.push_back(track_obj.FlagsAtPoint(ii));
              
        std::cout << "Position offsets in calibration: (" << fTrackPos.X() << ", " << fTrackPos.Y() << ", " << fTrackPos.Z() << ") -> (" << fSCEPos.X() << ", " << fSCEPos.Y() << ", " << fSCEPos.Z() << ")" << std::endl;
      
      
        // geo::Point_t fSCEPos -> recob::tracking::trajectoryPoint_t -> recob:Track
        
      } 
     
      auto fCovariances = track_obj.Covariances();
     
      recob::Track* fSCETrackPtr = new recob::Track(std::move(SCE_points), std::move(SCE_momenta), std::move(SCE_flags), track_obj.HasMomentum(), track_obj.ParticleId(),track_obj.Chi2(),track_obj.Ndof(),std::move(fCovariances.first),std::move(fCovariances.second),track_obj.ID());
       auto & fSCETrack(*fSCETrackPtr);
      SCECorrTrack.push_back(fSCETrack);
        
    }
  } 
  
  if (sce->EnableCalEfieldSCE()){
    art::ServiceHandle<art::TFileService> tfs;
    hEmag = tfs->make<TH2F>("hEmag","Electric field magnitude", 207, 0.0, 1035.0, 46, -115.0, 115.0);
    
    auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
    double efield = detprop->Efield();  
    
    Double_t x1 = 100.0;
    for( int i = 0; i < 206; i++ ){
  	  Double_t z = 5.0*(Double_t)i + 2.5;
  	  for( int j = 0; j < 45; j++ ){
  		  Double_t y = 5.0*(Double_t)j - 115.0 + 2.5;
  		
  		  geo::Vector_t fEfieldOffsets = sce->GetCalEfieldOffsets(geo::Point_t{x1,y,z});
  		  double Ex = efield + 1.0*efield*fEfieldOffsets.X();
  		  double Ey = 1.0*efield*fEfieldOffsets.Y();
  		  double Ez = 1.0*efield*fEfieldOffsets.Z();
  		  double Emag = std::sqrt(Ex*Ex + Ey*Ey + Ez*Ez);
  		  std::cout << "Efield at (" << x1 << ", " << y << ", " << z << ") = (" << Ex << ", " << Ey << ", " << Ez << ") = "<< std::sqrt(Ex*Ex + Ey*Ey + Ez*Ez) << " kV/cm." << std::endl;
  		  hEmag->Fill(z,y,100.*(Emag-efield)/efield);
  	  }
    }	
  
    hEmag->GetXaxis()->SetTitle("z (cm) - LArSoft Coordinates");
    hEmag->GetYaxis()->SetTitle("y (cm) - LArSoft Coordinates");
    hEmag->SetTitle("Electric Field at x = 200 cm");
    
  
  
  }
    
  e.put(std::move(SCECorrPtr));

}

DEFINE_ART_MODULE(spacecharge::SCECorrReco)
