art_make( BASENAME_ONLY
          LIBRARY_NAME CalData
          LIB_LIBRARIES
                        lardataobj_RawData
                        lardataobj_RecoBase
                        lardata_Utilities
                        fhiclcpp::fhiclcpp
                        cetlib::cetlib
                        cetlib_except::cetlib_except

          MODULE_LIBRARIES larcorealg_Geometry
                        CalData
                        ubevt_CalData_DeconTools
                        ubobj_Trigger
                        ubobj_RawData
                        NoiseFilterAlgs
                        larcore_Geometry_Geometry_service
                        lardata_Utilities
                        larevt_Filters
                        lardataobj_RawData
                        larevt_CalibrationDBI_IOVData
                        larevt_CalibrationDBI_Providers
                        lardataobj_RecoBase
                        lardata_ArtDataHelper
                        SignalShapingServiceMicroBooNE_service
                        art::Framework_Core
                        art::Framework_Principal
                        art::Framework_Services_Registry
                        art_root_io::TFileService_service
                        art::Persistency_Common
                        art::Persistency_Provenance
                        art::Utilities canvas::canvas
                        messagefacility::MF_MessageLogger
                        fhiclcpp::fhiclcpp
                        cetlib::cetlib
                        ROOT::Geom
                        ROOT::XMLIO
                        ROOT::Gdml
                        ROOT::FFTW
                        ROOT::Tree
        )

add_subdirectory(NoiseFilterAlgs)
add_subdirectory(DeconTools)

install_headers()
install_fhicl()
install_source()
