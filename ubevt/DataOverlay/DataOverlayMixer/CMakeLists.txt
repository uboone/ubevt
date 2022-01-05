include_directories( $ENV{IFDHC_FQ_DIR}/inc )

simple_plugin(RetrieveOverlayDataMicroBooNE "module"
              LIBRARIES
                lardataobj_RawData
                nusimdata::SimulationBase
                lardataobj_MCBase
                art::Framework_Core
                art::Framework_IO_ProductMix
                art_root_io::art_root_io
                art::Framework_Principal
                art::Framework_Services_Registry
                art::Persistency_Common
                art::Persistency_Provenance
                art_root_io_detail
                art::Utilities canvas::canvas
                ifdh::ifdh
                messagefacility::MF_MessageLogger
                fhiclcpp::fhiclcpp
                cetlib::cetlib
)

simple_plugin(OverlayRawDataMicroBooNE "module"
              LIBRARIES
                ub_RawDigitMixer
                ub_OpDetWaveformMixer
		ub_CRTMixer
                lardataobj_RawData
                nusimdata::SimulationBase
                lardataobj_MCBase
                larevt_CalibrationDBI_IOVData
                larevt_CalibrationDBI_Providers
                art::Framework_Core
                art::Framework_IO_ProductMix
                art_root_io::art_root_io
                art::Framework_Principal
                art::Framework_Services_Registry
                ubevt_Utilities
                art::Persistency_Common
                art::Persistency_Provenance
                art::Utilities
                art_root_io_detail
                canvas::canvas
                ifdh::ifdh
                messagefacility::MF_MessageLogger
                fhiclcpp::fhiclcpp
                cetlib::cetlib
)

simple_plugin(RawDigitOverlayMicroBooNEAna "module"
              LIBRARIES
                ub_RawDigitAdderAna
                lardataobj_RawData
                art::Framework_Core
                art_root_io::art_root_io
                art::Framework_Principal
                art::Framework_Services_Registry
                art_root_io::TFileService_service
                art::Persistency_Provenance
                art_root_io_detail
                fhiclcpp::fhiclcpp
                cetlib::cetlib
                ROOT::Hist
)

install_headers()
install_source()
install_fhicl()
