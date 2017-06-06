
////////////////////////////////////////////////////////////////////////
/// \file   SignalShapingServiceMicroBooNE_service.cc
/// \author H. Greenlee
/// Modified by X. Qian 1/6/2015
/// if histogram is used, inialize
/// Response_Offset, Response_Sampling, FieldBins from histogram
////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "uboone/Utilities/SignalShapingServiceMicroBooNE.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"
#include "larcore/CoreUtils/ServiceUtil.h" // lar::providerFrom<>()
#include "art/Framework/Services/Optional/TFileService.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "lardata/DetectorInfoServices/DetectorClocksService.h"
#include "lardata/Utilities/LArFFT.h"
#include "larevt/CalibrationDBI/Interface/ElectronicsCalibService.h"
#include "larevt/CalibrationDBI/Interface/ElectronicsCalibProvider.h"
#include "TFile.h"

#include <fstream>

//----------------------------------------------------------------------
// Constructor.
util::SignalShapingServiceMicroBooNE::SignalShapingServiceMicroBooNE(const fhicl::ParameterSet& pset,
                                                                     art::ActivityRegistry& /* reg */)
: fInitForConvolution(false),
  fInitForDeconvolution(false)
{
  for(size_t i=0; i<3; ++i) {
    fHRawResponse[i] = 0;
    fHStretchedResponse[i] = 0;
    fHFullResponse[i] = 0;
    fHistDone[i] = false;
    fHistDoneF[i] = false;
  }
    
  reconfigure(pset);
}


//----------------------------------------------------------------------
// Destructor.
util::SignalShapingServiceMicroBooNE::~SignalShapingServiceMicroBooNE()
{}


//----------------------------------------------------------------------
// Reconfigure method.
void util::SignalShapingServiceMicroBooNE::reconfigure(const fhicl::ParameterSet& pset)
{
  //Services
  art::ServiceHandle<geo::Geometry> geo;
  size_t NViews = geo->Nplanes();
  
  auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
 
  // Reset initialization flags.
  fInitForConvolution = false;
  fInitForDeconvolution = false;
  
  
  //------------------------------------------------------------
  //Read most fcl parameters here
  //------------------------------------------------------------
  fSetDeconKernelsUponInit = pset.get<bool>("SetDeconKernelsUponInit");
  
  //Convolution- and deconvolution-related objects
  fDeconvPol          = pset.get<std::vector<int> >("DeconvPol");
  
  fSignalShapingVec.resize(geo->Nchannels());  
  for (unsigned int channel=0; channel!=geo->Nchannels(); ++channel) {
    fSignalShapingVec[channel].ResetAll();
  }  
  
  
  //Electronics Response-related parameters
  fADCPerPCAtLowestASICGain = pset.get<double>("ADCPerPCAtLowestASICGain");
  
  fElectResponse.resize(geo->Nchannels());
  
  
  //Field Response-related parameters
  fNResponses           = pset.get<std::vector<unsigned int> >("NResponses");
  fFieldRespAmpVec      = pset.get<DoubleVec>("FieldRespAmpVec");
  fYZdependentResponse  = pset.get<bool>("YZdependentResponse");
  fdatadrivenResponse   = pset.get<bool>("datadrivenResponse");
  fViewForNormalization = pset.get<size_t>("ViewForNormalization");

  fFieldResponseVec.resize(NViews);


  //Field Response time offset and stretching
  f3DCorrectionVec      = pset.get<DoubleVec>("Drift3DCorrVec");  
  fCalibResponseTOffset = pset.get< DoubleVec >("CalibResponseTOffset");
  fStretchFullResponse  = pset.get<bool>("StretchFullResponse");
  fTimeScaleParams      = pset.get<DoubleVec>("TimeScaleParams");	  
  fDefaultEField        = pset.get<double>("DefaultEField");	 	  
  fDefaultTemperature   = pset.get<double>("DefaultTemperature");	  


  //Noise
  fNoiseFactVec       = pset.get<DoubleVec2>("NoiseFactVec");
  fDeconNorm = pset.get<double>("DeconNorm");
  
  
  //Diagnostics
  fPrintResponses   = pset.get<bool>("PrintResponses");
  
 
  
  //------------------------------------------------------------
  //Filter function construction
  //------------------------------------------------------------
  
  // Construct parameterized collection filter function.
  fGetFilterFromHisto = pset.get<bool>("GetFilterFromHisto");  
  fFilterWidthCorrectionFactor = pset.get<DoubleVec>("FilterWidthCorrectionFactor", DoubleVec() = {1.0, 1.0, 1.0});   
  if(!fGetFilterFromHisto) {

    fFilterFuncVec.resize(NViews);
    mf::LogInfo("SignalShapingServiceMicroBooNE") << "Getting Filters from .fcl file" ;

    fFilterParamsVec = pset.get< DoubleVec2 >("FilterParamsVec");
    fFilterFuncVec = pset.get<std::vector<std::string> > ("FilterFuncVec");

    fFilterTF1Vec.resize(NViews);
    for(unsigned int _vw=0;_vw<NViews; ++_vw) {
      std::string name = Form("Filter_vw%02i_wr%02i", (int)_vw, 0);
      fFilterTF1Vec[_vw] = new TF1(name.c_str(), fFilterFuncVec[_vw].c_str() );
      for(unsigned int _ind=0; _ind<fFilterParamsVec[_vw].size(); ++_ind) {
        fFilterTF1Vec[_vw]->SetParameter(_ind, fFilterParamsVec[_vw][_ind]);
      }
    }
  } else {

    std::string histoname = pset.get<std::string>("FilterHistoName");
    mf::LogInfo("SignalShapingServiceMicroBooNE") << " using filter from .root file " ;

    // constructor decides if initialized value is a path or an environment variable
    std::string fname;
    cet::search_path sp("FW_SEARCH_PATH");
    sp.find_file(pset.get<std::string>("FilterFunctionFname"), fname);

    TFile * in=new TFile(fname.c_str(),"READ");
    for(unsigned int _vw=0;_vw<NViews;_vw++){
      std::string name = Form("%s_vw%02i", histoname.c_str(), (int)_vw);
      fFilterHistVec[_vw] = (TH1D *)in->Get(name.c_str());
    }

    in->Close();
    delete in;
  }
   
  // Load 2D filters for induced charge deconvolution (M. Mooney)
  fFilterFuncVecICTime.resize(NViews);
  fFilterFuncVecICWire.resize(NViews);
  mf::LogInfo("SignalShapingServiceMicroBooNE") << "Getting 2D Filters from .fcl file" ;

  DoubleVec2 paramsICTime = pset.get< DoubleVec2 >("FilterParamsVecICTime");
  fFilterFuncVecICTime = pset.get<std::vector<std::string> > ("FilterFuncVecICTime");
  
  DoubleVec2 paramsICWire = pset.get< DoubleVec2 >("FilterParamsVecICWire");
  fFilterFuncVecICWire = pset.get<std::vector<std::string> > ("FilterFuncVecICWire");

  fFilterTF1VecICTime.resize(NViews);
  fFilterICTimeMaxFreq.resize(NViews);
  fFilterICTimeMaxVal.resize(NViews);
  
  fFilterTF1VecICWire.resize(NViews);
  fFilterICWireMaxFreq.resize(NViews);
  fFilterICWireMaxVal.resize(NViews);
  for(unsigned int _vw=0;_vw<NViews; ++_vw) {
    std::string name = Form("FilterICTime_vw%02i_wr%02i", (int)_vw, 0);
    fFilterTF1VecICTime[_vw] = new TF1(name.c_str(), fFilterFuncVecICTime[_vw].c_str() );
    
    name = Form("FilterICWire_vw%02i_wr%02i", (int)_vw, 0);
    fFilterTF1VecICWire[_vw] = new TF1(name.c_str(), fFilterFuncVecICWire[_vw].c_str() );
    for(unsigned int _ind=0; _ind<paramsICTime[_vw].size(); ++_ind) {
      fFilterTF1VecICTime[_vw]->SetParameter(_ind, paramsICTime[_vw][_ind]);
      fFilterICTimeMaxFreq[_vw] = fFilterTF1VecICTime[_vw]->GetMaximumX();
      fFilterICTimeMaxVal[_vw] = fFilterTF1VecICTime[_vw]->GetMaximum();
    }
    for(unsigned int _ind=0; _ind<paramsICWire[_vw].size(); ++_ind) {
      fFilterTF1VecICWire[_vw]->SetParameter(_ind, paramsICWire[_vw][_ind]);
      fFilterICWireMaxFreq[_vw] = fFilterTF1VecICWire[_vw]->GetMaximumX();
      fFilterICWireMaxVal[_vw] = fFilterTF1VecICWire[_vw]->GetMaximum();
    }
  }
  
    
    
  //------------------------------------------------------------
  //Set fTimeScaleFactor - used to the generate field response
  //------------------------------------------------------------
  
  double defaultVelocity = detprop->DriftVelocity(fDefaultEField, fDefaultTemperature);
  double thisVelocity    = detprop->DriftVelocity( detprop->Efield(0), detprop->Temperature() );
  double vRatio = defaultVelocity/thisVelocity;
  double vDiff = vRatio -1.0;

  fTimeScaleFactor = 0.0;
  double term = 1.0;

  // the time scale params are from a fit to Garfield simulations at different E Fields
  for(size_t i = 0;i<fTimeScaleParams.size(); ++i) {
    fTimeScaleFactor += fTimeScaleParams[i]*term;
    term *= vDiff;
  }

  std::cout << "Current E field = " << detprop->Efield(0) << " KV/cm, Ratio of drift velocities = " << vRatio << ", timeScaleFactor = " << fTimeScaleFactor << std::endl;



  //------------------------------------------------------------
  //Fill field response histograms and determine time offsets
  //------------------------------------------------------------
  
  // constructor decides if initialized value is a path or an environment variable
  mf::LogInfo("SignalShapingServiceMicroBooNE") << " using the field response provided from a .root file " ;
  std::string fileNameBase = pset.get<std::string>("FieldResponseFNameBase"); 
  std::string version      = pset.get<std::string>("FieldResponseFVersion");
  std::string histNameBase = pset.get<std::string>("FieldResponseHNameBase");
  std::vector<std::string> ddr_resp_names = {"nominal", "shortedY", "shortedU"};
  cet::search_path sp("FW_SEARCH_PATH");
  
  // get the field responses
  fFieldResponseHistVec.resize(NViews);
  fFieldResponseTOffset.resize(NViews);
  for(unsigned int vw=0; vw!=NViews; ++vw) {

    std::string fname0, fname, response_name;     
    for (unsigned int rp=0; rp!=fNResponses[vw]; ++rp) {

      // open file with this response
      if (fdatadrivenResponse) {
	response_name = ddr_resp_names[rp];
	fname0 = Form("%s_vw%02i_%s_%s.root", fileNameBase.c_str(), vw, response_name.c_str(), version.c_str());	  
      }
      else {
	response_name =  ( rp == 0 ? "nominal" : "alt_" + std::to_string(rp) );
	fname0 = Form("%s_vw%02i_%s.root", fileNameBase.c_str(), vw, version.c_str());
      }  
      sp.find_file(fname0, fname);
      std::unique_ptr<TFile> fin(new TFile(fname.c_str(), "READ"));

      // store response histogram
      TString histName;
      if (fdatadrivenResponse) {
	histName = Form("%s_vw%02i_wr00", histNameBase.c_str(), vw);
      }
      else {
	histName = Form("%s_vw%02i_wr%02i", histNameBase.c_str(), vw, rp);
      }
      fFieldResponseHistVec[vw][response_name] = (TH1F*)fin->Get(histName);
      TH1F* resp = fFieldResponseHistVec[vw][response_name];
      fin->Close();

      // get the offsets for each plane... use wire 0 and either peak or zero-crossing
      double tOffset = 0.0;	
      if(rp==0 && vw==fViewForNormalization) { // this is for the standard response
	// for the collection plane, find the peak
	int binMax = resp->GetMaximumBin();
	tOffset = (resp->GetXaxis()->GetBinCenter(binMax) - resp->GetXaxis()->GetBinCenter(1));
	// for later, to be a bit cleverer, take weighted average of 3 bins
	//          for(int bin=binMax-1; bin<=binMax+1; ++ bin) {
	//            content = resp->GetBinContent(bin);
	//            binVal = resp->GetXaxis()->GetBinCenter(bin);
	//            numer += content*binVal;
	//            denom += content;
	//          }
	//          tOffset[_vw] = numer/denom*delta - resp->GetXaxis()->GetBinCenter(1);
      } else {
	// for the other planes, find the zero-crossing
	// lets find the minimum, and work backwards!

	int binMin = resp->GetMinimumBin();
	for(int bin=binMin;bin>0; --bin) {
	  double content = resp->GetBinContent(bin);
	  bool found = false;
	  if(content>0) {
            double binVal = resp->GetXaxis()->GetBinCenter(bin);
            tOffset = binVal - resp->GetXaxis()->GetBinCenter(1);
            found = true;
            // for later
            //            } else if (content>0) {
            //              // If it's already gone through zero, split the difference
            //              std::cout << resp->GetBinContent(bin) << " " << resp->GetBinContent(bin+1) << " " << bin << std::endl;
            //              binVal = resp->GetXaxis()->GetBinCenter(bin);
            //              numer = resp->GetBinContent(bin)*binVal - resp->GetBinContent(bin+1)*(binVal+delta);
            //              denom = resp->GetBinContent(bin) - resp->GetBinContent(bin+1);
            //              found = true;
	  }
	  if(found) break;
	}
      }

      tOffset *= f3DCorrectionVec[vw]*fTimeScaleFactor;
      fFieldResponseTOffset.at(vw) = (-tOffset + fCalibResponseTOffset[vw])*1000.;

    }//end loop over responses
  }//end loop over plane, done filling field response histograms

}


//----------------------------------------------------------------------
// Initialization method.
// Here we do initialization that can't be done in the constructor.
// All public methods should ensure that this method is called as necessary.
void util::SignalShapingServiceMicroBooNE::init()
{

  //do nothing if already initialized
  if (fInitForConvolution && fInitForDeconvolution) return;

  art::ServiceHandle<geo::Geometry> geo;

  // re-initialize the FFT service for the request size
  art::ServiceHandle<util::LArFFT> fFFT;
  int fftsize = (int) fFFT->FFTSize();
  
  
  //----------------------------------------------------------------------
  // first do convolution initialization
  //----------------------------------------------------------------------
  if (!fInitForConvolution) {

    // Calculate field responses first, so that the binning will be known to SetElectResponse
    SetFieldResponse();

    // Calculate electronic responses
    SetElectResponse();

    //Add the convolution responses to fSignalShapingVec
    for(unsigned int channel=0; channel!=geo->Nchannels(); ++channel) {
      size_t view = geo->View(channel);

      if ( (size_t)fFFT->FFTSize() < fElectResponse[channel].size()) {
	fFFT->ReinitializeFFT( fElectResponse.size(), fFFT->FFTOptions(), fFFT->FFTFitBins());
      }

      for (auto itResp = fFieldResponseVec[view].begin(); itResp != fFieldResponseVec[view].end(); ++itResp) {
	std::string resp_name = itResp->first;
	if ( StoreThisResponse(resp_name,channel) ) {
	  fSignalShapingVec[channel].Response(resp_name).AddResponseFunction(itResp->second);
	  fSignalShapingVec[channel].Response(resp_name).AddResponseFunction(fElectResponse[channel]);
	  fSignalShapingVec[channel].Response(resp_name).save_response();
	  fSignalShapingVec[channel].Response(resp_name).set_normflag(false); 
	}
      }
    }

    // see if we get the same toffsets
    SetResponseSampling();

    // Return to original fftsize
    //
    // Note: Currently we only have fine binning
    // for the field and electronic responses.
    // Now we are sampling the convoluted field-electronic response
    // with the nominal sampling.
    // We may consider to do the same for the filters as well.
    if ((int)fftsize!=fFFT->FFTSize()){
      fFFT->ReinitializeFFT( (size_t)fftsize, fFFT->FFTOptions(), fFFT->FFTFitBins());
    }
    
    //Print responses if desired
    if(fPrintResponses) {
      unsigned int print_channel = 0;
      size_t print_view = geo->View(print_channel);
      for(size_t i = 0; i<100; ++i) {
	std::cout << "Electronic Response for channel "<<print_channel <<": "<< fElectResponse[print_channel][i] << " " ;
	if((i+1)%10==0) std::cout << std::endl;
      }
      std::cout << std::endl;

      for (auto itResp = fFieldResponseVec[print_view].begin(); itResp != fFieldResponseVec[print_view].end(); ++itResp) {
	std::cout << "Input field response for view " << print_view << " response " << itResp->first
                  << ", " << itResp->second.size() << " bins" << std::endl;

	for(size_t i = 0; i<itResp->second.size(); ++i) {
	  std::cout << itResp->second[i] << " " ;
	  if((i+1)%10==0) std::cout << std::endl;
	}
	std::cout << std::endl;    
      }
    } 

    fInitForConvolution = true;
  } //end if !fInitForConvolution
  
  
  //----------------------------------------------------------------------
  // Now do deconvolution initialization
  //
  // If fSetDeconKernelsUponInit=false, you have to initialize the 
  // deconvolution kernels later using SetDecon() before doing deconvolutions
  //----------------------------------------------------------------------
  if (!fInitForDeconvolution && fSetDeconKernelsUponInit) {
    
    // Calculate filter functions.
    SetFilters();

    // Configure deconvolution kernels.
    if (fSetDeconKernelsUponInit) {
      for (unsigned int channel=0; channel!=geo->Nchannels(); ++channel) {
	size_t view = geo->View(channel);
	for (auto itResp = fFieldResponseVec[view].begin(); itResp != fFieldResponseVec[view].end(); ++itResp) {
	  std::string resp_name = itResp->first;
	  if ( StoreThisResponse(resp_name,channel) ) {
	    fSignalShapingVec[channel].Response(resp_name).AddFilterFunction(fFilterVec[view]);
	    fSignalShapingVec[channel].Response(resp_name).SetDeconvKernelPolarity( fDeconvPol.at(view) );
	    fSignalShapingVec[channel].Response(resp_name).CalculateDeconvKernel();
	  }
	}
      }
    }
    
    fInitForDeconvolution = true;
  } //end if !fInitForDeconvolution
   
} //end init()


//----------------------------------------------------------------------
// Set Deconvolution kernels using a characteristic data size
// This is needed to match deconvolution kernel sampling to size of data
//
// Does the following after calling init():
// 1) Reset all ktype=1 entries in fSignalShapingVec
// 2) Call SetResponseSampling for the given channel for each config, with mode=0 and ktype=1
// 3) Call SetFilters if config=0
// 4) Do:
//   4a) AddFilterFunction
//   4b) SetDeconvKernelPolarity
//   4c) CalculateDeconvKernel
void util::SignalShapingServiceMicroBooNE::SetDecon(size_t datasize)
{
  init();

  art::ServiceHandle<util::LArFFT> fft;  
  art::ServiceHandle<geo::Geometry> geo;

  // streamline this method:
  // if the deconvolution kernel is already appropriate for the datasize do nothing
  // otherwise, set it to the appropriate size
  // do this test for *every* ss
  // But it will in general only happen once per run! 
  size_t FFTSize = fft->FFTSize();
  bool changedFFTSize = false;
  if (datasize > FFTSize || datasize <= FFTSize/2){
    fft->ReinitializeFFT( datasize, fft->FFTOptions(), fft->FFTFitBins() );
    changedFFTSize = true;
  }  
  
  //do nothing if deconvolution kernels are already initialized and FFTSize is appropriate
  if (fInitForDeconvolution && !changedFFTSize) return;
   
  //reset deconvolution responses in fSignalShapingVec
  //Note that ResetAll() does not remove the saved response set when we called 
  //SignalShaping::save_response() on these objects.  This is intended, see SetResponseSampling()
  for (unsigned int channel=0; channel!=geo->Nchannels(); ++channel) {
    fSignalShapingVec[channel].ResetAll();
  }
  
  //Resample convolution responses and kernels in fSignalShapingVec, using the saved response
  //and the FFT Size
  SetResponseSampling();

  //Set Filter Function
  SetFilters();

  // Configure deconvolution kernels.
  for (unsigned int channel=0; channel!=geo->Nchannels(); ++channel) {
    size_t view = geo->View(channel);
    for (auto itResp = fFieldResponseVec[view].begin(); itResp != fFieldResponseVec[view].end(); ++itResp) {
      std::string resp_name = itResp->first;
      if ( StoreThisResponse(resp_name,channel) ) {
	fSignalShapingVec[channel].Response(resp_name).AddFilterFunction(fFilterVec[view]);
	fSignalShapingVec[channel].Response(resp_name).SetDeconvKernelPolarity( fDeconvPol.at(view));
	fSignalShapingVec[channel].Response(resp_name).CalculateDeconvKernel();
      }
    }
  }
  
  fInitForDeconvolution = true;
}




//----------------------------------------------------------------------
// Calculate microboone field response.
void util::SignalShapingServiceMicroBooNE::SetFieldResponse()
{

  art::ServiceHandle<art::TFileService> tfs; 
  
  art::ServiceHandle<geo::Geometry> geo;
  size_t NViews = geo->Nplanes();
  
  char buff0[80];

  // Ticks in nanosecond
  // Calculate the normalization of the collection plane
  std::string nominal_resp_name = "nominal";
  double integral = fFieldResponseHistVec[fViewForNormalization][nominal_resp_name]->Integral();
  double weight = 1./integral;

  // we adjust the size of the fieldresponse vector to account for the stretch
  // and interpolate the histogram to fill the vector with the stretched response        
  for(unsigned int _vw=0; _vw<NViews; ++_vw) {
    
    double timeFactor = f3DCorrectionVec[_vw];
    if(!fStretchFullResponse) timeFactor *= fTimeScaleFactor;

    for (auto itResp = fFieldResponseHistVec[_vw].begin(); itResp != fFieldResponseHistVec[_vw].end(); ++itResp) {
      
      std::string resp_name = itResp->first;
      TH1F* histPtr = fFieldResponseHistVec[_vw][resp_name];
      size_t nBins = histPtr->GetNbinsX();
      size_t nResponseBins = nBins*timeFactor;
      
      fFieldResponseVec[_vw][resp_name] = {0.0};
      DoubleVec* responsePtr = &fFieldResponseVec[_vw][resp_name];
      responsePtr->resize(nResponseBins);
      
      //fill response vector
      double x0 = histPtr->GetBinCenter(1);
      double xf = histPtr->GetBinCenter(nBins);
      double deltaX = (xf - x0)/(nBins-1);
      for(unsigned int _bn=1; _bn<=nResponseBins; ++_bn) {
	double xVal = x0 + deltaX*(_bn-1)/timeFactor;
	double yVal = histPtr->Interpolate(xVal);
	responsePtr->at(_bn-1) = yVal; 
	responsePtr->at(_bn-1) *= fFieldRespAmpVec[_vw]*weight;
      }
      
      // fill some histos
      if(itResp == fFieldResponseHistVec[_vw].begin() && !fHistDone[_vw]) {
	sprintf(buff0, "hRawResp%i", (int)_vw);
	fHRawResponse[_vw] = tfs->make<TH1D>(buff0, buff0, nBins, x0-0.5*deltaX, xf+0.5*deltaX);
	sprintf(buff0, "hStretchedResp%i", (int)_vw);
	double x0S = timeFactor*x0 - 0.5*deltaX/timeFactor;
	double xfS = timeFactor*xf + 0.5*deltaX/timeFactor;
	//std::cout << "title " << buff0 << std::endl;
	fHStretchedResponse[_vw] = tfs->make<TH1D>(buff0, buff0, nResponseBins, x0S, xfS);
	for(size_t i=0;i<nBins; ++i) {
	  fHRawResponse[_vw]->SetBinContent(i, histPtr->GetBinContent(i));
	}
	for(size_t i=0;i<nResponseBins; ++i) {
	  fHStretchedResponse[_vw]->SetBinContent(i+1, responsePtr->at(i));
	}
	fHistDone[_vw] = true;
      }
    }//end loop over Responses     
  }//end loop over views

  return;
}


//----------------------------------------------------------------------
// Calculate microboone field response.
void util::SignalShapingServiceMicroBooNE::SetElectResponse()
{
 
  LOG_DEBUG("SignalShapingMicroBooNE") << "Setting MicroBooNE electronics response function...";

  art::ServiceHandle<geo::Geometry> geo;

  const lariov::ElectronicsCalibProvider& elec_provider
    = art::ServiceHandle<lariov::ElectronicsCalibService>()->GetProvider();


  // The following sets the microboone electronics response function in
  // time-space. Function comes from BNL SPICE simulation of MicroBooNE
  // electronics. SPICE gives the electronics transfer function in
  // frequency-space. The inverse laplace transform of that function
  // (in time-space) was calculated in Mathematica and is what is being
  // used below. Parameters Ao and To are cumulative gain/timing parameters
  // from the full (ASIC->Intermediate amp->Receiver->ADC) electronics chain.
  // They have been adjusted to make the SPICE simulation to match the
  // actual electronics response. Default params are Ao=1.4, To=0.5us.


  // For the cold electronics,  the gain (i.e. 4.7 mV/fC) represents the peak 
  // height. The shaping time will not affect the peak height, but make the 
  // peak broader  

  std::string nominal_resp_name = "nominal";
  for (unsigned int channel=0; channel!=geo->Nchannels(); ++channel) {
    size_t view = geo->View(channel);
    unsigned int NFieldBins = fFieldResponseHistVec[view][nominal_resp_name]->GetXaxis()->GetNbins(); 
    fElectResponse[channel].resize( 4*NFieldBins,0.0);
    
    double FieldBinWidth = fFieldResponseHistVec[view][nominal_resp_name]->GetBinWidth(1);   
    double To = elec_provider.ShapingTime(channel);    
    double Ao = 1.0;
    double max = 0;
    for (unsigned int i=0; i < fElectResponse[channel].size(); ++i) {
      double timebin = (1.*i) * FieldBinWidth / To; 

      fElectResponse[channel][i] = 4.31054*exp(-2.94809*timebin) 
      -2.6202*exp(-2.82833*timebin)*cos(1.19361*timebin)
      -2.6202*exp(-2.82833*timebin)*cos(1.19361*timebin)*cos(2.38722*timebin)
      +0.464924*exp(-2.40318*timebin)*cos(2.5928*timebin)
      +0.464924*exp(-2.40318*timebin)*cos(2.5928*timebin)*cos(5.18561*timebin)
      +0.762456*exp(-2.82833*timebin)*sin(1.19361*timebin)
      -0.762456*exp(-2.82833*timebin)*cos(2.38722*timebin)*sin(1.19361*timebin)
      +0.762456*exp(-2.82833*timebin)*cos(1.19361*timebin)*sin(2.38722*timebin)
      -2.6202*exp(-2.82833*timebin)*sin(1.19361*timebin)*sin(2.38722*timebin)
      -0.327684*exp(-2.40318*timebin)*sin(2.5928*timebin)
      +0.327684*exp(-2.40318*timebin)*cos(5.18561*timebin)*sin(2.5928*timebin)
      -0.327684*exp(-2.40318*timebin)*cos(2.5928*timebin)*sin(5.18561*timebin)
      +0.464924*exp(-2.40318*timebin)*sin(2.5928*timebin)*sin(5.18561*timebin);
      
      fElectResponse[channel][i] *= Ao;

      if (fElectResponse[channel][i] > max) max = fElectResponse[channel][i];
    }// end loop over time buckets
    
    // normalize fElectResponse[i], before the convolution
    // Put in overall normalization in a pedantic way:
    // first put in the pulse area per eleectron at the lowest gain setting,
    // then normalize by the actual ASIC gain setting used.
    // This code is executed only during initialization of service,
    // so don't worry about code inefficiencies here.

    //Normalization are the following
    // Peak is firstly normalized to 1
    // thus we expect peak to be 1 * 9390 (fADCPerPCtAtLowestAsicGain) * 1.602e-7 * (1 fC) = 9.39 ADC
    // At 4.7 mV/fC, the ADC value should be 4.7 (mV/fC) * 2 (ADC/mV) ~ 9.4 ADC/fC
    // so the normalization are consistent
    double gain = elec_provider.Gain(channel);
    for(auto& element : fElectResponse[channel]){
      element /= max;
      element *= gain * fADCPerPCAtLowestASICGain * 1.60217657e-7 / 4.7;
    }
    
  }//end loop over channels

  LOG_DEBUG("SignalShapingMicroBooNE") << " Done.";

  return;
}


//----------------------------------------------------------------------
// Calculate microboone filter functions.
void util::SignalShapingServiceMicroBooNE::SetFilters()
{

  auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
  art::ServiceHandle<util::LArFFT> fft;
  
  art::ServiceHandle<geo::Geometry> geo;

  double ts = detprop->SamplingRate();
  size_t nFFT2 = fft->FFTSize() / 2;

  // Calculate collection filter.
  fFilterVec.resize(geo->Nplanes());
  for(auto& filter : fFilterVec) {
    filter.resize(nFFT2+1);
  }

  if(!fGetFilterFromHisto) {
    unsigned int _vw = 0;
    for(auto& func : fFilterTF1Vec) {
      func->SetRange(0, double(nFFT2));
      size_t count = 0;
      
      // now to scale the filter function!
      // only scale params 1,2 &3
      
      double timeFactor = fTimeScaleFactor*f3DCorrectionVec[_vw]*fFilterWidthCorrectionFactor[_vw];
      for(size_t i=1;i<4;++i) {
        func->SetParameter(i, fFilterParamsVec[_vw][i]/timeFactor);
      }
      
      for(unsigned int _bn=0; _bn<=nFFT2; ++_bn) {
        double freq = 500.*_bn/(ts*nFFT2);
        double f = func->Eval(freq);
        if(f!=0.0) count++;
        fFilterVec[_vw][_bn] = TComplex(f, 0.);
      }
      _vw++;
    }
  } 
  else {
    unsigned int _vw = 0;
    for(auto hist : fFilterHistVec) {
      for(unsigned int _bn=1; _bn<=nFFT2+1; ++_bn) {
        double f = hist->GetBinContent(_bn);
        fFilterVec[_vw][_bn-1] = TComplex(f, 0.);
      }
      _vw++;
    }
  }

}


//----------------------------------------------------------------------
// Sample microboone response (the convoluted field and electronic
// response), will probably add the filter later
void util::SignalShapingServiceMicroBooNE::SetResponseSampling()
{
  // Get services
  auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
  auto const* geo = lar::providerFrom<geo::Geometry>();
  art::ServiceHandle<util::LArFFT> fft;
  art::ServiceHandle<art::TFileService> tfs; 
  
  char buff0[80], buff1[80];


  // we want to implement new scheme (fStretchFullResponse==false) while retaining the old
  std::string nominal_resp_name = "nominal";
  size_t nticks = fft->FFTSize();
  for(size_t ch=0; ch<geo->Nchannels(); ++ch) {
  
    size_t view = geo->View(ch); 
    DoubleVec SamplingTime( nticks, 0. );
    double deltaInputTime = fFieldResponseHistVec[view][nominal_resp_name]->GetBinWidth(1)*1000.0;
    for ( size_t itime = 0; itime < nticks; ++itime ) {
      SamplingTime[itime] = (1.*itime) * detprop->SamplingRate();
    }
    
    double timeFactor = f3DCorrectionVec[view];
    double plotTimeFactor = 1.0;
    if(fStretchFullResponse) timeFactor *= fTimeScaleFactor;
    else plotTimeFactor = f3DCorrectionVec[view]*fTimeScaleFactor;
    
    for (auto itResp = fFieldResponseVec[view].begin(); itResp != fFieldResponseVec[view].end(); ++itResp) {
      std::string resp_name = itResp->first;
      if ( !StoreThisResponse(resp_name,ch) ) continue;
      
      const DoubleVec* pResp = &(fSignalShapingVec[ch].Response(resp_name).Response_save());
              
      if (!fHistDoneF[view] && resp_name == nominal_resp_name) {
        TH1F* raw_resp = fFieldResponseHistVec[view][resp_name];
      
        double xLowF = (raw_resp->GetBinCenter(1) - 0.5*raw_resp->GetBinWidth(1))*plotTimeFactor;
	double xHighF = xLowF + 0.001*(raw_resp->GetXaxis()->GetNbins()+1)*(raw_resp->GetBinWidth(1)*1000.0)*plotTimeFactor;
	double nBins = (raw_resp->GetXaxis()->GetNbins())*plotTimeFactor;
	sprintf(buff0, "FullResponse%i", (int)view);
	fHFullResponse[view] = tfs->make<TH1D>(buff0, buff0, nBins, xLowF, xHighF);                
	for (size_t i=0; i<nBins; ++i) {
	  fHFullResponse[view]->SetBinContent(i+1, pResp->at(i));
	}
      }
      
      size_t nticks_input = pResp->size();
      DoubleVec InputTime(nticks_input, 0. );
      for (size_t itime = 0; itime < nticks_input; itime++ ) {
	InputTime[itime] = (1.*itime) * deltaInputTime*timeFactor;
      }
      DoubleVec SamplingResp(nticks, 0. );
      size_t SamplingCount = 0;
      size_t startJ = 1;
      SamplingResp[0] = (*pResp)[0];
      for ( size_t itime = 1; itime < nticks; itime++ ) {
	size_t low, high;
	for ( size_t jtime = startJ; jtime < nticks_input; jtime++ ) {
	  if ( InputTime[jtime] >= SamplingTime[itime] ) {
	    low  = jtime - 1;
	    high = jtime;
	    double interpolationFactor = ((*pResp)[high]-(*pResp)[low])/deltaInputTime;
	    SamplingResp[itime] = ((*pResp)[low] + ( SamplingTime[itime] - InputTime[low] ) * interpolationFactor);
	    SamplingResp[itime] /= timeFactor;
	    SamplingCount++;
	    startJ = jtime;
	    break;
	  }
	} // for (  jtime = 0; jtime < nticks; jtime++ )
      } // for (  itime = 0; itime < nticks; itime++ )
           
      if(!fHistDoneF[view] && resp_name == nominal_resp_name) {
        TH1F* raw_resp = fFieldResponseHistVec[view][resp_name];
      
	double plotTimeFactor = f3DCorrectionVec[view]*fTimeScaleFactor;
	double xLowF = (raw_resp->GetBinCenter(1) - 0.5*raw_resp->GetBinWidth(1))*plotTimeFactor;
	double xHighF = xLowF + 0.001*(raw_resp->GetXaxis()->GetNbins()+1)*(raw_resp->GetBinWidth(1)*1000.0)*plotTimeFactor;
	double binWidth = 0.5;
	size_t nBins = (xHighF-xLowF+1)/binWidth;
	sprintf(buff1, "SampledResponse%i", (int)view);
	fHSampledResponse[view] = tfs->make<TH1D>(buff1, buff1, nBins, xLowF, xHighF);                
	for (size_t i=0; i<nBins; ++i) {
	  fHSampledResponse[view]->SetBinContent(i+1, SamplingResp[i]);
	}
	fHistDoneF[view] = true;
      }     
      
      fSignalShapingVec[ch].Response(resp_name).AddResponseFunction( SamplingResp, true);

    }  //  loop over responses
  } // loop over channels
  
  return;
}


double util::SignalShapingServiceMicroBooNE::GetRawNoise(unsigned int const channel) const
{
  init();

  art::ServiceHandle<geo::Geometry> geom;
  geo::View_t view = geom->View(channel);
  
  const lariov::ElectronicsCalibProvider& elec_provider
    = art::ServiceHandle<lariov::ElectronicsCalibService>()->GetProvider();
  
  unsigned int plane;
  switch(view){
    case geo::kU:
      plane = 0;
      break;
    case geo::kV:
      plane = 1;
      break;
    case geo::kZ:
      plane = 2;
      break;
    default:
      throw cet::exception(__FUNCTION__) << "Invalid geo::View_t ... " << view << std::endl;
  }

  double shapingtime = elec_provider.ShapingTime(channel);
  double gain = elec_provider.Gain(channel);
  int temp;
  if (std::abs(shapingtime - 0.5)<1e-6){
    temp = 0;
  }else if (std::abs(shapingtime - 1.0)<1e-6){
    temp = 1;
  }else if (std::abs(shapingtime - 2.0)<1e-6){
    temp = 2;
  }else{
    temp = 3;
  }

  auto tempNoise = fNoiseFactVec.at(plane);
  double rawNoise = tempNoise.at(temp);

  rawNoise *= gain/4.7;
  return rawNoise;
}


double util::SignalShapingServiceMicroBooNE::GetDeconNoise(unsigned int const channel) const
{ 

  const lariov::ElectronicsCalibProvider& elec_provider
    = art::ServiceHandle<lariov::ElectronicsCalibService>()->GetProvider();

  double deconNoise = this->GetRawNoise(channel);
  deconNoise *= 4.7/elec_provider.Gain(channel);
  deconNoise = deconNoise /4096.*2000./4.7 *6.241*1000/fDeconNorm;
  
  return deconNoise;
}


int util::SignalShapingServiceMicroBooNE::FieldResponseTOffset(unsigned int const channel) const
{
  //no init needed - fFieldResponseTOffset is initialized in reconfigure()
  
  art::ServiceHandle<geo::Geometry> geom;
  geo::View_t view = geom->View(channel);

  double time_offset = 0;
  switch(view){
    case geo::kU:
      time_offset = fFieldResponseTOffset.at(0);
      break;
    case geo::kV:
      time_offset = fFieldResponseTOffset.at(1);
      break;
    case geo::kZ: 
      time_offset = fFieldResponseTOffset.at(2); 
      break;
    default:
      throw cet::exception(__FUNCTION__) << "Invalid geo::View_t ... " << view << std::endl;
  }
  auto tpc_clock = lar::providerFrom<detinfo::DetectorClocksService>()->TPCClock();
  return tpc_clock.Ticks(time_offset/1.e3);
}

//----------------------------------------------------------------------
// Accessor for single-plane signal shaper.
const util::SignalShaping&
util::SignalShapingServiceMicroBooNE::SignalShaping(size_t channel, const std::string& response_name) const
{
  init(); 
  return fSignalShapingVec[channel].Response(response_name);
}


//----------------------------------------------------------------------
// Get convolution kernel from SignalShaping service for use in CalWire's
// DeconvoluteInducedCharge() - added by M. Mooney
const std::vector<TComplex>& util::SignalShapingServiceMicroBooNE::GetConvKernel(unsigned int channel, const std::string& response_name) const
{
  init();
  
  // Return appropriate shaper.
  return fSignalShapingVec[channel].Response(response_name).ConvKernel();
}


//----------------------------------------------------------------------
// Evaluate 2D filter used in induced charge deconvolution (M. Mooney)
double util::SignalShapingServiceMicroBooNE::Get2DFilterVal(size_t planeNum, size_t freqDimension, double binFrac) const
{
  auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
  double ts = detprop->SamplingRate();

  double freq;
  double filtVal;
  double val;
  if(freqDimension == 1) {
    freq = fFilterScaleVecICTime.at(planeNum)*(500.0/ts)*2.0*(0.5-fabs(binFrac-0.5));
    filtVal = fFilterTF1VecICTime.at(planeNum)->Eval(freq);

    if(freq < fFilterICTimeMaxFreq.at(planeNum))
      val = fFilterICTimeMaxVal.at(planeNum);
    else
      val = filtVal;

    return val;
  }
  else if(freqDimension == 2) {
    freq = fFilterScaleVecICWire.at(planeNum)*(0.5-fabs(binFrac-0.5));
    filtVal = fFilterTF1VecICWire.at(planeNum)->Eval(freq);

    if(freq < fFilterICWireMaxFreq.at(planeNum))
      val = fFilterICWireMaxVal.at(planeNum);
    else
      val = filtVal;

    return val;
  }
  else {
    return 0.0;
  }
}


//----------------------------------------------------------------------
// Return 2D filter normalization, used in induced charge deconvolution (M. Mooney)
double util::SignalShapingServiceMicroBooNE::Get2DFilterNorm(size_t planeNum) const
{
  return fFilterNormVecIC.at(planeNum);
}


//----------------------------------------------------------------------
// Get name of the response to use, as well as any scaling to be applied to the input signal (B. Eberly)
std::string util::SignalShapingServiceMicroBooNE::DetermineResponseName(unsigned int chan, double z, double y, double& charge_fraction) const
{
  
  //defaults
  std::string resp_name = "nominal";
  charge_fraction = 1.0;
  
  if (!fYZdependentResponse) return resp_name;

  art::ServiceHandle<geo::Geometry> geo;
  size_t view = (size_t)geo->View(chan);

  if (view==0) { //U-plane
    //Wires overlap with Y-plane shorted wires
    if ( IsYShortedOverlapChannel(chan) ) { 

      // YZ region overlaps with Y-plane shorted wires
      if ( IsYShortedZRegion(z) ){ 
	if (fdatadrivenResponse) resp_name = "shortedY";
	else if (fYZdependentResponse) charge_fraction = 0.98;
      }  
    }
  }


  if (view==1) { //V-plane
    //Wires overlap with U-plane shorted wires
    if (  IsUShortedOverlapChannel(chan) &&
         !IsYShortedOverlapChannel(chan) ) {

      //YZ region overlaps with U-plane shorted wires
      if ( IsUShortedYZRegion(y,z) ) {
	if (fdatadrivenResponse) resp_name = "shortedU";
	else if (fYZdependentResponse) {
	  charge_fraction = 0.685;
	}
      }   
      else {
        if (fYZdependentResponse) {
	  charge_fraction =  0.7;
	}
      }
    }

    //Wires overlap with U-plane AND Y-plane shorted wires
    else if ( IsUShortedOverlapChannel(chan) && 
              IsYShortedOverlapChannel(chan) ) {

      //YZ region overlaps with U-plane shorted wires
      if ( IsUShortedYZRegion(y,z) ) { 
        if (fdatadrivenResponse) resp_name = "shortedU";
	else if (fYZdependentResponse) {
	  charge_fraction = 0.685;
	}
      }

      //YZ region overlaps with Y-plane shorted wires
      else if ( IsYShortedZRegion(z) ) {
        if (fdatadrivenResponse) resp_name = "shortedY";
	else if (fYZdependentResponse) {
	  charge_fraction = 0.7; 
	  resp_name = "alt_1";
	}
      }

      //nominal
      else {
        if (fYZdependentResponse) {
	  charge_fraction = 0.7;
	}
      }
    }

    //Wires overlap with Y-plane shorted wires
    else if ( !IsUShortedOverlapChannel(chan) &&
               IsYShortedOverlapChannel(chan) ) {

      //YZ region overlaps with Y-plane shorted wires
      if ( IsYShortedZRegion(z) ) {
	if (fdatadrivenResponse) resp_name = "shortedY";
	else if (fYZdependentResponse) {
	  charge_fraction = 0.7; 
	  resp_name = "alt_1";
	}
      }

      //nominal
      else {
        if (fYZdependentResponse) {
	  charge_fraction = 0.7;
	}
      }
    }

    //No overlap with shorted wires
    else {
      if (fYZdependentResponse) {
	charge_fraction = 0.7;
      }
    }
  }//if view == 1


  if (view==2) { //Y-plane 
    //Wires overlap with U-plane shorted wires
    if ( IsUShortedOverlapChannel(chan) ) {

      //YZ region overlaps with U-plane shorted wires
      if ( IsUShortedYZRegion(y,z) ) {
	if (fdatadrivenResponse) resp_name = "shortedU";
	else if (fYZdependentResponse) charge_fraction = 0.8;
      }
    }
  }//if view == 2

  return resp_name;
}//end DDRName

bool util::SignalShapingServiceMicroBooNE::IsUShortedYZRegion(double y, double z) const {
  if ( ( (y < (z*0.577)+14.769)  && (y > (z*0.577)+14.422) )  || 
       ( (y < (z*0.577)+14.076)  && (y > (z*0.577)+7.840) )   || 
       ( (y < (z*0.577)+7.494)   && (y > (z*0.577)+7.148) )   || 
       ( (y < (z*0.577)+6.801)   && (y > (z*0.577)+3.683) )   || 
       ( (y < (z*0.577)+0.912)   && (y > (z*0.577)+0.219) )   || 
       ( (y < (z*0.577)-1.513)   && (y > (z*0.577)-2.552) )   || 
       ( (y < (z*0.577)-3.245)   && (y > (z*0.577)-4.630) )   || 
       ( (y < (z*0.577)-12.944)  && (y > (z*0.577)-21.604) )  || 
       ( (y < (z*0.577)-24.722)  && (y > (z*0.577)-37.193) )  || 
       ( (y < (z*0.577)-37.539)  && (y > (z*0.577)-50.703) )  || 
       ( (y < (z*0.577)-56.245)  && (y > (z*0.577)-57.284) )  || 
       ( (y < (z*0.577)-57.631)  && (y > (z*0.577)-63.174) )  || 
       ( (y < (z*0.577)-63.520)  && (y > (z*0.577)-64.559) )  || 
       ( (y < (z*0.577)-68.370)  && (y > (z*0.577)-76.684) )  || 
       ( (y < (z*0.577)-77.030)  && (y > (z*0.577)-88.115) )  || 
       ( (y < (z*0.577)-88.808)  && (y > (z*0.577)-90.194) )  || 
       ( (y < (z*0.577)-90.540)  && (y > (z*0.577)-101.971) ) || 
       ( (y < (z*0.577)-102.318) && (y > (z*0.577)-108.900) ) || 
       ( (y < (z*0.577)-109.246) && (y > (z*0.577)-109.592) ) || 
       ( (y < (z*0.577)-109.934) && (y > (z*0.577)-115.417) ) ) return true;
  else return false;
}

bool util::SignalShapingServiceMicroBooNE::IsYShortedZRegion(double z) const {
  if ( (z > 700.9 && z < 720.1) ||
       (z > 720.4 && z < 724.6) ||
       (z > 724.9 && z < 739.3) ) return true;
  else return false;
} 

bool util::SignalShapingServiceMicroBooNE::IsUShortedOverlapChannel(unsigned int channel) const {
  if ( (channel >= 2400 && channel <= 3743) ||
       (channel >= 4800 && channel <= 6143) ) return true;
  else return false;
}

bool util::SignalShapingServiceMicroBooNE::IsYShortedOverlapChannel(unsigned int channel) const {
  if ( (channel >= 1168 && channel <= 1903) ||
       (channel >= 3568 && channel <= 4303) ) return true;
  else return false;
}

bool util::SignalShapingServiceMicroBooNE::StoreThisResponse(const std::string& response_name, unsigned int channel) const {
  if (fdatadrivenResponse && fYZdependentResponse) {
    if (response_name=="shortedU") {
      if ( IsUShortedOverlapChannel(channel) ) return true;
      else return false;
    }
    else if (response_name=="shortedY") {
      if ( IsYShortedOverlapChannel(channel) ) return true;
      else return false;
    }
    else return true;
  }
  else if (fdatadrivenResponse && !fYZdependentResponse) {
    if (response_name !="nominal") return false;
    else return true;
  }
  else return true;
}

namespace util {
  
  DEFINE_ART_SERVICE(SignalShapingServiceMicroBooNE)
  
}
