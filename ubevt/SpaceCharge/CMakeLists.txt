art_make( BASENAME_ONLY

		  MODULE_LIBRARIES
              lardataobj_RecoBase
              lardata_Utilities
              lardataalg_DetectorInfo
              larevt_SpaceCharge
              ubevt_SpaceCharge
              cetlib cetlib_except
		  		
          
          LIB_LIBRARIES
              lardataobj_RecoBase
              lardata_Utilities
              lardataalg_DetectorInfo
              larevt_SpaceCharge
              ${ART_FRAMEWORK_CORE}
              ${ART_FRAMEWORK_PRINCIPAL}
              ${ART_FRAMEWORK_SERVICES_REGISTRY}
              art_Persistency_Common canvas
              art_Persistency_Provenance canvas
              ${ART_FRAMEWORK_SERVICES_BASIC}
              ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_CORE}
              ${ART_ROOT_IO_TFILESERVICE_SERVICE}
#              ${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
              art_Utilities canvas
              ${MF_MESSAGELOGGER}
              
              cetlib cetlib_except
              ${ROOT_BASIC_LIB_LIST}
                       	larcorealg_Geometry
			${FHICLCPP}
                        ${ROOT_BASIC_LIB_LIST}
			${Boost_SYSTEM_LIBRARY}
                        ${GSL}
	)


install_headers()
install_fhicl()
install_source()
