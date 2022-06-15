# basic source code CMakeLists.txt

art_make(
    MODULE_LIBRARIES
    lardataobj_Simulation
    nusimdata_SimulationBase
    larcoreobj_SummaryData
    ${ART_FRAMEWORK_CORE}
    ${ART_FRAMEWORK_IO_PRODUCTMIX}
    ${ART_FRAMEWORK_IO_ROOT}
    art_Framework_IO_Root_detail_sources
    ${ART_FRAMEWORK_MODULES}
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    ${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
    art_Persistency_Provenance
    art_Persistency_Common
    art_Utilities
    canvas
    gallery
    ifdh
    ${MF_MESSAGELOGGER}
    ${BOOST_LIB}
    ${FHICLCPP}
    cetlib cetlib_except
    ${ROOT_BASIC_LIB_LIST}
    )	
include_directories( $ENV{IFDHC_FQ_DIR}/inc )
install_headers()
install_source()
install_fhicl()
