cet_build_plugin(
  SignalShapingMicroBooNETest art::EDAnalyzer
  LIBRARIES
  PRIVATE
  ubevt::Utilities_SignalShapingServiceMicroBooNE_service
  lardata::Utilities
  art_root_io::TFileService_service
)

install_headers()
install_fhicl()
install_source()


