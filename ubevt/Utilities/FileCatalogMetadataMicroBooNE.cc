////////////////////////////////////////////////////////////////////////
// Name:  FileCatalogMetadataMicroBooNE_service.cc.  
//
// Purpose:  Implementation for FileCatalogMetadataMicroBooNE.
//
// Created:  28-Oct-2014,  H. Greenlee
//
////////////////////////////////////////////////////////////////////////

#include "ubevt/Utilities/FileCatalogMetadataMicroBooNE.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/Handle.h"
#include "larcoreobj/SummaryData/POTSummary.h"
#include "cetlib_except/exception.h"

//--------------------------------------------------------------------
// Constructor.

util::FileCatalogMetadataMicroBooNE::
FileCatalogMetadataMicroBooNE(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg)
{
  // Get parameters.

  fFCLName = pset.get<std::string>("FCLName");
  fFCLVersion = pset.get<std::string>("FCLVersion");
  fProjectName = pset.get<std::string>("ProjectName");
  fProjectStage = pset.get<std::string>("ProjectStage");
  fProjectVersion = pset.get<std::string>("ProjectVersion");    
  fMerge = pset.get<int>("Merge", 0);
  fParameters = pset.get<std::vector<std::string> >("Parameters", std::vector<std::string>());

  // It doesn't make sense for parameter vector to have an odd number of elements.

  if(fParameters.size() % 2 != 0) {
    throw cet::exception("FileCatalogMetadataMicroBooNE") 
      << "Parameter vector has odd number of elements.\n";
  }

  // Register for callbacks.

  reg.sPostBeginJob.watch(this, &FileCatalogMetadataMicroBooNE::postBeginJob);
  reg.sPostEndSubRun.watch(this, &FileCatalogMetadataMicroBooNE::postEndSubRun);

}

//--------------------------------------------------------------------
// PostBeginJob callback.
// Insert per-job metadata via FileCatalogMetadata service.
void util::FileCatalogMetadataMicroBooNE::postBeginJob()
{
  // Get art metadata service.

  art::ServiceHandle<art::FileCatalogMetadata> mds;

  // Add metadata.

  mds->addMetadata("fclName", fFCLName);
  mds->addMetadata("fclVersion", fFCLVersion);
  mds->addMetadata("ubProjectName", fProjectName);
  mds->addMetadata("ubProjectStage", fProjectStage);
  mds->addMetadata("ubProjectVersion", fProjectVersion);
  std::ostringstream ostr;
  ostr << fMerge;
  mds->addMetadata("merge", ostr.str());
  mds->addMetadata("merged", "0");
  for(unsigned int i=0; i<fParameters.size(); i += 2)
    mds->addMetadata(fParameters[i], fParameters[i+1]);
}

//--------------------------------------------------------------------
// PostEndSubrun callback.
void util::FileCatalogMetadataMicroBooNE::postEndSubRun(art::SubRun const& sr)
{

  art::ServiceHandle<art::FileCatalogMetadata> mds;

  std::string fPOTModuleLabel= "generator";
  art::Handle< sumdata::POTSummary > potListHandle;
  if(sr.getByLabel(fPOTModuleLabel,potListHandle)){
    fTotPOT+=potListHandle->totpot;}

  if(fTotPOT > 0.) {
    std::ostringstream streamObj;
    streamObj << fTotPOT;
    std::string strPOT = streamObj.str();
    mds->addMetadata("mc.pot", strPOT);
  }
}
