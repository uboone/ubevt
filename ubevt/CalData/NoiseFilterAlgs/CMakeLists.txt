art_make( BASENAME_ONLY
          LIBRARY_NAME NoiseFilterAlgs
          LIB_LIBRARIES
			lardataobj_RawData
                        lardataobj_RecoBase
	                lardata_Utilities
	                fhiclcpp::fhiclcpp
			cetlib::cetlib
                        cetlib_except::cetlib_except

	                larcorealg_Geometry
	  		larcore_Geometry_Geometry_service
	                lardata_Utilities
			larevt_Filters
			lardataobj_RawData
			larevt_CalibrationDBI_IOVData
			larevt_CalibrationDBI_Providers
			lardataobj_RecoBase
			SignalShapingServiceMicroBooNE_service
                        art::Framework_Core
			art::Framework_Principal
			art::Framework_Services_Registry
			art_root_io::tfile_support ROOT::Core
			art_root_io::TFileService_service
			art::Persistency_Common
			art::Persistency_Provenance
			art::Utilities
                        canvas::canvas
			messagefacility::MF_MessageLogger

	                fhiclcpp::fhiclcpp
			cetlib::cetlib
                        cetlib_except::cetlib_except
                        ROOT::Geom
                        ROOT::XMLIO
                        ROOT::Gdml
			ROOT::FFTW

        )

install_headers()
install_fhicl()
install_source()
