cet_make_library(
  SOURCE
  PMTRemapProvider.cxx
  SignalShapingLite.cxx
  LIBRARIES
  PUBLIC
  lardata::Utilities
  art::Framework_Services_Registry
  fhiclcpp::fhiclcpp
  ROOT::MathCore
)

cet_build_plugin(
  OpHitRemapProducer art::EDProducer
  LIBRARIES
  PRIVATE
  ubevt::Utilities
  lardataobj::RecoBase
)

cet_build_plugin(
  RandomPrescale art::EDFilter
  LIBRARIES
  PRIVATE
  nurandom::RandomUtils_NuRandomService_service
)

cet_build_plugin(
  TinyHitFinderAna art::EDAnalyzer
  LIBRARIES
  PRIVATE
  lardata::Utilities
  larcore::Geometry_Geometry_service
  lardataobj::RecoBase
  art_root_io::TFileService_service
  ROOT::MathCore
)

cet_build_plugin(
  FileCatalogMetadataMicroBooNE art::service
  LIBRARIES
  PUBLIC
  larcoreobj::SummaryData
  art::Framework_Services_System_FileCatalogMetadata_service
  art::Framework_Principal
)

cet_build_plugin(
  PMTRemapService art::service
  LIBRARIES
  PUBLIC
  ubevt::Utilities
)

cet_build_plugin(
  SignalShapingServiceMicroBooNE art::service
  LIBRARIES
  PUBLIC
  ubevt::Utilities
  larevt::CalibrationDBI_Providers
  art_root_io::TFileService_service
)

cet_build_plugin(
  TFileMetadataMicroBooNE art::service
  LIBRARIES
  PUBLIC
  larcoreobj::SummaryData
  art_root_io::RootDB
  art::Framework_Principal
  art::Framework_Services_System_FileCatalogMetadata_service
  messagefacility::MF_MessageLogger
  ROOT::MathCore
)

install_headers()
# do not install duplicate standard_reco_uboone.fcl file
install_fhicl( EXCLUDES standard_reco_uboone.fcl )
install_source()

#add_subdirectory(test)
