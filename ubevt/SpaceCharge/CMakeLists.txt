cet_make_library(
  SOURCE
  SpaceChargeMicroBooNE.cxx
  LIBRARIES
  PUBLIC
  larevt::SpaceCharge
  lardataalg::DetectorInfo
  GSL::gsl
)

cet_build_plugin(
  SCECorrReco art::EDProducer
  LIBRARIES
  PRIVATE
  larevt::SpaceCharge
  lardata::DetectorInfoServices_DetectorPropertiesServiceStandard_service
  larcore::ServiceUtil
  lardataobj::RecoBase
  art_root_io::TFileService_service
  GSL::gsl
)

cet_build_plugin(
  ShiftEdepSCE art::EDProducer
  LIBRARIES
  PRIVATE
  larevt::SpaceCharge
  larcore::ServiceUtil
  lardataobj::Simulation
  art_root_io::TFileService_service
  ROOT::Tree
)

install_headers()
install_fhicl()
install_source()
