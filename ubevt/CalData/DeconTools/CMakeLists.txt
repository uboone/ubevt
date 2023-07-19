cet_make_library(
  SOURCE
  WaveformPropertiesAlg.cxx
  LIBRARIES
  PUBLIC
  lardata::Utilities
  fhiclcpp::fhiclcpp
)

install_headers()
install_source()
install_fhicl()
