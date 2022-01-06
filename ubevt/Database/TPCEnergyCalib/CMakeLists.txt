cet_find_library( LIBWDA NAMES wda PATHS ENV LIBWDA_LIB NO_DEFAULT_PATH )

include_directories( $ENV{LIBWDA_FQ_DIR}/include )

art_make( BASENAME_ONLY
          LIB_LIBRARIES
			larevt_CalibrationDBI_IOVData
			larevt_CalibrationDBI_Providers
			larcorealg_Geometry
			LIBWDA
                        messagefacility::MF_MessageLogger
	                fhiclcpp::fhiclcpp
			cetlib::cetlib
                        cetlib_except::cetlib_except
			art::Framework_Services_Registry
			art::Utilities
                        canvas::canvas
          MODULE_LIBRARIES larcorealg_Geometry
                        larcore_Geometry_Geometry_service
                        larsim_Simulation nutools_ParticleNavigation lardataobj_Simulation
                        lardata_Utilities
                        larevt_Filters
                        lardataobj_RawData
                        SignalShapingServiceMicroBooNE_service
			larevt_CalibrationDBI_Providers
			ubevt_Database
                        nurandom::RandomUtils_NuRandomService_service
                        art::Framework_Core
                        art::Framework_Principal
                        art::Framework_Services_Registry
                        art_root_io::tfile_support ROOT::Core
                        art::Framework_Services_Optional_RandomNumberGenerator_service
                        art_root_io::TFileService_service
                        art::Persistency_Common
                        art::Persistency_Provenance
                        art::Utilities
                        canvas::canvas
                        messagefacility::MF_MessageLogger

                        fhiclcpp::fhiclcpp
                        cetlib::cetlib
                        cetlib_except::cetlib_except
                        CLHEP::CLHEP
                        ROOT::Geom
                        ROOT::XMLIO
                        ROOT::Gdml

	  SERVICE_LIBRARIES
		      ubevt_Database
		      ubevt_Database_TPCEnergyCalib
	              larevt_CalibrationDBI_Providers
                      lardataobj_RawData
		      larcorealg_Geometry
	              larcore_Geometry_Geometry_service
                      art::Framework_Principal
                      art::Framework_Services_Registry
                      art::Persistency_Common
                      art::Persistency_Provenance
                      art_root_io::tfile_support ROOT::Core
                      art::Framework_Services_Optional_RandomNumberGenerator_service
                      art_root_io::TFileService_service
                      art::Utilities
                      canvas::canvas
                      fhiclcpp::fhiclcpp

)

install_headers()
install_fhicl()
install_source()
