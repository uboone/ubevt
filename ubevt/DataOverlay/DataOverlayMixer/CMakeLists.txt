cet_build_plugin(
  OverlayRawDataMicroBooNE art::EDProducer
  LIBRARIES
  PRIVATE
  ubevt::Utilities
  ubevt::DataOverlay_DataOverlay
  larevt::CalibrationDBI_IOVData
  larevt::ChannelStatusService
  larcore::Geometry_Geometry_service
  lardataobj::RawData
)

cet_build_plugin(
  RawDigitOverlayMicroBooNEAna art::EDAnalyzer
  LIBRARIES
  PRIVATE
  ubevt::DataOverlay_DataOverlay
  lardataobj::RawData
  art_root_io::TFileService_service
  ROOT::Hist
)

cet_build_plugin(
  RetrieveOverlayDataMicroBooNE art::module
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  lardataobj::Simulation
  lardataobj::MCBase
  nusimdata::SimulationBase
  ifdh_art::IFDH_service
  art::Framework_IO_ProductMix
  art_root_io::art_root_io
)

install_headers()
install_source()
install_fhicl()
