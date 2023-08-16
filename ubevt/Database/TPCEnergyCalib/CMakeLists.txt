cet_make_library(
  SOURCE
  UboonePlaneCorrectionProvider.cxx
  UbooneTPCEnergyCalibProvider.cxx
  UboonedqdxCorrectionProvider.cxx
  LIBRARIES
  PUBLIC
  larevt::CalibrationDBI_IOVData
  larevt::CalibrationDBI_Providers
  larcore::Geometry_Geometry_service
  messagefacility::MF_MessageLogger
  cetlib::cetlib
)

cet_build_plugin(
  UbooneTPCEnergyCalibService art::service
  LIBRARIES
  PRIVATE
  ubobj::DataOverlay
  ubevt::Database_TPCEnergyCalib
  art::Framework_Principal
)

install_headers()
install_fhicl()
install_source()
