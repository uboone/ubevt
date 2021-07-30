art_make(BASENAME_ONLY
         MODULE_LIBRARIES
              lardataobj_RecoBase
              lardata_Utilities
              lardataalg_DetectorInfo
              larevt_SpaceCharge
              ubevt_SpaceCharge
              cetlib::cetlib
              ROOT::Tree
          LIB_LIBRARIES
              lardataobj_RecoBase
              lardata_Utilities
              lardataalg_DetectorInfo
              larevt_SpaceCharge
              ${ART_FRAMEWORK_CORE}
              ${ART_FRAMEWORK_PRINCIPAL}
              ${ART_FRAMEWORK_SERVICES_REGISTRY}
              art_Persistency_Common
              art_Persistency_Provenance
              ${ART_ROOT_IO_TFILE_SUPPORT}
              ${ROOT_CORE}
              ${ART_ROOT_IO_TFILESERVICE_SERVICE}
              art_Utilities canvas
              ${MF_MESSAGELOGGER}
              cetlib::cetlib
              larcorealg_Geometry
              ${FHICLCPP}
              ${GSL}
        )


install_headers()
install_fhicl()
install_source()
