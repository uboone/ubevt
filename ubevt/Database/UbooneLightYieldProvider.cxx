#ifndef UBOONELIGHTYIELDPROVIDER_CXX
#define UBOONELIGHTYIELDPROVIDER_CXX

#include "ubevt/Database/UbooneLightYieldProvider.h"
#include "larevt/CalibrationDBI/Providers/WebError.h"

// art/LArSoft libraries
#include "cetlib_except/exception.h"
#include "larcore/Geometry/Geometry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Registry/ServiceHandle.h" 


#include <fstream>

namespace lariov {

  //constructor      
  UbooneLightYieldProvider::UbooneLightYieldProvider(fhicl::ParameterSet const& p) :
    DatabaseRetrievalAlg(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg")),
    fEventTimeStamp(0),
    fCurrentTimeStamp(0) {
    
    this->Reconfigure(p);
  }
      
  void UbooneLightYieldProvider::Reconfigure(fhicl::ParameterSet const& p) {
    
    this->DatabaseRetrievalAlg::Reconfigure(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg"));
    fData.Clear();
    IOVTimeStamp tmp = IOVTimeStamp::MaxTimeStamp();
    tmp.SetStamp(tmp.Stamp()-1, tmp.SubStamp());
    fData.SetIoV(tmp, IOVTimeStamp::MaxTimeStamp());

    bool UseDB      = p.get<bool>("UseDB", false);
    bool UseFile    = p.get<bool>("UseFile", false);
    std::string fileName = p.get<std::string>("FileName", "");

    //priority:  (1) use db, (2) use table, (3) use defaults
    //If none are specified, use defaults
    if ( UseDB )      fDataSource = DataSource::Database;
    else if (UseFile) fDataSource = DataSource::File;
    else              fDataSource = DataSource::Default;

    if (fDataSource == DataSource::Default) {
      float default_lyscale         = p.get<float>("DefaultLYScale");
      float default_lyscale_err     = p.get<float>("DefaultLYScaleErr");
      
      float default_promptlight     = p.get<float>("DefaultPromptLight");
      float default_latelight       = p.get<float>("DefaultLateLight");
      std::string default_xmodel    = "NONE";
      
      PmtGain defaultGain(0);
      CalibrationExtraInfo extra_info("PmtGain");

      defaultGain.SetGain(default_lyscale);
      defaultGain.SetGainErr(default_lyscale_err);
      extra_info.AddOrReplaceFloatData("promptlight",default_promptlight);
      extra_info.AddOrReplaceFloatData("latelight",default_latelight);
      extra_info.AddOrReplaceStringData("xdependencemodel",default_xmodel);
      
      defaultGain.SetExtraInfo(extra_info);
      
      art::ServiceHandle<geo::Geometry> geo;
      for (unsigned int od=0; od!=geo->NOpDets(); ++od) {
        if (geo->IsValidOpChannel(od)) {
	  defaultGain.SetChannel(od);
	  fData.AddOrReplaceRow(defaultGain);
	}
      }
      
    }
    else if (fDataSource == DataSource::File) {
      cet::search_path sp("FW_SEARCH_PATH");
      std::string abs_fp = sp.find_file(fileName);
      std::cout << "Using light yield scaling from local file: "<<abs_fp<<"\n";
      std::ifstream file(abs_fp);
      if (!file) {
        throw cet::exception("UbooneLightYieldProvider")
          << "File "<<abs_fp<<" is not found.";
      }

      std::string line;
      PmtGain dp(0);
      while (std::getline(file, line)) {
        if (line[0] == '#') continue;
        size_t current_comma = line.find(',');
        DBChannelID_t ch = (DBChannelID_t)std::stoi(line.substr(0, current_comma));   
        float lyscale           = std::stof( line.substr(current_comma+1, line.find(',',current_comma+1)-(current_comma+1)) );
        
        current_comma = line.find(',',current_comma+1);
        float lyscale_err       = std::stof( line.substr(current_comma+1, line.find(',',current_comma+1)-(current_comma+1)) );
	
	current_comma = line.find(',',current_comma+1);
        float promptlight       = std::stof( line.substr(current_comma+1, line.find(',',current_comma+1)-(current_comma+1)) );
        
        current_comma = line.find(',',current_comma+1);
        float latelight   = std::stof( line.substr(current_comma+1, line.find(',',current_comma+1)-(current_comma+1)) );
	
	current_comma = line.find(',',current_comma+1);
        std::string xmodel = line.substr(current_comma+1, line.find(',',current_comma+1)-(current_comma+1));
	
        CalibrationExtraInfo extra_info("PmtGain");
	extra_info.AddOrReplaceFloatData("promptlight",promptlight);
	extra_info.AddOrReplaceFloatData("latelight",latelight);
	extra_info.AddOrReplaceStringData("xdependencemodel",xmodel);

        dp.SetChannel(ch);
        dp.SetGain(lyscale);
        dp.SetGainErr(lyscale_err);
	dp.SetExtraInfo(extra_info);
	       
        fData.AddOrReplaceRow(dp);
      }
    }
    else {
      std::cout << "Using light yield from conditions database"<<std::endl;
    }
  }

  // This method saves the time stamp of the latest event.

  void UbooneLightYieldProvider::UpdateTimeStamp(DBTimeStamp_t ts) {
    mf::LogInfo("UbooneLightYieldProvider") << "UbooneLightYieldProvider::UpdateTimeStamp called.";
    fEventTimeStamp = ts;
  }

  // Maybe update method cached data (public non-const version).

  bool UbooneLightYieldProvider::Update(DBTimeStamp_t ts) {
    
    fEventTimeStamp = ts;
    return DBUpdate(ts);
  }

  // Maybe update method cached data (private const version using current event time).

  bool UbooneLightYieldProvider::DBUpdate() const {
    return DBUpdate(fEventTimeStamp);
  }

  // Maybe update method cached data (private const version).
  // This is the function that does the actual work of updating data from database.

  bool UbooneLightYieldProvider::DBUpdate(DBTimeStamp_t ts) const {

    bool result = false;
    
    if (fDataSource == DataSource::Database && ts != fCurrentTimeStamp) {
      
      mf::LogInfo("UbooneLightYieldProvider") << "UbooneLightYieldProvider::DBUpdate called with new timestamp.";

      fCurrentTimeStamp = ts;     

      // Call non-const base class method.

      result = const_cast<UbooneLightYieldProvider*>(this)->UpdateFolder(ts);
      if(result) {
	//DBFolder was updated, so now update the Snapshot
	fData.Clear();
	fData.SetIoV(this->Begin(), this->End());

	std::vector<DBChannelID_t> channels;
	fFolder->GetChannelList(channels);
	for (auto it = channels.begin(); it != channels.end(); ++it) {

	  double lyscale, lyscale_err, promptlight, latelight;
	  std::string xdependencemodel;
	  fFolder->GetNamedChannelData(*it, "lightyieldscale",       lyscale);
	  fFolder->GetNamedChannelData(*it, "lightyieldscaleerror",  lyscale_err); 
	  fFolder->GetNamedChannelData(*it, "promptlight",           promptlight);
	  fFolder->GetNamedChannelData(*it, "latelight",             latelight); 
	  fFolder->GetNamedChannelData(*it, "xdependencemodel",      xdependencemodel);
      
	  PmtGain pg(*it);
	  CalibrationExtraInfo extra_info("PmtGain");
	  extra_info.AddOrReplaceFloatData("promptlight",promptlight);
	  extra_info.AddOrReplaceFloatData("latelight",latelight);
	  extra_info.AddOrReplaceStringData("xdependencemodel",xdependencemodel);

	  pg.SetGain( (float)lyscale );
	  pg.SetGainErr( (float)lyscale_err );
	  pg.SetExtraInfo(extra_info);

	  fData.AddOrReplaceRow(pg);
	}
      }
    }

    return result;
  }
  
  const PmtGain& UbooneLightYieldProvider::PmtGainObject(DBChannelID_t ch) const { 
    DBUpdate();
    return fData.GetRow(ch);
  }
      
  float UbooneLightYieldProvider::LYScaling(DBChannelID_t ch) const {
    return this->PmtGainObject(ch).Gain();
  }
  
  float UbooneLightYieldProvider::LYScalingErr(DBChannelID_t ch) const {
    return this->PmtGainObject(ch).GainErr();
  }
  
  CalibrationExtraInfo const& UbooneLightYieldProvider::ExtraInfo(DBChannelID_t ch) const {
    return this->PmtGainObject(ch).ExtraInfo();
  }


}//end namespace lariov
	
#endif
        
