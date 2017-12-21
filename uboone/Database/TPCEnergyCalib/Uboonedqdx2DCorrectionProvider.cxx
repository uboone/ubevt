#ifndef UBOONEDQDX2DCORRECTIONPROVIDER_CXX
#define UBOONEDQDX2DCORRECTIONPROVIDER_CXX

#include "Uboonedqdx2DCorrectionProvider.h"

// art/LArSoft libraries
#include "cetlib/exception.h"


#include <fstream>

namespace lariov {

  //constructor      
  Uboonedqdx2DCorrectionProvider::Uboonedqdx2DCorrectionProvider(fhicl::ParameterSet const& p) :
    DatabaseRetrievalAlg(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg")) {	
    
    this->Reconfigure(p);
  }
      
  void Uboonedqdx2DCorrectionProvider::Reconfigure(fhicl::ParameterSet const& p) {
    
    this->DatabaseRetrievalAlg::Reconfigure(p.get<fhicl::ParameterSet>("DatabaseRetrievalAlg"));
    fData.Clear();
    IOVTimeStamp tmp = IOVTimeStamp::MaxTimeStamp();
    tmp.SetStamp(tmp.Stamp()-1, tmp.SubStamp());
    fData.SetIoV(tmp, IOVTimeStamp::MaxTimeStamp());
    
    fCoord_min.clear();
    fCoord_max.clear();
    fNbins.clear();
    fBinEdgeNames.clear();
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
        throw cet::exception("UboonedqdxCorrectionProvider")
          << "File "<<abs_fp<<" is not found.";
      }
      
      std::string line;
      dqdx1DCorrection dp(0);
      std::vector<float> prev_lowedge;
      std::vector<float> prev_highedge;
      int prev_bin = -1;
      std::vector<float> prev_diff;
      std::vector<bool> stop_incr_bins;
      int line_number = -1;
      unsigned int NDimensions = 0;
      while (std::getline(file, line)) {
        line_number++;
	if (line_number==0) {
	  NDimensions = std::count(line.begin(), line.end(), ',') - 2;
	  fCoord_min.resize(NDimensions);
          fCoord_max.resize(NDimensions);
	  fNbins.resize(NDimensions,0);
	  stop_incr_bins.resize(NDimensions, false);
	  prev_lowedge.resize(NDimensions);
	  prev_highedge.resize(NDimensions);
	  prev_diff.resize(NDimensions);
	}
	
	//get bin number, make sure it is reasonable
        size_t current_comma = line.find(',');
        DBChannelID_t bin = (DBChannelID_t)std::stoi(line.substr(0, current_comma)); 
	if ( (int)bin != prev_bin+1 || (int)bin != line_number) {
	  throw cet::exception("UboonedqdxCorrectionProvider")
	    << "Bins in file "<<abs_fp<<" are not in order of increasing bin number or do not start at zero";
	}
	prev_bin = bin;
	
	
	    
	//get the bin low and high edges
	std::vector<float> low_edge;
	std::vector<float> high_edge;
	for (unsigned int n_dim = 0; n_dim != NDimensions; ++n_dim) {
          low_edge.push_back( std::stof(line.substr(current_comma+1, line.find(',',current_comma+1))) );        
          current_comma = line.find(',',current_comma+1);
          
	  high_edge.push_back( std::stof(line.substr(current_comma+1, line.find(',',current_comma+1))) );
	  current_comma = line.find(',',current_comma+1);
	  
	  //sanity check - the first dimension is always incrementing
	  if (line_number!=0 && n_dim==0 && low_edge[n_dim] < prev_lowedge[n_dim]) {
	    throw cet::exception("UboonedqdxCorrectionProvider")
	      << "Lines in file "<<abs_fp<<" are not in order of increasing coordinate values";
	  }
	  
	  //stop counting bins for this dimension?
	  if (line_number!= 0 && n_dim!=0 && low_edge[n_dim] < prev_lowedge[n_dim]) {
	    stop_incr_bins[n_dim] = true;
	  }
	  
	  //increment bin count?
	  if (!stop_incr_bins[n_dim] && low_edge[n_dim]!=prev_lowedge[n_dim]) fNbins[n_dim]++;
	  
	  //is the bin size fixed?
	  float bin_size = high_edge[n_dim] - low_edge[n_dim];
	  if (bin_size != 0.0) {
	    if (line_number!=0 && (bin_size - prev_diff[n_dim])/bin_size > 1.0e-6) {
	      fIsFixedBinSize = false;
	    }
	    //are the bin edges consistent?
	    if (line_number!=0 && low_edge[n_dim] < prev_lowedge[n_dim] && ( low_edge[n_dim] != fCoord_min[n_dim] || prev_highedge[n_dim] != fCoord_max[n_dim] ) ) {
	      fIsFixedBinSize = false;
	    }
	  }

	  //update cached values
	  prev_lowedge[n_dim] = low_edge[n_dim];
	  prev_highedge[n_dim] = high_edge[n_dim];	  
	  prev_diff[n_dim] = bin_size;
	  
	  //update min and max bins
	  if (line_number!=0) {
	    if (low_edge[n_dim] < fCoord_min[n_dim]) fCoord_min[n_dim] = low_edge[n_dim];
	    if (high_edge[n_dim] > fCoord_max[n_dim]) fCoord_max[n_dim] = high_edge[n_dim];
	  }
	  else {
	    fCoord_min[n_dim] = low_edge[n_dim];
	    fCoord_max[n_dim] = high_edge[n_dim];
	  }
	  	  
	}//end for loop over n dimensions
	
		
	//get the coorection and its error, make sure the number of columns hasn't changed
        float correction = std::stof(line.substr(current_comma+1, line.find(',',current_comma+1)));	
	current_comma = line.find(',',current_comma+1);
        float correction_err = std::stof(line.substr(current_comma+1, line.find(',',current_comma+1)));
	if ( current_comma == std::string::npos || current_comma != line.find_last_of(',') ) {
	  throw cet::exception("UboonedqdxCorrectionProvider")
          << "Number of columns in calibration file do not match n dimensions";
        }


        //Add calibration data to fData
        dp.SetChannel(bin);
	dp.SetCorrection(correction);
        dp.SetCorrectionErr(correction_err);
        
        fData.AddOrReplaceRow(dp);
	

	//fill map of bin number to coordinate low edge
	fCoordToBinMap.push_back(low_edge);
      }//end loop over file lines
    }
    else {
      std::cout << "Using electronics calibrations from conditions database"<<std::endl;     
      fBinEdgeNames = p.get<std::vector<std::string> >("BinEdgeNames");
    }
  }

  bool Uboonedqdx2DCorrectionProvider::Update(DBTimeStamp_t ts) {
    
    if (fDataSource != DataSource::Database) return false;
      
    if (!this->UpdateFolder(ts)) return false;

    //DBFolder was updated, so now update the Snapshot
    fData.Clear();
    fData.SetIoV(this->Begin(), this->End());

    std::vector<DBChannelID_t> bins;
    fFolder->GetChannelList(bins);
    
    //prep
    fCoord_min.resize(fBinEdgeNames.size());
    fCoord_max.resize(fBinEdgeNames.size());
    fNbins.resize(fBinEdgeNames.size(),0);
    fIsFixedBinSize = true;
    fCoordToBinMap.clear();
    std::vector<float> prev_lowedge(fBinEdgeNames.size());
    std::vector<float> prev_highedge(fBinEdgeNames.size());
    std::vector<float> prev_diff(fBinEdgeNames.size());
    std::vector<bool>  stop_incr_bins(fBinEdgeNames.size(), false);
    int row_number = -1;   
    for (auto it = bins.begin(); it != bins.end(); ++it, ++row_number) {

      std::vector<float> low_edge(fBinEdgeNames.size());
      std::vector<float> high_edge(fBinEdgeNames.size());
      double correction, correction_err;
      for (unsigned int n_dim = 0; n_dim != fBinEdgeNames.size(); ++n_dim) {
      
        std::string low_edge_str = fBinEdgeNames[n_dim]+"_low_edge";
	std::string high_edge_str = fBinEdgeNames[n_dim]+"_high_edge";
	double le, he;
        fFolder->GetNamedChannelData(*it, low_edge_str,   le);
        fFolder->GetNamedChannelData(*it, high_edge_str,  he); 
	low_edge[n_dim] = (float)le;
	high_edge[n_dim] = (float)he;
	
	if (it != bins.begin()) {
	  if (low_edge[n_dim] <  fCoord_min[n_dim]) fCoord_min[n_dim] = low_edge[n_dim];
	  if (high_edge[n_dim] > fCoord_max[n_dim]) fCoord_max[n_dim] = high_edge[n_dim];
	}
	else {
	  fCoord_min[n_dim] = low_edge[n_dim];
	  fCoord_max[n_dim] = high_edge[n_dim];
	}
	
	//checks
	if (it != bins.begin() && low_edge[n_dim] < prev_lowedge[n_dim]) {
	  throw cet::exception("UboonedqdxCorrectionProvider")
	    << "Database is not in order of increasing coordinate values";
	}
	prev_lowedge[n_dim] = low_edge[n_dim];
	
	//is bin size fixed?
	float bin_size = high_edge[n_dim] - low_edge[n_dim];
	if (it != bins.begin() && (bin_size - prev_diff[n_dim])/bin_size > 1.0e-6) {
	  fIsFixedBinSize = false;
	}
	prev_diff[n_dim] = bin_size;
      }
      
      fFolder->GetNamedChannelData(*it, "correction",     correction);
      fFolder->GetNamedChannelData(*it, "correction_err", correction_err); 
      
      dqdx1DCorrection pg(*it);
      
      pg.SetCorrection( (float)correction );
      pg.SetCorrectionErr( (float)correction_err );

      fData.AddOrReplaceRow(pg);
      
      //checks
      if (*it < *(it-1) || row_number != (int)(*it) ) {
	throw cet::exception("UboonedqdxCorrectionProvider")
	  << "bins in database do not start numbering at zero and/or increase by one";
      }

      //fill map of bin number to coordinate low edge
      fCoordToBinMap.push_back(low_edge);
    }

    return true;
  }
  
  //returns true if each element of v1 is less than or equal to its corresponding element in v2
  bool Uboonedqdx2DCorrectionProvider::CompareFunction(const std::vector<float>& v1, const std::vector<float>& v2){
    if (v2.empty()) return false;
    if (v1.empty()) return true;
    
    std::vector<float> const_iterator it1 = v1.cbegin();
    std::vector<float> const_iterator it2 = v2.cbegin();
    
    std::vector<float> const_iterator end1 = v1.cend();
    std::vector<float> const_iterator end2 = v2.cend();
    
    while (it1!=end1) {
      if (it2 == end2) return true;
      if (*it1 > *it2) return false;
      ++it1; ++it2;
    }
    return true;
  }
    
  
  int Uboonedqdx2DCorrectionProvider::GetBinNumber(const std::vector<float>& coord) const {
    if (coord.size() != fCoord_min.size()) {
      throw cet::exception("UboonedqdxCorrectionProvider")
        << "number of input coordinate dimensions doesn't match initialization";
    }
    
    for (unsigned int n_dim=0; n_dim != coord.size(); ++n_dim) {
      if (coord[n_dim] < fCoord_min[n_dim] || coord[n_dim] > fCoord_max[n_dim]) {
	throw cet::exception("UboonedqdxCorrectionProvider")
          << "coordinate #"<<n_dim<<" is out of bounds!";
      }
    }
    
    if (fIsFixedBinSize) {
      
    
    
      return int( fCoordToBinMap.size()*(coord-fCoord_min)/(fCoord_max-fCoord_min) );
    }
    
    std::vector<float>::const_iterator it = std::lower_bound(fCoordToBinMap.begin(), fCoordToBinMap.end(), coord, Uboonedqdx2DCorrectionProvider::CompareFunction);
    if ( it==fCoordToBinMap.end() ) {
      throw cet::exception("UboonedqdxCorrectionProvider")
        << "coordinate is out of search bounds!";
    }
    return std::distance(fCoordToBinMap.cbegin(),it);
    return 0;
  }
  
  
  const dqdx1DCorrection& Uboonedqdx2DCorrectionProvider::Dqdx2DCorrectionObject(const std::vector<float>& coord) const { 
    return fData.GetRow( this->GetBinNumber(coord) );
  }
 
      
  float Uboonedqdx2DCorrectionProvider::DqdxCorrection(const std::vector<float>& coord) const {
    return this->Dqdx2DCorrectionObject(coord).Correction();
  }
  
  
  float Uboonedqdx2DCorrectionProvider::DqdxCorrectionErr(const std::vector<float>& coord) const {
    return this->Dqdx2DCorrectionObject(coord).CorrectionErr();
  }
  
}//end namespace lariov
	
#endif
        
