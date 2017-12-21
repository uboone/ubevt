#ifndef UBOONETPCENERGYCALIBPROVIDER_CXX
#define UBOONETPCENERGYCALIBPROVIDER_CXX

#include "UbooneTPCEnergyCalibProvider.h"

// art/LArSoft libraries
#include "cetlib/exception.h"

#include <fstream>

namespace lariov {

  //constructor
  UbooneTPCEnergyCalibProvider::UbooneTPCEnergyCalibProvider(fhicl::ParameterSet const& p) : 
    fXProvider(p.get<fhicl::ParameterSet>("XCorrectionProvider")) {
    
    this->Reconfigure(p);
  }
  
  void UbooneTPCEnergyCalibProvider::Reconfigure(fhicl::ParameterSet const& p) {
    
    fXProvider.Reconfigure(p.get<fhicl::ParameterSet>("XCorrectionProvider"));
  }
  
  bool UbooneTPCEnergyCalibProvider::Update(DBTimeStamp_t ts) {
    return fXProvider.Update(ts);
  }
  
  float UbooneTPCEnergyCalibProvider::YZdqdxCorrection(float y, float z) const {
    std::cout<<"WARNING: YZ dqdx Correction Not implemented!"<<std::endl;
    return 1.0;
  }
  
  float UbooneTPCEnergyCalibProvider::YZdqdxCorrectionErr(float y, float z) const {
    std::cout<<"WARNING: YZ dqdx Correction Not implemented!"<<std::endl;
    return 0.0;
  }
  
  float UbooneTPCEnergyCalibProvider::DriftdqdxCorrection(float drift_coord) const {
    return fXProvider.DqdxCorrection(drift_coord);
  }
  
  float UbooneTPCEnergyCalibProvider::DriftdqdxCorrectionErr(float drift_coord) const {
    return fXProvider.DqdxCorrectionErr(drift_coord);
  }

  float UbooneTPCEnergyCalibProvider::XdqdxCorrection(float x) const {
    return fXProvider.DqdxCorrection(x);
  }
  
  float UbooneTPCEnergyCalibProvider::XdqdxCorrectionErr(float x) const {
    return fXProvider.DqdxCorrectionErr(x);
  }
  
  float UbooneTPCEnergyCalibProvider::YdqdxCorrection(float y) const {
    std::cout<<"WARNING: MicroBooNE dqdx calibration does not factorize into separate y and z dependencies"<<std::endl;
    return 1.0;
  }
  
  float UbooneTPCEnergyCalibProvider::YdqdxCorrectionErr(float y) const {
    std::cout<<"WARNING: MicroBooNE dqdx calibration does not factorize into separate y and z dependencies"<<std::endl;
    return 0.0;
  }
  
  float UbooneTPCEnergyCalibProvider::ZdqdxCorrection(float z) const {
    std::cout<<"WARNING: MicroBooNE dqdx calibration does not factorize into separate y and z dependencies"<<std::endl;
    return 1.0;
  }
  
  float UbooneTPCEnergyCalibProvider::ZdqdxCorrectionErr(float y) const {
    std::cout<<"WARNING: MicroBooNE dqdx calibration does not factorize into separate y and z dependencies"<<std::endl;
    return 0.0;
  }
    
  float UbooneTPCEnergyCalibProvider::ThetadqdxCorrection(float theta) const {
    std::cout<<"WARNING: theta dqdx Correction Not implemented!"<<std::endl;
    return 1.0;
  }
  
  float UbooneTPCEnergyCalibProvider::ThetadqdxCorrectionErr(float theta) const {
    std::cout<<"WARNING: theta dqdx Correction Not implemented!"<<std::endl;
    return 0.0;
  }
  
  float UbooneTPCEnergyCalibProvider::PhidqdxCorrection(float phi) const {
    std::cout<<"WARNING: MicroBooNE dqdx calibration does not have a phi dependence"<<std::endl;
    return 1.0;
  }
  
  float UbooneTPCEnergyCalibProvider::PhidqdxCorrectionErr(float phi) const {
    std::cout<<"WARNING: MicroBooNE dqdx calibration does not have a phi dependence"<<std::endl;
    return 0.0;
  }
  
  float UbooneTPCEnergyCalibProvider::TotaldqdxCorrection(float x, float y, float z, float theta, float phi /*=0.0*/) const {
    return ( this->XdqdxCorrection(x) )*( this->YZdqdxCorrection(y, z) )*( this->ThetadqdxCorrection(theta) );
  }
  
  float UbooneTPCEnergyCalibProvider::TotaldqdxCorrectionErr(float x, float y, float z, float theta, float phi /*=0.0*/) const {
    float x_frac_err =     ( this->XdqdxCorrectionErr(x) )         / ( this->XdqdxCorrection(x) );
    float yz_frac_err =    ( this->YZdqdxCorrectionErr(y, z) )     / ( this->YZdqdxCorrection(y, z) );
    float theta_frac_err = ( this->ThetadqdxCorrectionErr(theta) ) / ( this->ThetadqdxCorrection(theta) );
    
    
    return (this->TotaldqdxCorrection(x, y, z, theta, phi))*sqrt(x_frac_err*x_frac_err + yz_frac_err*yz_frac_err + theta_frac_err*theta_frac_err);
  }
  
  float UbooneTPCEnergyCalibProvider::dEdxCorrection(int plane) const {
    std::cout<<"WARNING: dEdx Correction Not implemented!"<<std::endl;
    return 1.0;
  }
  
  float UbooneTPCEnergyCalibProvider::dEdxCorrectionErr(int plane) const {
    std::cout<<"WARNING: dEdx Correction Not implemented!"<<std::endl;
    return 0.0;
  }

} //end namespace lariov

#endif
