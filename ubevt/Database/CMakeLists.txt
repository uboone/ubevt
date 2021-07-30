cet_find_library( LIBWDA NAMES wda PATHS ENV LIBWDA_LIB NO_DEFAULT_PATH )

include_directories( $ENV{LIBWDA_FQ_DIR}/include )

art_make( BASENAME_ONLY
          LIB_LIBRARIES
                        larevt_CalibrationDBI_IOVData
                        larevt_CalibrationDBI_Providers
                        larcorealg_Geometry
                        ${LIBWDA}
                        ${MF_MESSAGELOGGER}
                        ${FHICLCPP}
                        cetlib::cetlib
                        ${ART_FRAMEWORK_SERVICES_REGISTRY}

                        art_Utilities canvas
          MODULE_LIBRARIES larcorealg_Geometry
                        larcore_Geometry_Geometry_service
                        lardata_Utilities
                        larevt_Filters
                        lardataobj_RawData
                        SignalShapingServiceMicroBooNE_service
                        larevt_CalibrationDBI_Providers
                        ubevt_Database
                        ${ART_FRAMEWORK_CORE}
                        ${ART_FRAMEWORK_PRINCIPAL}
                        ${ART_FRAMEWORK_SERVICES_REGISTRY}
                        ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_CORE}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL_RANDOMNUMBERGENERATOR_SERVICE}
                        ${ART_ROOT_IO_TFILESERVICE_SERVICE}
                        art_Persistency_Common canvas
                        art_Persistency_Provenance canvas
                        art_Utilities canvas
                        ${MF_MESSAGELOGGER}

                        ${FHICLCPP}
                        cetlib::cetlib
                        ${CLHEP}
                        ${ROOT_GEOM}
                        ${ROOT_XMLIO}
                        ${ROOT_GDML}
                        ROOT::EG

          SERVICE_LIBRARIES
                      larevt_CalibrationDBI_Providers
                      ubevt_Database
                      ${ART_FRAMEWORK_PRINCIPAL}
                      ${ART_FRAMEWORK_SERVICES_REGISTRY}
                      art_Persistency_Common canvas
                      art_Persistency_Provenance canvas
                      ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_CORE}
                      ${ART_FRAMEWORK_SERVICES_OPTIONAL_RANDOMNUMBERGENERATOR_SERVICE}
                      ${ART_ROOT_IO_TFILESERVICE_SERVICE}
                      ${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
                      art_Utilities canvas
                            larevt_CalibrationDBI_IOVData
                            lardataobj_RawData
                            larcorealg_Geometry
                            larcore_Geometry_Geometry_service
                            ${ART_FRAMEWORK_SERVICES_REGISTRY}
                            ${ART_FRAMEWORK_CORE}
                            ${ART_FRAMEWORK_PRINCIPAL}
                            ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_CORE}
                            ${ART_ROOT_IO_TFILESERVICE_SERVICE}
                            art_Persistency_Common canvas
                            art_Persistency_Provenance canvas
                            art_Utilities canvas
                            ${FHICLCPP}

)

install_headers()
install_fhicl()
install_source()

add_subdirectory(TPCEnergyCalib)
