cet_enable_asserts()

art_make( BASENAME_ONLY
          LIB_LIBRARIES
                         fhiclcpp::fhiclcpp
          TOOL_LIBRARIES SignalShapingServiceMicroBooNE_service
                         lardata_Utilities_LArFFT_service
                         larcorealg_Geometry
                         larevt_CalibrationDBI_IOVData
                         larevt_CalibrationDBI_Providers
                         lardataobj_RecoBase
                         larcore_Geometry_Geometry_service
                         lardata_Utilities
                         nurandom_RandomUtils_NuRandomService_service
                         ${ART_FRAMEWORK_CORE}
                         ${ART_FRAMEWORK_PRINCIPAL}
                         ${ART_FRAMEWORK_SERVICES_REGISTRY}
                         art_Persistency_Common
                         art_Persistency_Provenance
                         art_root_io::TFileService_service
                         ${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
                         art_Utilities
                         canvas
                         ${MF_MESSAGELOGGER}
                         ${CETLIB}
                         ${Boost_FILESYSTEM_LIBRARY}
                         ${CLHEP}
    )

install_headers()
install_source()
install_fhicl()
