////////////////////////////////////////////////////////////////////////
// Name:  TFileMetadataMicroBooNE_service.cc.  
//
// Purpose:  generate microboone-specific sam metadata for root Tfiles (histogram or ntuple files).
//
// FCL parameters: GenerateTFileMetadata: This needs to be set to "true" in the fcl file
//				    to generate metadata (default value: false)
//                 JSONFileName: Name of generates .json file.
//	     	   dataTier: Currrently this needs to be parsed by the user
//		     	     for ntuples, dataTier = root-tuple; 
//		             for histos, dataTier = root-histogram
//		             (default value: root-tuple)
//	           fileFormat: This is currently specified by the user,
//			       the fileFormat for Tfiles is "root" (default value: root)
//                 Merge: Merge flag (0 or 1).  If true, include sam metadata parameters
//                        merge.merge = 1 and merge.merged = 0.
//
//                 These parameters can be scalars or sequences.  In case of sequences
//                 with length greater than one, multiple json files will be
//                 generated.  Sequences must be equal length.
//
// Other notes: 1. This service uses the ART's standard file_catalog_metadata service
//		to extract some of the common (common to both ART and TFile outputs)
//	        job-specific metadata parameters, so, it is important to call this
//  		service in your fcl file
//		stick this line in your "services" section of fcl file:
//		FileCatalogMetadata:  @local::art_file_catalog_mc
//	
//              2. When you call FileCatalogMetadata service in your fcl file, and if 
//		you have (art) root Output section in your fcl file, and if you do not 
//		have "dataTier" specified in that section, then this service will throw
//		an exception. To avoid this, either remove the entire root Output section
//		in your fcl file (and remove art stream output from your end_paths) or 
//		include appropriate dataTier information in the section.If you are only
//		running analysis job, best way is to not include any art root Output section.
//
//	        3. This service is exclusively written to work with production (in other
//		words for jobs submitted through grid). Some of the metadata parameters
//		(output TFileName, filesize, Project related details) are captured/updated
//		during and/or after the workflow. So, if you are trying to generate metadata
//		stand-alone, although you will generate metadata, you will miss the
//		fields coming from workflow. so your generated metadata won't be valid to
//		be declared to SAM.
//		Please look at this wiki for more details: soon to come!
//	
//
// Created:  15-Jan-2015,  S. Gollapinni
//
////////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "ubevt/Utilities/TFileMetadataMicroBooNE.h"
#include "ubevt/Utilities/FileCatalogMetadataMicroBooNE.h"
#include "art/Framework/Principal/SubRun.h"
#include "larcoreobj/SummaryData/POTSummary.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Utilities/OutputFileInfo.h"
#include "art_root_io/RootDB/SQLErrMsg.h"
#include "cetlib_except/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTimeStamp.h"
#include <ctime>
#include <stdio.h>
#include <time.h>

using namespace std;


//--------------------------------------------------------------------

// Constructor.
util::TFileMetadataMicroBooNE::TFileMetadataMicroBooNE(fhicl::ParameterSet const& pset, 
							art::ActivityRegistry& reg):
  fGenerateTFileMetadata(false)							
{
  reconfigure(pset);

  // Register for callbacks.
 
  reg.sPostBeginJob.watch(this, &TFileMetadataMicroBooNE::postBeginJob);
  reg.sPostOpenFile.watch(this, &TFileMetadataMicroBooNE::postOpenFile);
  reg.sPostEndJob.watch(this, &TFileMetadataMicroBooNE::postEndJob);
  reg.sPostProcessEvent.watch(this, &TFileMetadataMicroBooNE::postEvent);
  reg.sPostBeginSubRun.watch(this, &TFileMetadataMicroBooNE::postBeginSubRun);
  reg.sPostEndSubRun.watch(this, &TFileMetadataMicroBooNE::postEndSubRun);
}

//--------------------------------------------------------------------
// Set service paramters
void util::TFileMetadataMicroBooNE::reconfigure(fhicl::ParameterSet const& pset)
{    
  // Get parameters.
  fGenerateTFileMetadata.erase(fGenerateTFileMetadata.begin(), fGenerateTFileMetadata.end());
  if(pset.is_key_to_atom("GenerateTFileMetadata"))
    fGenerateTFileMetadata.push_back(pset.get<bool>("GenerateTFileMetadata"));
  else if(pset.is_key_to_sequence("GenerateTFileMetadata"))
    fGenerateTFileMetadata = pset.get<std::vector<bool> >("GenerateTFileMetadata");

  fJSONFileName.erase(fJSONFileName.begin(), fJSONFileName.end());
  if(pset.is_key_to_atom("JSONFileName"))
    fJSONFileName.push_back(pset.get<std::string>("JSONFileName"));
  else if(pset.is_key_to_sequence("JSONFileName"))
    fJSONFileName = pset.get<std::vector<std::string> >("JSONFileName");
  if(fJSONFileName.size() != fGenerateTFileMetadata.size())
    throw cet::exception("TFileMetadataMicroBooNE") << "FCL sequence size mismatch.\n";
    
  fDataTier.erase(fDataTier.begin(), fDataTier.end());
  if(pset.is_key_to_atom("dataTier"))
    fDataTier.push_back(pset.get<std::string>("dataTier"));
  else if(pset.is_key_to_sequence("dataTier"))
    fDataTier = pset.get<std::vector<std::string> >("dataTier");
  if(fDataTier.size() != fGenerateTFileMetadata.size())
    throw cet::exception("TFileMetadataMicroBooNE") << "FCL sequence size mismatch.\n";

  fFileFormat.erase(fFileFormat.begin(), fFileFormat.end());
  if(pset.is_key_to_atom("fileFormat"))
    fFileFormat.push_back(pset.get<std::string>("fileFormat"));
  else if(pset.is_key_to_sequence("fileFormat"))
    fFileFormat = pset.get<std::vector<std::string> >("fileFormat");
  if(fFileFormat.size() != fGenerateTFileMetadata.size())
    throw cet::exception("TFileMetadataMicroBooNE") << "FCL sequence size mismatch.\n";

  fMerge.erase(fMerge.begin(), fMerge.end());
  if(pset.has_key("Merge")) {
    if(pset.is_key_to_atom("Merge"))
      fMerge.push_back(pset.get<bool>("Merge"));
    else if(pset.is_key_to_sequence("Merge"))
      fMerge = pset.get<std::vector<bool> >("Merge");
  }

  fEnable = false;
  for(bool enable : fGenerateTFileMetadata) {
    if(enable) {
      fEnable = true;
      break;
    }
  }
}

//--------------------------------------------------------------------
// PostBeginJob callback.
// Insert per-job metadata via TFileMetadata service.
void util::TFileMetadataMicroBooNE::postBeginJob()
{ 
  // only generate metadata when this is true
  if (!fEnable) return;
    
  // get the start time  
  md.fstart_time = time(0); 
  
  // Get art metadata service and extract paramters from there
  art::ServiceHandle<art::FileCatalogMetadata> artmds;
  
  art::FileCatalogMetadata::collection_type artmd;
  artmds->getMetadata(artmd);
  
  for(auto const & d : artmd)
    mdmap[d.first] = d.second;
    
  std::map<std::string,std::string>::iterator it;
  
  // if a certain paramter/key is not found, assign an empty string value to it
  
  if ((it=mdmap.find("application.family"))!=mdmap.end()) std::get<0>(md.fapplication) = it->second;
  else std::get<0>(md.fapplication) = "\" \"";   
   
  if ((it=mdmap.find("art.process_name"))!=mdmap.end()) std::get<1>(md.fapplication) = it->second;
  else std::get<1>(md.fapplication) = "\" \"";  
  
  if ((it=mdmap.find("application.version"))!=mdmap.end()) std::get<2>(md.fapplication) = it->second;
  else  std::get<2>(md.fapplication) = "\" \"";   
  
  if ((it=mdmap.find("group"))!=mdmap.end()) md.fgroup = it->second;
  else md.fgroup = "\" \"";   
    
  if ((it=mdmap.find("file_type"))!=mdmap.end()) md.ffile_type = it->second;
  else  md.ffile_type = "\" \"";  
    
  if ((it=mdmap.find("art.run_type"))!=mdmap.end()) frunType = it->second;
  else frunType = "\" \"";         	     	
}


//--------------------------------------------------------------------        
// PostOpenFile callback.
void util::TFileMetadataMicroBooNE::postOpenFile(std::string const& fn)
{
  if (!fEnable) return;
  
  // save parent input files here
  md.fParents.insert(fn);
  
}

//--------------------------------------------------------------------  	
// PostEvent callback.
void util::TFileMetadataMicroBooNE::postEvent(art::Event const& evt,
                                              art::ScheduleContext)
{
 
  if(!fEnable) return;	
  
  art::RunNumber_t run = evt.run();
  art::SubRunNumber_t subrun = evt.subRun();
  art::EventNumber_t event = evt.event();
  art::SubRunID srid = evt.id().subRunID();
      
  // save run, subrun and runType information once every subrun    
  if (fSubRunNumbers.count(srid) == 0){
    fSubRunNumbers.insert(srid);
    md.fruns.push_back(make_tuple(run, subrun, frunType));   
  }
  
  // save the first event
  if (md.fevent_count == 0) md.ffirst_event = event;
  md.flast_event = event;
  // event counter
  ++md.fevent_count;
    
}

//--------------------------------------------------------------------  	
// PostSubRun callback.
void util::TFileMetadataMicroBooNE::postBeginSubRun(art::SubRun const& sr)
{

  if(!fEnable) return;

  art::RunNumber_t run = sr.run();
  art::SubRunNumber_t subrun = sr.subRun();
  art::SubRunID srid = sr.id();

  // save run, subrun and runType information once every subrun
  if (fSubRunNumbers.count(srid) == 0){
    fSubRunNumbers.insert(srid);
    md.fruns.push_back(make_tuple(run, subrun, frunType));
  }
}

//--------------------------------------------------------------------  	
// PostEndSubRun callback.
void util::TFileMetadataMicroBooNE::postEndSubRun(art::SubRun const& sr)
{

  if(!fEnable) return;
 
  std::string fPOTModuleLabel= "generator";
  art::Handle< sumdata::POTSummary > potListHandle;
  double fTotPOT = 0;
  if(sr.getByLabel(fPOTModuleLabel,potListHandle)){
    fTotPOT+=potListHandle->totpot;
  }

  md.fTotPOT += fTotPOT;
  
}


//--------------------------------------------------------------------
// PostCloseFile callback.
void util::TFileMetadataMicroBooNE::postEndJob()
{
	
   // Do nothing if generating TFile metadata is disabled.	
  if(!fEnable) return;	
  
  //get job submission related paramters from FileCatalogMetadataMicroBooNE service
        
  art::ServiceHandle<util::FileCatalogMetadataMicroBooNE> paramhandle;	
  
  md.ffcl_name        = paramhandle->FCLName();
  md.ffcl_version     = paramhandle->FCLVersion();
  md.fproject_name    = paramhandle->ProjectName();
  md.fproject_stage   = paramhandle->ProjectStage();
  md.fproject_version = paramhandle->ProjectVersion();
  
  //update end time
  md.fend_time = time(0);

  // convert start and end times into time format: Year-Month-DayTHours:Minutes:Seconds
  char endbuf[80], startbuf[80];
  struct tm tstruct;
  tstruct = *localtime(&md.fend_time);
  strftime(endbuf,sizeof(endbuf),"%Y-%m-%dT%H:%M:%S",&tstruct);
  tstruct = *localtime(&md.fstart_time);
  strftime(startbuf,sizeof(startbuf),"%Y-%m-%dT%H:%M:%S",&tstruct);

  // Loop over TFiles.

  for(unsigned int i=0; i<fGenerateTFileMetadata.size(); ++i) {

    if(fGenerateTFileMetadata[i]) {
  
      // open a json file and write everything from the struct md complying to the 
      // samweb json format. This json file holds the below information temporarily. 
      // If you submitted a grid job invoking this service, the information from 
      // this file is appended to a final json file and this file will be removed
  
      std::ofstream jsonfile;
      jsonfile.open(fJSONFileName[i]);
      jsonfile<<"{\n  \"application\": {\n    \"family\": "<<std::get<0>(md.fapplication)<<",\n    \"name\": ";
      jsonfile<<std::get<1>(md.fapplication)<<",\n    \"version\": "<<std::get<2>(md.fapplication)<<"\n  },\n  ";
      jsonfile<<"\"data_tier\": \""<<fDataTier[i]<<"\",\n  ";
      jsonfile<<"\"end_time\": \""<<endbuf<<"\",\n  ";
      jsonfile<<"\"event_count\": "<<md.fevent_count<<",\n  ";
      jsonfile<<"\"fcl.name\": \""<<md.ffcl_name<<"\",\n  ";
      jsonfile<<"\"fcl.version\":  \""<<md.ffcl_version<<"\",\n  ";
      jsonfile<<"\"file_format\": \""<<fFileFormat[i]<<"\",\n  ";
      jsonfile<<"\"file_type\": "<<md.ffile_type<<",\n  ";
      jsonfile<<"\"first_event\": "<<md.ffirst_event<<",\n  ";
      jsonfile<<"\"group\": "<<md.fgroup<<",\n  ";
      jsonfile<<"\"last_event\": "<<md.flast_event<<",\n  ";
      jsonfile<<"\"mc.pot\": "<<md.fTotPOT<<",\n  ";
      //if (md.fdataTier != "generated"){
      unsigned int c=0;
      jsonfile<<"\"parents\": [\n";
      for(auto parent : md.fParents) {
	c++;
	size_t n = parent.find_last_of('/');
	size_t f1 = (n == std::string::npos ? 0 : n+1);
	jsonfile<<"    {\n     \"file_name\": \""<<parent.substr(f1)<<"\"\n    }";
	if (md.fParents.size()==1 || c==md.fParents.size()) jsonfile<<"\n";
	else jsonfile<<",\n"; 
      }      
      jsonfile<<"  ],\n  "; 
      //}   
      c=0;
      jsonfile<<"\"runs\": [\n";
      for(auto &t : md.fruns){
	c++;
	jsonfile<<"    [\n     "<<std::get<0>(t)<<",\n     "<<std::get<1>(t)<<",\n     "<<std::get<2>(t)<<"\n    ]";
	if (md.fruns.size()==1 || c==md.fruns.size()) jsonfile<<"\n";
	else jsonfile<<",\n"; 
      }
      jsonfile<<"  ]";          
      jsonfile<<",\n  \"start_time\": \""<<startbuf<<"\"";  
      jsonfile<<",\n  \"ub_project.name\": \""<<md.fproject_name<<"\"";
      jsonfile<<",\n  \"ub_project.stage\": \""<<md.fproject_stage<<"\"";
      jsonfile<<",\n  \"ub_project.version\": \""<<md.fproject_version<<"\"";

      if(fMerge.size() > i && fMerge[i]) {
	jsonfile << ",\n  \"merge.merge\": 1";
	jsonfile << ",\n  \"merge.merged\": 0";
      }
  
      jsonfile<<"\n}";
      jsonfile.close();
    }
  } 
}
