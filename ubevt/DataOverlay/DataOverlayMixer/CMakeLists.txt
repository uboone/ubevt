include_directories( $ENV{IFDHC_FQ_DIR}/inc )

simple_plugin(RetrieveOverlayDataMicroBooNE "module"
              LIBRARIES
                lardataobj_RawData
                nusimdata_SimulationBase
                lardataobj_MCBase
                ${ART_FRAMEWORK_CORE}
                ${ART_FRAMEWORK_IO_PRODUCTMIX}
                ${ART_ROOT_IO}
                ${ART_FRAMEWORK_PRINCIPAL}
                ${ART_FRAMEWORK_SERVICES_REGISTRY}
                ${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
                art_Persistency_Common
                art_Persistency_Provenance
                art_root_io_detail
                art_Utilities canvas
                ifdh
                ${MF_MESSAGELOGGER}
                ${FHICLCPP}
                cetlib::cetlib
)

simple_plugin(OverlayRawDataMicroBooNE "module"
              LIBRARIES
                ub_RawDigitMixer
                ub_OpDetWaveformMixer
		ub_CRTMixer
                lardataobj_RawData
                nusimdata_SimulationBase
                lardataobj_MCBase
                larevt_CalibrationDBI_IOVData
                larevt_CalibrationDBI_Providers
                ${ART_FRAMEWORK_CORE}
                ${ART_FRAMEWORK_IO_PRODUCTMIX}
                ${ART_ROOT_IO}
                ${ART_FRAMEWORK_PRINCIPAL}
                ${ART_FRAMEWORK_SERVICES_REGISTRY}
                ${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
                ubevt_Utilities
                art_Persistency_Common
                art_Persistency_Provenance
                art_Utilities
                art_root_io_detail
                canvas
                ifdh
                ${MF_MESSAGELOGGER}
                ${FHICLCPP}
                cetlib::cetlib
)

simple_plugin(RawDigitOverlayMicroBooNEAna "module"
              LIBRARIES
                ub_RawDigitAdderAna
                lardataobj_RawData
                ${ART_FRAMEWORK_CORE}
                ${ART_ROOT_IO}
                ${ART_FRAMEWORK_PRINCIPAL}
                ${ART_FRAMEWORK_SERVICES_REGISTRY}
                art_root_io::TFileService_service
                art_Persistency_Provenance
                art_root_io_detail
                ${FHICLCPP}
                cetlib::cetlib
                ROOT::Hist
)

install_headers()
install_source()
install_fhicl()
