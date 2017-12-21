#ifndef UBOONEDQDX1DCORRECTIONPROVIDER_CXX
#define UBOONEDQDX1DCORRECTIONPROVIDER_CXX

#include "Uboonedqdx1DCorrectionProvider.h"

// art/LArSoft libraries
#include "cetlib/exception.h"


#include <fstream>

namespace lariov {

  //constructor      
  Uboonedqdx1DCorrectionProvider::Uboonedqdx1DCorrectionProvider(fhicl::ParameterSet const& p) :
    DatabaseRetrievalAlg(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg")) {	
    
    this->Reconfigure(p);
  }
      
  void Uboonedqdx1DCorrectionProvider::Reconfigure(fhicl::ParameterSet const& p) {
    
    this->DatabaseRetrievalAlg::Reconfigure(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg"));
    fData.Clear();
    IOVTimeStamp tmp = IOVTimeStamp::MaxTimeStamp();
    tmp.SetStamp(tmp.Stamp()-1, tmp.SubStamp());
    fData.SetIoV(tmp, IOVTimeStamp::MaxTimeStamp());
    
    fCoord_min = 0.0;
    fCoord_max = 0.0;
    fIsFixedBinSize = true;
    fCoordToBinMap.clear();

    bool UseDB      = p.get<bool>("UseDB", false);
    bool UseFile    = p.get<bool>("UseFile", false);
    std::string fileName = p.get<std::string>("FileName", "");

    //priority:  (1) use db, (2) use table, (3) use defaults
    //If none are specified, use defaults
    if ( UseDB )      fDataSource = DataSource::Database;
    else if (UseFile) fDataSource = DataSource::File;
    else              fDataSource = DataSource::Default;

    if (fDataSource == DataSource::Default) {
      float default_correction     = p.get<float>("DefaultCorrection");
      float default_correction_err = p.get<float>("DefaultCorrectionErr");

      dqdx1DCorrection defaultCalib(0);
      defaultCalib.SetCorrection(default_correction);
      defaultCalib.SetCorrectionErr(default_correction_err);
      fData.AddOrReplaceRow(defaultCalib);
      
    }
    else if (fDataSource == DataSource::File) {
      cet::search_path sp("FW_SEARCH_PATH");
      std::string abs_fp = sp.find_file(fileName);
      std::cout << "Using dqdx corrections from local file: "<<abs_fp<<"\n";
      std::ifstream file(abs_fp);
      if (!file) {
        throw cet::exception("Uboonedqdx1DCorrectionProvider")
          << "File "<<abs_fp<<" is not found.";
      }
      
      std::string line;
      dqdx1DCorrection dp(0);
      float prev_lowedge = -99999999.9;
      int prev_bin = -1;
      float prev_diff = -1.0;
      int line_number = -1;
      while (std::getline(file, line)) {
        line_number++;
        size_t current_comma = line.find(',');
        DBChannelID_t bin = (DBChannelID_t)std::stoi(line.substr(0, current_comma));     
        float low_edge = std::stof(line.substr(current_comma+1, line.find(',',current_comma+1)));
        
        current_comma = line.find(',',current_comma+1);
        float high_edge = std::stof(line.substr(current_comma+1, line.find(',',current_comma+1)));
	
	current_comma = line.find(',',current_comma+1);
        float correction = std::stof(line.substr(current_comma+1, line.find(',',current_comma+1)));
	
	current_comma = line.find(',',current_comma+1);
        float correction_err = std::stof(line.substr(current_comma+1, line.find(',',current_comma+1)));

        dp.SetChannel(bin);
	dp.SetCorrection(correction);
        dp.SetCorrectionErr(correction_err);
        
        fData.AddOrReplaceRow(dp);
	
	if (low_edge < fCoord_min) fCoord_min = low_edge;
	if (high_edge > fCoord_max) fCoord_max = high_edge;
	
	//checks
	if (low_edge < prev_lowedge || (int)bin < prev_bin) {
	  throw cet::exception("Uboonedqdx1DCorrectionProvider")
	    << "Lines in file "<<abs_fp<<" are not in order of increasing coordinate values or bin number";
	}
	prev_lowedge = low_edge;
	prev_bin = bin;
	
	if (line_number != (int)bin) {
	  throw cet::exception("Uboonedqdx1DCorrectionProvider")
	    << "bins in file "<<abs_fp<<" do not start numbering at zero and/or increase by one";
	}
	
	//fill map of bin number to coordinate low edge
	fCoordToBinMap.push_back(low_edge);
	
	//is bin size fixed?
	float bin_size = high_edge - low_edge;
	if (prev_diff > 0.0 && (bin_size - prev_diff)/bin_size > 1.0e-6) {
	  fIsFixedBinSize = false;
	}
	prev_diff = bin_size;
      }
    }
    else {
      std::cout << "Using electronics calibrations from conditions database"<<std::endl;
    }
  }

  bool Uboonedqdx1DCorrectionProvider::Update(DBTimeStamp_t ts) {
    
    if (fDataSource != DataSource::Database) return false;
      
    if (!this->UpdateFolder(ts)) return false;

    //DBFolder was updated, so now update the Snapshot
    fData.Clear();
    fData.SetIoV(this->Begin(), this->End());

    std::vector<DBChannelID_t> bins;
    fFolder->GetChannelList(bins);
    
    //prep
    fCoord_min = 99999999.9;
    fCoord_max = 0.0;
    fIsFixedBinSize = true;
    fCoordToBinMap.clear();
    float prev_lowedge = -99999999.9;
    float prev_diff = -1;
    int row_number = -1;
    for (auto it = bins.begin(); it != bins.end(); ++it, ++row_number) {

      double low_edge, high_edge, correction, correction_err;
      fFolder->GetNamedChannelData(*it, "low_edge",       low_edge);
      fFolder->GetNamedChannelData(*it, "high_edge",      high_edge); 
      fFolder->GetNamedChannelData(*it, "correction",     correction);
      fFolder->GetNamedChannelData(*it, "correction_err", correction_err); 
      
      dqdx1DCorrection pg(*it);
      
      pg.SetCorrection( (float)correction );
      pg.SetCorrectionErr( (float)correction_err );

      fData.AddOrReplaceRow(pg);
      
      if (low_edge < fCoord_min) fCoord_min = low_edge;
      if (high_edge > fCoord_max) fCoord_max = high_edge;

      //checks
      if (low_edge < prev_lowedge || *it < *(it-1) ) {
	throw cet::exception("Uboonedqdx1DCorrectionProvider")
	  << "Database is not in order of increasing coordinate values or bin number";
      }
      prev_lowedge = low_edge;

      if (row_number != (int)(*it)) {
	throw cet::exception("Uboonedqdx1DCorrectionProvider")
	  << "bins in database do not start numbering at zero and/or increase by one";
      }

      //fill map of bin number to coordinate low edge
      fCoordToBinMap.push_back(low_edge);

      //is bin size fixed?
      float bin_size = high_edge - low_edge;
      if (prev_diff > 0.0 && (bin_size - prev_diff)/bin_size > 1.0e-6) {
	fIsFixedBinSize = false;
      }
      prev_diff = bin_size;
    }

    return true;
  }
  
  
  int Uboonedqdx1DCorrectionProvider::GetBinNumber(float coord) const {
    if (coord < fCoord_min || coord > fCoord_max) {
      throw cet::exception("Uboonedqdx1DCorrectionProvider")
        << "coordinate is out of bounds!";
    }
    
    if (fIsFixedBinSize) {
      return int( fCoordToBinMap.size()*(coord-fCoord_min)/(fCoord_max-fCoord_min) );
    }
    
    std::vector<float>::const_iterator it = std::lower_bound(fCoordToBinMap.begin(), fCoordToBinMap.end(), coord);
    if ( it==fCoordToBinMap.end() ) {
      throw cet::exception("Uboonedqdx1DCorrectionProvider")
        << "coordinate is out of search bounds!";
    }
    return std::distance(fCoordToBinMap.cbegin(),it);
  }
  
  
  const dqdx1DCorrection& Uboonedqdx1DCorrectionProvider::Dqdx1DCorrectionObject(float coord) const { 
    return fData.GetRow( this->GetBinNumber(coord) );
  }
 
      
  float Uboonedqdx1DCorrectionProvider::DqdxCorrection(float coord) const {
    return this->Dqdx1DCorrectionObject(coord).Correction();
  }
  
  
  float Uboonedqdx1DCorrectionProvider::DqdxCorrectionErr(float coord) const {
    return this->Dqdx1DCorrectionObject(coord).CorrectionErr();
  }
  
}//end namespace lariov
	
#endif
        
