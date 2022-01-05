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
                         nurandom::RandomUtils_NuRandomService_service
                         art::Framework_Core
                         art::Framework_Principal
                         art::Framework_Services_Registry
                         art::Persistency_Common
                         art::Persistency_Provenance
                         art_root_io::TFileService_service
                         art::Utilities
                         canvas::canvas
                         messagefacility::MF_MessageLogger
                         cetlib::cetlib
                         Boost::filesystem
                         CLHEP::CLHEP
    )

install_headers()
install_source()
install_fhicl()
