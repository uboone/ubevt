# basic source code CMakeLists.txt

art_make(
	MODULE_LIBRARIES
                        larcoreobj_SummaryData
                        lardataobj_Simulation
			nusimdata::SimulationBase
                        art::Framework_Core
			art::Framework_IO_ProductMix
			art_root_io::art_root_io
                        art_root_io_detail
                        art::Framework_Principal
                        art::Framework_Services_Registry
                        art::Persistency_Provenance
                        art::Persistency_Common
                        art::Utilities
                        canvas::canvas
			gallery
                        messagefacility::MF_MessageLogger
                        fhiclcpp::fhiclcpp
                        cetlib::cetlib cetlib_except

)

install_headers()
install_source()
install_fhicl()
