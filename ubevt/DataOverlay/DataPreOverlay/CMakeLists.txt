cet_build_plugin(
  CRTpreOverlay art::EDProducer
  LIBRARIES
  PRIVATE
  ubobj::CRT
)

cet_build_plugin(
  ZeroingOutBadChannelsAna art::EDAnalyzer
  LIBRARIES
  PRIVATE
  lardataobj::RawData
  art_root_io::TFileService_service
  ROOT::Tree
)

cet_build_plugin(
  ZeroingOutBadChannels art::EDProducer
  LIBRARIES
  PRIVATE
  lardataobj::RawData
)

install_headers()
install_fhicl()
install_source()

