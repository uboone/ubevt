# basic source code CMakeLists.txt

art_make(
	MODULE_LIBRARIES
                        larcoreobj_SummaryData
                        lardataobj_Simulation
			nusimdata::SimulationBase
                        art::Framework_Core
			${ART_FRAMEWORK_IO_PRODUCTMIX}
			art_root_io::art_root_io
                        art_root_io_detail
			${ART_FRAMEWORK_MODULES}
                        art::Framework_Principal
                        art::Framework_Services_Registry
                        ${ART_FRAMEWORK_SERVICES_SYSTEM_FILECATALOGMETADATA_SERVICE}
                        art::Persistency_Provenance
                        art::Persistency_Common
                        art::Utilities
                        canvas::canvas
			gallery
                        messagefacility::MF_MessageLogger
                        ${BOOST_LIB}
                        fhiclcpp::fhiclcpp
                        cetlib::cetlib cetlib_except

)

install_headers()
install_source()
install_fhicl()
