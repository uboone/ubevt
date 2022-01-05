cet_enable_asserts()

art_make ( NO_PLUGINS
           BASENAME_ONLY
	   LIB_LIBRARIES lardata_Utilities_LArFFT_service
	   	         art::Framework_Core
                         art::Framework_Principal
                         art::Framework_Services_Registry
			 messagefacility::MF_MessageLogger

			 art::Persistency_Common canvas::canvas
			 art::Persistency_Provenance canvas::canvas
			 art::Utilities canvas::canvas
			 cetlib::cetlib cetlib_except

	                 fhiclcpp::fhiclcpp
			 Boost::filesystem
	  )

set( uboone_util_lib_list lardata_Utilities_LArFFT_service
			larcorealg_Geometry
			larevt_CalibrationDBI_IOVData
			larevt_CalibrationDBI_Providers
                        lardataalg_DetectorInfo
			lardataobj_RecoBase
			larcore_Geometry_Geometry_service
			lardata_Utilities
			ubevt_Utilities
			nurandom::RandomUtils_NuRandomService_service
  			art::Framework_Core
			art::Framework_Principal
			art::Framework_Services_Registry
			art::Persistency_Common canvas::canvas
			art::Persistency_Provenance canvas::canvas
			art_root_io::tfile_support ROOT::Core
                        art::Framework_Services_Optional_RandomNumberGenerator_service
			art_root_io::TFileService_service
			art::Utilities canvas::canvas
			messagefacility::MF_MessageLogger

			fhiclcpp::fhiclcpp
			cetlib::cetlib cetlib_except
			Boost::filesystem
                        CLHEP::CLHEP

    )


simple_plugin( SignalShapingServiceMicroBooNE  "service"
               ${uboone_util_lib_list}
	       BASENAME_ONLY
        )
simple_plugin( FileCatalogMetadataMicroBooNE "service"
               ${uboone_util_lib_list}
	       BASENAME_ONLY
	)
simple_plugin( TFileMetadataMicroBooNE "service"
               ${uboone_util_lib_list}
	       BASENAME_ONLY
	)
simple_plugin( PMTRemapService "service"
               ${uboone_util_lib_list}
	       BASENAME_ONLY
	)
simple_plugin( TinyHitFinderAna "module"
               ${uboone_util_lib_list}
	       BASENAME_ONLY
        )
simple_plugin( RandomPrescale "module"
               ${uboone_util_lib_list}
	       BASENAME_ONLY
	)
simple_plugin( OpHitRemapProducer "module"
               ${uboone_util_lib_list}
               BASENAME_ONLY
	)
simple_plugin( FileCatalogMetadataRawDigit "plugin"
               ${uboone_util_lib_list}
	       BASENAME_ONLY
	)



install_headers()
# do not install duplicate standard_reco_uboone.fcl file
install_fhicl( EXCLUDES standard_reco_uboone.fcl )
install_source()

add_subdirectory(test)
