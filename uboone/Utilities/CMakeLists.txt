cet_enable_asserts()

art_make ( NO_PLUGINS
           BASENAME_ONLY
	   LIB_LIBRARIES lardata_Utilities_LArFFT_service
	   	         ${ART_FRAMEWORK_CORE}
                         ${ART_FRAMEWORK_PRINCIPAL}
			 ${ART_FRAMEWORK_SERVICES_BASIC}
                         ${ART_FRAMEWORK_SERVICES_REGISTRY}
			 ${MF_MESSAGELOGGER}
			 ${MF_UTILITIES}
			 art_Persistency_Common canvas
			 art_Persistency_Provenance canvas
			 art_Utilities canvas
			 cetlib cetlib_except
			 ${ROOT_BASIC_LIB_LIST}
	                 ${FHICLCPP}
			 ${Boost_FILESYSTEM_LIBRARY}
			 ${Boost_SYSTEM_LIBRARY}
	  )

set( uboone_util_lib_list lardata_Utilities_LArFFT_service
			larcorealg_Geometry
			larevt_CalibrationDBI_IOVData
			larevt_CalibrationDBI_Providers
			larreco_HitFinder
			lardataobj_RecoBase
			larcore_Geometry_Geometry_service
			lardata_Utilities
			uboone_Utilities
			nutools_RandomUtils_NuRandomService_service
  			${ART_FRAMEWORK_CORE}
			${ART_FRAMEWORK_PRINCIPAL}
			${ART_FRAMEWORK_SERVICES_REGISTRY}
			art_Persistency_Common canvas
			art_Persistency_Provenance canvas
			${ART_FRAMEWORK_SERVICES_BASIC}
			${ART_FRAMEWORK_SERVICES_OPTIONAL}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL_RANDOMNUMBERGENERATOR_SERVICE}
			${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
			${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
			art_Utilities canvas
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			${FHICLCPP}
			cetlib cetlib_except
			${Boost_FILESYSTEM_LIBRARY}
			${Boost_SYSTEM_LIBRARY}
                        ${CLHEP}
			${ROOT_BASIC_LIB_LIST}
    )

art_make( BASENAME_ONLY
          LIB_LIBRARIES
	               ${uboone_util_lib_list}
	  MODULE_LIBRARIES
	               ${uboone_util_lib_list}
	  SERVICE_LIBRARIES
	               ${uboone_util_lib_list}
		       uboone_Utilities
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
