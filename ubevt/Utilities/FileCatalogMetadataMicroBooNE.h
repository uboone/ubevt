////////////////////////////////////////////////////////////////////////
//
// Name:  FileCatalogMetadataMicroBooNE.h.  
//
// Purpose:  Art service adds microboone-specific per-job sam metadata.
//
//           FCL parameters:
//
//           FCLName        - FCL file name.
//           FCLVersion     - FCL file version.
//           ProjectName    - Project name.
//           ProjectStage   - Project stage.
//           ProjectVersion - Project version.
//           Merge          - Merge flag (0 or 1).
//           Parameters     - Arbitrary (key, value) parameters
//
//           Above values will be added in internal metadata of artroot
//           output files whenever this service is included in job
//           configuration (service does not need to be called).  The
//           public interface of this service consists of accessors that
//           allow other code to discover above metadata parameters.
//
// Created:  28-Oct-2014,  H. Greenlee
//
////////////////////////////////////////////////////////////////////////

#ifndef FILECATALOGMETADATAMICROBOONE_H
#define FILECATALOGMETADATAMICROBOONE_H

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace util {

  class FileCatalogMetadataMicroBooNE
  {
  public:

    // Constructor, destructor.

    FileCatalogMetadataMicroBooNE(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
    ~FileCatalogMetadataMicroBooNE() = default;

    // Accessors.

    const std::string& FCLName() const {return fFCLName;}
    const std::string& FCLVersion() const {return fFCLVersion;}
    const std::string& ProjectName() const {return fProjectName;}
    const std::string& ProjectStage() const {return fProjectStage;}
    const std::string& ProjectVersion() const {return fProjectVersion;}
    int Merge() const {return fMerge;}
    const std::vector<std::string>& Parameters() const {return fParameters;}

  private:

    // Callbacks.

    void postBeginJob();
    void postEndSubRun(art::SubRun const& subrun);

    // Data members.

    std::string fFCLName;
    std::string fFCLVersion;
    std::string fProjectName;
    std::string fProjectStage;
    std::string fProjectVersion;
    int fMerge;
    std::vector<std::string> fParameters;

    double fTotPOT = 0;
    
  };

} // namespace util

DECLARE_ART_SERVICE(util::FileCatalogMetadataMicroBooNE, LEGACY)

#endif
