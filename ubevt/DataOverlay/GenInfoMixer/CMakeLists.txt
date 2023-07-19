cet_build_plugin(
  G4InfoMixerLocal art::module
  LIBRARIES
  PRIVATE
  lardataobj::Simulation
  nusimdata::SimulationBase
  art::Framework_IO_ProductMix
  art_root_io::art_root_io
)

cet_build_plugin(
  GenInfoMixerLocal art::module
  LIBRARIES
  PRIVATE
  lardataobj::Simulation
  nusimdata::SimulationBase
  art::Framework_IO_ProductMix
  art_root_io::art_root_io
)

cet_build_plugin(
  SimInfoOverlayFilter art::EDFilter
  LIBRARIES
  PRIVATE
  larcoreobj::SummaryData
  lardataobj::Simulation
  nusimdata::SimulationBase
  gallery::gallery
)

cet_build_plugin(
  SubRunPOTInEvent art::EDProducer
  LIBRARIES
  PRIVATE
  larcoreobj::SummaryData
)

install_headers()
install_source()
install_fhicl()
