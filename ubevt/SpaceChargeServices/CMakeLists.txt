include_directories( $ENV{GSL_INC} )
include(lar::SpaceChargeService)

cet_build_plugin(
  SpaceChargeServiceMicroBooNE lar::SpaceChargeService
  LIBRARIES
  PUBLIC
  ubevt::SpaceCharge
  larcore::ServiceUtil
  lardata::DetectorPropertiesService
  art::Framework_Principal
)

install_headers()
install_fhicl()
install_source()
