cet_make_library(
  SOURCE
  ROIAlg.cxx
  RawDigitAndWireComparisonAlg.cxx
  LIBRARIES
  PUBLIC
  lardata::Utilities
  fhiclcpp::fhiclcpp
)

cet_build_plugin(
  CalWireInvert art::EDProducer
  LIBRARIES
  PRIVATE
  larevt::Filters
  lardata::ArtDataHelper
  larcore::Geometry_Geometry_service
  lardataobj::RawData
  art_root_io::TFileService_service
  ROOT::MathCore
)

cet_build_plugin(
  CalWireMicroBooNE art::EDProducer
  LIBRARIES
  PRIVATE
  ubevt::Utilities
  ubevt::Utilities_SignalShapingServiceMicroBooNE_service
  larevt::DetPedestalService
  lardata::ArtDataHelper
  lardata::Utilities_LArFFT_service
  larcore::Geometry_Geometry_service
  lardataalg::DetectorInfo
  lardataobj::RawData
  art_root_io::TFileService_service
  ROOT::MathCore
  ROOT::FFTW
)

cet_build_plugin(
  CalWireROI art::EDProducer
  LIBRARIES
  PRIVATE
  ubobj::RawData
  larevt::DetPedestalProvider
  lardataalg::DetectorInfo
  lardataobj::RawData
  larcore::Geometry_Geometry_service
  lardata::ArtDataHelper
  lardata::Utilities_LArFFT_service
  art_root_io::TFileService_service
  ROOT::Hist
)

cet_build_plugin(
  DumpOpDetGeoMicroBooNE art::EDAnalyzer
  LIBRARIES
  PRIVATE
  larcore::Geometry_Geometry_service
)

cet_build_plugin(
  RawDigitAndWireComparison art::EDAnalyzer
  LIBRARIES
  PRIVATE
  ubevt::CalData
  lardata::Utilities
  lardataobj::RawData
  art_root_io::TFileService_service
  ROOT::Tree
)

cet_build_plugin(
  RawDigitFilterUBooNE art::EDProducer
  LIBRARIES
  PRIVATE
  ubevt::CalData_NoiseFilterAlgs
  larcore::Geometry_Geometry_service
  lardata::DetectorPropertiesService
  larevt::DetPedestalService
  art_root_io::TFileService_service
)

cet_build_plugin(
  ShowWire art::EDAnalyzer
  LIBRARIES
  PRIVATE
  lardataobj::RecoBase
  larevt::DetPedestalService
  art_root_io::TFileService_service
)

cet_build_plugin(
  TestCFI art::EDAnalyzer
  LIBRARIES
  PRIVATE
  larevt::ChannelStatusService
  larcore::Geometry_Geometry_service
)

add_subdirectory(NoiseFilterAlgs)
add_subdirectory(DeconTools)

install_headers()
install_fhicl()
install_source()
