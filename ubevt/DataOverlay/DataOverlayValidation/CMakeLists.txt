
art_make(
	  LIB_LIBRARIES
			art::Framework_Core
			art::Utilities canvas::canvas
			messagefacility::MF_MessageLogger
			cetlib::cetlib cetlib_except

          MODULE_LIBRARIES
			larcorealg_Geometry
			larcore_Geometry_Geometry_service
			lardataobj_RecoBase
			lardataobj_AnalysisBase
			lardataobj_RawData
			lardata_RecoObjects
			lardata_Utilities
			larevt_Filters
			nusimdata::SimulationBase
			${LARRECO_LIB}
			${LARDATA_LIB}
                        art::Framework_Core
			art::Framework_Principal
			art::Framework_Services_Registry
			art_root_io::tfile_support ROOT::Core
			art::Framework_Services_Optional_RandomNumberGenerator_service
			art_root_io::TFileService_service
			art::Persistency_Common canvas::canvas
			art::Persistency_Provenance canvas::canvas
			art::Utilities canvas::canvas
			messagefacility::MF_MessageLogger
	                fhiclcpp::fhiclcpp
			cetlib::cetlib cetlib_except
			CLHEP::CLHEP
                        ROOT::Geom
                        ROOT::XMLIO
                        ROOT::Gdml
                        ROOT::Tree
        )

install_headers()
install_fhicl()
install_source()

