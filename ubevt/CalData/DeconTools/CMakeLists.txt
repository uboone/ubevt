cet_make_library(
  SOURCE
  WaveformPropertiesAlg.cxx
  LIBRARIES
  PUBLIC
  lardata::Utilities
  fhiclcpp::fhiclcpp
)

cet_make_library(LIBRARY_NAME IBaseline INTERFACE
  SOURCE IBaseline.h
  LIBRARIES INTERFACE
  larcoreobj::headers
)

cet_write_plugin_builder(
  uboone_tool::IBaseline art::tool Modules
  INSTALL_BUILDER
  LIBRARIES CONDITIONAL
  ubevt::IBaseline
  art_root_io::TFileService_service
  canvas::canvas
)

include(uboone_tool::IBaseline)

cet_build_plugin(
  NoBaseline uboone_tool::IBaseline
  LIBRARIES
  PRIVATE
)

cet_build_plugin(
  BaselineMostProbAve uboone_tool::IBaseline
  LIBRARIES
  PRIVATE
  ubevt::Utilities_SignalShapingServiceMicroBooNE_service
  lardata::Utilities
)

cet_build_plugin(
  BaselineStandard uboone_tool::IBaseline
  LIBRARIES
  PRIVATE
  ubevt::Utilities_SignalShapingServiceMicroBooNE_service
  lardata::Utilities
)

cet_build_plugin(
  BaselineWaveformProperties uboone_tool::IBaseline
  LIBRARIES
  PRIVATE
  lardata::Utilities
)

cet_make_library(LIBRARY_NAME IROIFinder INTERFACE
  SOURCE IROIFinder.h
  LIBRARIES INTERFACE
  larcoreobj::headers
)

cet_write_plugin_builder(
  uboone_tool::IROIFinder art::tool Modules
  INSTALL_BUILDER
  LIBRARIES CONDITIONAL
  ubevt::IROIFinder
  art_root_io::TFileService_service
  canvas::canvas
)

include(uboone_tool::IROIFinder)

cet_build_plugin(
  ROIFinderDifferential uboone_tool::IROIFinder
  LIBRARIES
  PRIVATE
  lardata::DetectorPropertiesService
  larcore::Geometry_Geometry_service
)

cet_build_plugin(
  ROIFinderMorphological uboone_tool::IROIFinder
  LIBRARIES
  PRIVATE
  lardata::DetectorPropertiesService
  larcore::Geometry_Geometry_service
)

cet_build_plugin(
  ROIFinderStandard uboone_tool::IROIFinder
  LIBRARIES
  PRIVATE
  ubevt::Utilities_SignalShapingServiceMicroBooNE_service
  lardata::Utilities_LArFFT_service
)

cet_make_library(LIBRARY_NAME IDeconvolution INTERFACE
  SOURCE IDeconvolution.h
  LIBRARIES INTERFACE
  ubevt::IROIFinder
  lardataobj::RecoBase
)

cet_write_plugin_builder(
  uboone_tool::IDeconvolution art::tool Modules
  INSTALL_BUILDER
  LIBRARIES CONDITIONAL
  ubevt::IDeconvolution
  art_root_io::TFileService_service
  canvas::canvas
)

include(uboone_tool::IDeconvolution)

cet_build_plugin(
  MCC7Deconvolution uboone_tool::IDeconvolution
  LIBRARIES
  PRIVATE
  ubevt::Utilities_SignalShapingServiceMicroBooNE_service
  lardata::DetectorPropertiesService
  larcore::Geometry_Geometry_service
)

cet_build_plugin(
  ROIDeconvolution uboone_tool::IDeconvolution
  LIBRARIES
  PRIVATE
  ubevt::Utilities_SignalShapingServiceMicroBooNE_service
  lardata::DetectorPropertiesService
  larcore::Geometry_Geometry_service
)

install_headers()
install_source()
install_fhicl()
