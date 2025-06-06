cet_make_library(
  SOURCE
  UbooneElectronLifetimeProvider.cxx
  UbooneElectronicsCalibProvider.cxx
  UbooneLightYieldProvider.cxx
  UboonePmtGainProvider.cxx
  WireCellChannelStatusProvider.cxx
  LIBRARIES
  PUBLIC
  larevt::CalibrationDBI_IOVData
  larevt::CalibrationDBI_Providers
  larcore::Geometry_Geometry_service
  larcorealg::CoreUtils
  fhiclcpp::fhiclcpp
  messagefacility::MF_MessageLogger
)

cet_build_plugin(
  TestDBI art::EDAnalyzer
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  larevt::CalibrationDBI_Providers
  larcore::Geometry_Geometry_service
  lardataobj::RawData
  art_root_io::TFileService_service
  ROOT::Gpad
)

cet_make_library(
  LIBRARY_NAME CRTChannelStatusService INTERFACE
  SOURCE CRTChannelStatusService.h
  LIBRARIES
  INTERFACE
  larcore::ServiceUtil
)

cet_write_plugin_builder(
  uboone::CRTChannelStatusService art::service Modules
  INSTALL_BUILDER
  LIBRARIES CONDITIONAL
  ubevt::CRTChannelStatusService
)

include(uboone::CRTChannelStatusService)

cet_build_plugin(
  UbooneCRTChannelStatusService uboone::CRTChannelStatusService
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  larevt::CalibrationDBI_Providers
  lardataobj::RawData
  larcorealg::CoreUtils
  art::Framework_Principal
  ROOT::Geom
  CLHEP::Geometry
)

include(lar::ChannelStatusService)

cet_build_plugin(
  UbooneChannelStatusService lar::ChannelStatusService
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  larevt::CalibrationDBI_Providers
  larcorealg::Geometry
  lardataobj::RawData
  art::Framework_Principal
  ROOT::Geom
  CLHEP::Geometry
)

include(lar::DetPedestalService)

cet_build_plugin(
  UbooneDetPedestalService lar::DetPedestalService
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  larevt::CalibrationDBI_Providers
  art::Framework_Principal
)

cet_build_plugin(
  UbooneElectronLifetimeService art::service
  LIBRARIES
  PUBLIC
  ubobj::DataOverlay
  ubevt::Database
  art::Framework_Principal
)

include(lar::ElectronicsCalibService)

cet_build_plugin(
  UbooneElectronicsCalibService lar::ElectronicsCalibService
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  ubevt::Database
  art::Framework_Principal
)

cet_make_library(
  LIBRARY_NAME LightYieldService INTERFACE
  SOURCE LightYieldService.h
  LIBRARIES
  INTERFACE
  larcore::ServiceUtil
)

cet_write_plugin_builder(
  uboone::LightYieldService art::service Modules
  INSTALL_BUILDER
  LIBRARIES CONDITIONAL
  ubevt::LightYieldService
)

include(uboone::LightYieldService)

cet_build_plugin(
  UbooneLightYieldService uboone::LightYieldService
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  ubevt::Database
  art::Framework_Principal
)

include(lar::PmtGainService)

cet_build_plugin(
  UboonePmtGainService lar::PmtGainService
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  ubevt::Database
  art::Framework_Principal
)

include(lar::ChannelStatusService)

cet_build_plugin(
  WireCellChannelStatusService lar::ChannelStatusService
  LIBRARIES
  PRIVATE
  ubevt::Database
  art::Framework_Principal
)

install_headers()
install_fhicl()
install_source()

add_subdirectory(TPCEnergyCalib)
