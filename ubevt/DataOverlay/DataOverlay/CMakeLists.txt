cet_make_library(
  SOURCE
  CRTMixer.cxx
  OpDetWaveformMixer.cxx
  RawDigitAdder.cxx
  RawDigitAdderAna.cxx
  RawDigitAdder_HardSaturate.cxx
  RawDigitMixer.cxx
  LIBRARIES
  PUBLIC
  lardataobj::RawData
)

install_headers()
install_source()
