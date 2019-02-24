#ifndef UBOONEELECTRONLIFETIMEPROVIDER_CXX
#define UBOONEELECTRONLIFETIMEPROVIDER_CXX

#include "UbooneElectronLifetimeProvider.h"
//#include "larevt/CalibrationDBI/Providers/WebError.h"

// art/LArSoft libraries
#include "cetlib_except/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

//C/C++
#include <fstream>

namespace lariov {

  //constructors
  UbooneElectronLifetimeProvider::UbooneElectronLifetimeProvider(const std::string& foldername, 
      			      			   const std::string& url, 
			      			   const std::string& tag /*=""*/) : 
    DatabaseRetrievalAlg(foldername, url, tag),
    fEventTimeStamp(0),
    fCurrentTimeStamp(0),
    fDataSource(DataSource::Database) {
    
    fData.Clear();
    IOVTimeStamp tmp = IOVTimeStamp::MaxTimeStamp();
    tmp.SetStamp(tmp.Stamp()-1, tmp.SubStamp());
    fData.SetIoV(tmp, IOVTimeStamp::MaxTimeStamp());
  }
	
      
  UbooneElectronLifetimeProvider::UbooneElectronLifetimeProvider(fhicl::ParameterSet const& p) :
    DatabaseRetrievalAlg(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg")) {	
    
    this->Reconfigure(p);
  }
      
  void UbooneElectronLifetimeProvider::Reconfigure(fhicl::ParameterSet const& p) {
    
    this->DatabaseRetrievalAlg::Reconfigure(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg"));
    fData.Clear();
    IOVTimeStamp tmp = IOVTimeStamp::MaxTimeStamp();
    tmp.SetStamp(tmp.Stamp()-1, tmp.SubStamp());
    fData.SetIoV(tmp, IOVTimeStamp::MaxTimeStamp());

    bool UseDB      = p.get<bool>("UseDB", false);
    bool UseFile   = p.get<bool>("UseFile", false);
    std::string fileName = p.get<std::string>("FileName", "");

    //priority:  (1) use db, (2) use table, (3) use defaults
    //If none are specified, use defaults
    if ( UseDB )      fDataSource = DataSource::Database;
    else if (UseFile) fDataSource = DataSource::File;
    else              fDataSource = DataSource::Default;

    if (fDataSource == DataSource::Default) {
      std::cout << "Using default lifetime fit values\n";
      float default_lifetime       = p.get<float>("DefaultLifetime");
      float default_lifetimeerr    = p.get<float>("DefaultLifetimeErr");
            
      UbooneElectronLifetimeContainer default_pars(fLifetimeChannel);

      default_pars.SetLifetime(default_lifetime);
      default_pars.SetLifetimeErr(default_lifetimeerr);
      fData.AddOrReplaceRow(default_pars);
 
    }
    else if (fDataSource == DataSource::File) {
    cet::search_path sp("FW_SEARCH_PATH");
      std::string abs_fp = sp.find_file(fileName);
      std::cout << "Using pedestals from local file: "<<abs_fp<<"\n";
      std::ifstream file(abs_fp);
      if (!file) {
        throw cet::exception("UbooneElectronLifetimeProvider")
	  << "File "<<abs_fp<<" is not found.";
      }
      
      std::string line;
      UbooneElectronLifetimeContainer dp(fLifetimeChannel);
      while (std::getline(file, line)) {
        size_t current_comma = line.find(',');	
	float lifetime        = std::stof( line.substr(current_comma+1, line.find(',',current_comma+1)-(current_comma+1)) );
	
	current_comma = line.find(',',current_comma+1);
	float lifetime_err = std::stof( line.substr(current_comma+1) );

	dp.SetChannel(fLifetimeChannel);
	dp.SetLifetime(lifetime);
        dp.SetLifetimeErr(lifetime_err);
	fData.AddOrReplaceRow(dp);
	
	break; //only should have one line
      }
    } // if source from file
    else { 
      std::cout << "Using electron lifetime from conditions database\n";
    }
  }


  // This method saves the time stamp of the latest event.

  void UbooneElectronLifetimeProvider::UpdateTimeStamp(DBTimeStamp_t ts) {
    mf::LogInfo("UbooneElectronLifetimeProvider") << "UbooneElectronLifetimeProvider::UpdateTimeStamp called.";
    fEventTimeStamp = ts;
  }

  // Maybe update method cached data (public non-const version).

  bool UbooneElectronLifetimeProvider::Update(DBTimeStamp_t ts) {
    
    fEventTimeStamp = ts;
    return DBUpdate(ts);
  }

  // Maybe update method cached data (private const version using current event time).

  bool UbooneElectronLifetimeProvider::DBUpdate() const {
    return DBUpdate(fEventTimeStamp);
  }

  // Maybe update method cached data (private const version).
  // This is the function that does the actual work of updating data from database.

  bool UbooneElectronLifetimeProvider::DBUpdate(DBTimeStamp_t ts) const {

    bool result = false;
    if (fDataSource == DataSource::Database && ts != fCurrentTimeStamp) {
      
      mf::LogInfo("UbooneElectronLifetimeProvider") << "UbooneElectronLifetimeProvider::DBUpdate called with new timestamp.";

      fCurrentTimeStamp = ts;     

      // Call non-const base class method.

      result = const_cast<UbooneElectronLifetimeProvider*>(this)->UpdateFolder(ts);
      if(result) {
	//DBFolder was updated, so now update the Snapshot
	fData.Clear();
	fData.SetIoV(this->Begin(), this->End());

	double lifetime, lifetime_err;
	fFolder->GetNamedChannelData(fLifetimeChannel, "lifetime",     lifetime);
	fFolder->GetNamedChannelData(fLifetimeChannel, "lifetime_err",          lifetime_err);

	UbooneElectronLifetimeContainer pd(fLifetimeChannel);
	pd.SetLifetime( (float)lifetime );
	pd.SetLifetimeErr( (float)lifetime_err );

	fData.AddOrReplaceRow(pd);
      }
    }

    return result;
  }
  
  const UbooneElectronLifetimeContainer& UbooneElectronLifetimeProvider::LifetimeContainer() const {     
    DBUpdate();
    return fData.GetRow(fLifetimeChannel);
  }
  
  float UbooneElectronLifetimeProvider::Lifetime() const {
    return this->LifetimeContainer().Lifetime();
  }
  
  float UbooneElectronLifetimeProvider::LifetimeErr() const {
    return this->LifetimeContainer().LifetimeErr();
  }

}//end namespace lariov
	
#endif
        
