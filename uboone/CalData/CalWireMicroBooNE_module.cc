////////////////////////////////////////////////////////////////////////
//
// CalWireMicroBooNE class
//
// brebel@fnal.gov
//
// 11-3-09 Pulled all FFT code out and put into Utilitiess/LArFFT
//
////////////////////////////////////////////////////////////////////////

// C/C++ standard libraries
#include <string>
#include <vector>
#include <utility> // std::move()
#include <memory> // std::unique_ptr<>
#include <stdint.h>

// ROOT libraries
#include "TComplex.h"
#include "TFFTComplex.h"
#include "TH1D.h"
#include "TH1F.h"

// framework libraries
#include "fhiclcpp/ParameterSet.h" 
#include "messagefacility/MessageLogger/MessageLogger.h" 
#include "art/Framework/Core/ModuleMacros.h" 
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h" 
#include "art/Framework/Principal/Handle.h" 
#include "art/Persistency/Common/Ptr.h" 
#include "art/Persistency/Common/Assns.h" 
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "art/Framework/Services/Optional/TFileService.h"

// LArSoft libraries
#include "SimpleTypesAndConstants/RawTypes.h" // raw::ChannelID_t
#include "Geometry/Geometry.h"
#include "Filters/ChannelFilter.h"
#include "RawData/RawDigit.h"
#include "RawData/raw.h"
#include "RecoBase/Wire.h"
#include "RecoBaseArt/WireCreator.h"
#include "Utilities/LArFFT.h"
#include "Utilities/AssociationUtil.h"
#include "uboone/Utilities/SignalShapingServiceMicroBooNE.h"
#include "CalibrationDBI/WebDBI/DetPedestalRetrievalAlg.h"

///creation of calibrated signals on wires
namespace caldata {

  class CalWireMicroBooNE : public art::EDProducer {

  public:
    
    // create calibrated signals on wires. this class runs 
    // an fft to remove the electronics shaping.     
    explicit CalWireMicroBooNE(fhicl::ParameterSet const& pset); 
    virtual ~CalWireMicroBooNE();
    
    void produce(art::Event& evt); 
    void beginJob();
    void endJob();
    void reconfigure(fhicl::ParameterSet const& p);
 
  private:
    
    int          fDataSize;          ///< size of raw data on one wire
    int          fPostsample;        ///< number of postsample bins
    int          fDoBaselineSub;        ///< do original baseline subtraction
    int          fDoSimpleBaselineSub;   ///< do simple baseline subtraction (M. Mooney)
    int          fDoAdaptiveBaselineSub;   ///< do adaptive baseline subtraction (M. Mooney)
    float        fBaseVarCut;        ///< baseline variance cut
    int          fSaveWireWF;        ///< Save recob::wire object waveforms
    std::string  fDigitModuleLabel;  ///< module that made digits
                                                       ///< constants
    std::string  fSpillName;  ///< nominal spill is an empty string
                              ///< it is set by the DigitModuleLabel
                              ///< ex.:  "daq:preSpill" for prespill data
    size_t fEventCount; ///< count of event processed

    int fBaselineWindowSize;   ///< window size for adaptive baseline method (M. Mooney)
    int fBaselineSigBinBuffer;   ///< bin buffer between baseline and nearest signal bin for adaptive baseline method (M. Mooney)
    double fBaselineSigThreshold;   ///< signal threshold for adaptive baseline method (M. Mooney)

    void SubtractBaseline(std::vector<float>& holder);
    void SubtractBaselineSimple(std::vector<float>& holder); // M. Mooney
    void SubtractBaselineAdaptive(std::vector<float>& holder); // M. Mooney
    std::vector<bool> FindSignalRegions(const std::vector<float>& deconvVec, int windowSize, int sigBinBuffer, double sigThreshold); // M. Mooney
    void RunAdaptiveBaselinePass(std::vector<float>& deconvVec, int runMode, int windowSize, int sigBinBuffer, double sigThreshold, const std::vector<bool>& signalRegions = std::vector<bool>()); // M. Mooney

    template <class T> void DeconvoluteInducedCharge(size_t firstChannel, std::vector<std::vector<T> >& signal) const; // M. Mooney

    lariov::DetPedestalRetrievalAlg fPedestalRetrievalAlg;


  protected: 
    
  }; // class CalWireMicroBooNE

  DEFINE_ART_MODULE(CalWireMicroBooNE)
  
  //-------------------------------------------------
  CalWireMicroBooNE::CalWireMicroBooNE(fhicl::ParameterSet const& pset) :
    fPedestalRetrievalAlg(pset.get<fhicl::ParameterSet>("DetPedestalRetrievalAlg"))
  {
    this->reconfigure(pset);

    produces< std::vector<recob::Wire> >(fSpillName);
    produces<art::Assns<raw::RawDigit, recob::Wire>>(fSpillName);
  } // CalWireMicroBooNE::CalWireMicroBooNE()
  
  //-------------------------------------------------
  CalWireMicroBooNE::~CalWireMicroBooNE()
  {
  }

  //////////////////////////////////////////////////////
  void CalWireMicroBooNE::reconfigure(fhicl::ParameterSet const& p)
  {
    fDigitModuleLabel = p.get< std::string >("DigitModuleLabel", "daq");
    fPostsample       = p.get< int >        ("PostsampleBins");
    fDoBaselineSub    = p.get< bool >       ("DoBaselineSub");
    fDoSimpleBaselineSub    = p.get< bool >       ("DoSimpleBaselineSub");
    fDoAdaptiveBaselineSub    = p.get< bool >       ("DoAdaptiveBaselineSub");
    fBaselineWindowSize   = p.get< int >        ("BaselineWindowSize");
    fBaselineSigBinBuffer   = p.get< int >        ("BaselineSigBinBuffer");
    fBaselineSigThreshold   = p.get< double >        ("BaselineSigThreshold");
    fBaseVarCut       = p.get< int >        ("BaseVarCut");
    fSaveWireWF       = p.get< int >        ("SaveWireWF");
    
    fSpillName.clear();
    
    fPedestalRetrievalAlg.Reconfigure(p.get<fhicl::ParameterSet>("DetPedestalRetrievalAlg"));
    
    size_t pos = fDigitModuleLabel.find(":");
    if( pos!=std::string::npos ) {
      fSpillName = fDigitModuleLabel.substr( pos+1 );
      fDigitModuleLabel = fDigitModuleLabel.substr( 0, pos );
    }
    
  }

  //-------------------------------------------------
  void CalWireMicroBooNE::beginJob()
  {  
    fEventCount = 0;
  }

  //////////////////////////////////////////////////////
  void CalWireMicroBooNE::endJob()
  {  
  }
  
  //////////////////////////////////////////////////////
  void CalWireMicroBooNE::produce(art::Event& evt)
  {      

    // update the database cache
    fPedestalRetrievalAlg.Update( lariov::IOVTimeStamp(evt) );

    // get the geometry
    art::ServiceHandle<geo::Geometry> geom;

    // get the FFT service to have access to the FFT size
    art::ServiceHandle<util::LArFFT> fFFT;
    int transformSize = fFFT->FFTSize();

    // Get signal shaping service.
    art::ServiceHandle<util::SignalShapingServiceMicroBooNE> sss;
    double DeconNorm = sss->GetDeconNorm();
    bool doInducedChargeDeconv = false;
    std::vector<std::vector<size_t> > respNums = sss->GetNResponses();
    for (size_t i = 0; i < respNums.at(1).size(); i++) {
      if (respNums.at(1).at(i) > 1) {
        doInducedChargeDeconv = true;
      }
    }

    // make a collection of Wires
    std::unique_ptr<std::vector<recob::Wire> > wirecol(new std::vector<recob::Wire>);
    // ... and an association set
    std::unique_ptr<art::Assns<raw::RawDigit,recob::Wire> > WireDigitAssn
      (new art::Assns<raw::RawDigit,recob::Wire>);
    
    // Read in the digit List object(s). 
    art::Handle< std::vector<raw::RawDigit> > digitVecHandle;
    if(fSpillName.size()>0) evt.getByLabel(fDigitModuleLabel, fSpillName, digitVecHandle);
    else evt.getByLabel(fDigitModuleLabel, digitVecHandle);

    if (!digitVecHandle->size()) {
      std::cout << "digitVecHandle Size is: " << digitVecHandle->size() << std::endl;
      return;
    }
    mf::LogInfo("CalWireMicroBooNE") << "CalWireMicroBooNE:: digitVecHandle size is " << digitVecHandle->size();

    // Use the handle to get a particular (0th) element of collection.
    art::Ptr<raw::RawDigit> digitVec0(digitVecHandle, 0);
        
    unsigned int dataSize = digitVec0->Samples(); //size of raw data vectors

    if( (unsigned int)transformSize < dataSize){
      mf::LogWarning("CalWireMicroBooNE")<<"FFT size (" << transformSize << ") "
					 << "is smaller than the data size (" << dataSize << ") "
					 << "\nResizing the FFT now...";
      fFFT->ReinitializeFFT(dataSize,fFFT->FFTOptions(),fFFT->FFTFitBins());
      transformSize = fFFT->FFTSize();
      mf::LogWarning("CalWireMicroBooNE")<<"FFT size is now (" << transformSize << ") "
					 << "and should be larger than the data size (" << dataSize << ")";
    }

    mf::LogInfo("CalWireMicroBooNE") << "Data size is " << dataSize << " and transform size is " << transformSize;

    //    if(fBaseSampleBins > 0 && dataSize % fBaseSampleBins != 0) {
    //  mf::LogError("CalWireMicroBooNE")<<"Set BaseSampleBins modulo dataSize= "<<dataSize;
    //}

    raw::ChannelID_t channel = raw::InvalidChannelID; // channel number
    unsigned int bin(0);     // time bin loop variable
    
    std::unique_ptr<filter::ChannelFilter> chanFilt(new filter::ChannelFilter());

    std::vector<float> holder;                // holds signal data
    std::vector<short> rawadc(transformSize);  // vector holding uncompressed adc values
    //std::vector<TComplex> freqHolder(transformSize+1); // temporary frequency data
    
    wirecol->reserve(digitVecHandle->size());

    if(!doInducedChargeDeconv) { ////////// Normal Deconvolution (No Induced Charge) //////////
      // loop over all wires
      for(size_t rdIter = 0; rdIter < digitVecHandle->size(); ++rdIter){ // ++ move
        holder.clear();
        
        // get the reference to the current raw::RawDigit
        art::Ptr<raw::RawDigit> digitVec(digitVecHandle, rdIter);
        channel = digitVec->Channel();
      
        // skip bad channels
        if(!chanFilt->BadChannel(channel)) {
      
          // resize and pad with zeros
      	  holder.resize(transformSize, 0.);
      	  
      	  // uncompress the data
      	  raw::Uncompress(digitVec->ADCs(), rawadc, digitVec->Compression());
      	  
      	  // loop over all adc values and subtract the pedestal
      	  // When we have a pedestal database, can provide the digit timestamp as the third argument of GetPedestalMean
          float pdstl = fPedestalRetrievalAlg.PedMean(channel);
      	  
      	  //David Caratelli
      	  //subtract time-offset added in SImWireMicroBooNE_module
      	  //Xin remove the time_offset
      	  int time_offset = 0;//sss->FieldResponseTOffset(channel);
      	  for(bin = 0; bin < dataSize; ++bin) {
      	    if ( (bin-time_offset >= 0) and (bin-time_offset < holder.size())  ) {
      	      holder[bin-time_offset]=(rawadc[bin]-pdstl);
	    }
      	  }
      	  //Xin fill the remaining bin with data
      	  for (bin = dataSize;bin<holder.size();bin++) {
      	    holder[bin] = (rawadc[bin-dataSize]-pdstl);
      	  }
          
      	  // Do deconvolution.
      	  sss->Deconvolute(channel, holder);
      	  for(bin = 0; bin < holder.size(); ++bin) {
            holder[bin]=holder[bin]/DeconNorm;
	  }
        } // end if not a bad channel 
        
        holder.resize(dataSize,1e-5);
      
        //normalize the holder (Xin Qian)
        
        //This restores the DC component to signal removed by the deconvolution.
        if(fPostsample) {
          float average=0.0;
      	  for(bin=0; bin < (unsigned short)fPostsample; ++bin) {
      	    average += holder[holder.size()-1+bin];
	  }

          average = average / (float)fPostsample;
          for(bin = 0; bin < holder.size(); ++bin) {
            holder[bin]-=average;
	  }
        }  
        // baseline subtraction
        if(fDoBaselineSub) SubtractBaseline(holder);
      
        // Make a single ROI that spans the entire data size
        wirecol->push_back(recob::WireCreator(holder,*digitVec).move());
        // add an association between the last object in wirecol
        // (that we just inserted) and digitVec
        if (!util::CreateAssn(*this, evt, *wirecol, digitVec, *WireDigitAssn, fSpillName)) {
          throw art::Exception(art::errors::InsertFailure)
            << "Can't associate wire #" << (wirecol->size() - 1)
            << " with raw digit #" << digitVec.key();
        } // if failed to add association
      }
    }
    else { ////////// Deconvolution with Induced Charge (M. Mooney) //////////

      std::vector<std::vector<float> > holders;                // holds signal data (for all wires of one plane)
      raw::ChannelID_t firstChannel; // first channel used to get plane

      for(size_t planeNum = 0; planeNum <= 2; planeNum++) { // loop over all planes (deconvolute one plane at a time)
        holders.clear();
        firstChannel = raw::InvalidChannelID;

        // loop over all wires
        for(size_t rdIter = 0; rdIter < digitVecHandle->size(); ++rdIter) { // ++ move
          holder.clear();

          // get the reference to the current raw::RawDigit
          art::Ptr<raw::RawDigit> digitVec(digitVecHandle, rdIter);
          channel = digitVec->Channel();

          // get plane number associated with channel and skip channel if not current plane being considered
	  auto view = (size_t)geom->View(channel);
          if(view != planeNum) {continue;}
          if(firstChannel == raw::InvalidChannelID) {firstChannel = channel;}

          // NB: should skip bad channels and factor this into deconvolution - add in later!
        
          // resize and pad with zeros
          holder.resize(transformSize, 0.);
        	
          // uncompress the data
          raw::Uncompress(digitVec->ADCs(), rawadc, digitVec->Compression());
        	
          // loop over all adc values and subtract the pedestal
          // When we have a pedestal database, can provide the digit timestamp as the third argument of GetPedestalMean
          float pdstl = fPedestalRetrievalAlg.PedMean(channel);
        	
          //David Caratelli
          //subtract time-offset added in SImWireMicroBooNE_module
          //Xin remove the time_offset
          int time_offset = 0;//sss->FieldResponseTOffset(channel);
          for(bin = 0; bin < dataSize; ++bin) {
            if ( (bin-time_offset >= 0) and (bin-time_offset < holder.size())  ) {
              holder[bin-time_offset]=(rawadc[bin]-pdstl);
            }
          }
          //Xin fill the remaining bin with data
          for (bin = dataSize;bin<holder.size();bin++) {
            holder[bin] = (rawadc[bin-dataSize]-pdstl);
          }

          // add processed raw digit to vector
          holders.push_back(holder);
        }

        fFFT->ReinitializeFFT(holders.size(),fFFT->FFTOptions(),fFFT->FFTFitBins());
        for(size_t i = holders.size(); i < (size_t)fFFT->FFTSize(); i++) {
          holders.push_back(holders.at(i-holders.size()));
	}

        // do induced charge deconvolution - M. Mooney
        DeconvoluteInducedCharge(firstChannel, holders);

        size_t counter = 0;
        for(size_t rdIter = 0; rdIter < digitVecHandle->size(); ++rdIter) { // ++ move
          // get the reference to the current raw::RawDigit
          art::Ptr<raw::RawDigit> digitVec(digitVecHandle, rdIter);
          channel = digitVec->Channel();

          // again, get plane number associated with channel and skip channel if not current plane being considered
	  auto view = (size_t)geom->View(channel);
          if(view != planeNum) {continue;}

          // NB: should skip bad channels again here

          holder = holders.at(counter);
          counter++;

          //normalize the holder (Xin Qian)
          double gain = sss->GetASICGain(channel);
          for(bin = 0; bin < holder.size(); ++bin) {
            holder[bin]=holder[bin]/DeconNorm/(gain/4.7);
          }
          
          holder.resize(dataSize,1e-5);
                  
          //This restores the DC component to signal removed by the deconvolution.
          if(fPostsample) {
            float average=0.0;
            for(bin=0; bin < (unsigned short)fPostsample; ++bin) {
              average += holder[holder.size()-1+bin];
            }
        
            average = average / (float)fPostsample;
            for(bin = 0; bin < holder.size(); ++bin) {
              holder[bin]-=average;
            }
          }  
          // original baseline subtraction
          if(fDoBaselineSub) SubtractBaseline(holder);
          // simple baseline subtraction - M. Mooney
          if(fDoSimpleBaselineSub) SubtractBaselineSimple(holder);
          // adaptive baseline subtraction - M. Mooney
          if(fDoAdaptiveBaselineSub) SubtractBaselineAdaptive(holder);
        
          // Make a single ROI that spans the entire data size
          wirecol->push_back(recob::WireCreator(holder,*digitVec).move());
          // add an association between the last object in wirecol
          // (that we just inserted) and digitVec
          if (!util::CreateAssn(*this, evt, *wirecol, digitVec, *WireDigitAssn, fSpillName)) {
            throw art::Exception(art::errors::InsertFailure)
              << "Can't associate wire #" << (wirecol->size() - 1)
              << " with raw digit #" << digitVec.key();
          } // if failed to add association
        }
      }
    }

    if(wirecol->size() == 0)
      mf::LogWarning("CalWireMicroBooNE") << "No wires made for this event.";


    //Make Histogram of recob::wire objects from Signal() vector
    // get access to the TFile service
    if ( fSaveWireWF ){
      art::ServiceHandle<art::TFileService> tfs;
      for (size_t wireN = 0; wireN < wirecol->size(); wireN++){
	std::vector<float> sigTMP = wirecol->at(wireN).Signal();
	TH1D* fWire = tfs->make<TH1D>(Form("Noise_Evt%04zu_N%04zu",fEventCount, wireN), ";Noise (ADC);",
				      sigTMP.size(),-0.5,sigTMP.size()-0.5);
	for (size_t tick = 0; tick < sigTMP.size(); tick++){
	  fWire->SetBinContent(tick+1, sigTMP.at(tick) );
	}
      }
    }

    evt.put(std::move(wirecol), fSpillName);
    evt.put(std::move(WireDigitAssn), fSpillName);
    
    fEventCount++;

    return;
  }
  
  void CalWireMicroBooNE::SubtractBaseline(std::vector<float>& holder)
  {
    float min = 0,max=0;
    for (unsigned int bin = 0; bin < holder.size(); bin++){
      if (holder[bin] > max) max = holder[bin];
      if (holder[bin] < min) min = holder[bin];
    }
    int nbin = max - min;
    if (nbin!=0){
      TH1F *h1 = new TH1F("h1","h1",nbin,min,max);
      for (unsigned int bin = 0; bin < holder.size(); bin++){
	h1->Fill(holder[bin]);
      }
      float ped = h1->GetMaximum();
      float ave=0,ncount = 0;
      for (unsigned int bin = 0; bin < holder.size(); bin++){
	if (fabs(holder[bin]-ped)<2){
	  ave +=holder[bin];
	  ncount ++;
	}
      }
      if (ncount==0) ncount=1;
      ave = ave/ncount;
      for (unsigned int bin = 0; bin < holder.size(); bin++){
	holder[bin] -= ave;
      }
      h1->Delete();
    }

    return;
  }

  void CalWireMicroBooNE::SubtractBaselineSimple(std::vector<float>& holder)
  {
    double sum = 0.0;
    for(size_t i = 0; i < holder.size(); i++) {
      sum += holder.at(i);
    }
    sum /= holder.size();

    for(size_t i = 0; i < holder.size(); i++) {
      holder.at(i) -= sum;
    }

    return;
  }

  void CalWireMicroBooNE::SubtractBaselineAdaptive(std::vector<float>& holder)
  {
    double sum = 0.0;
    for(size_t i = 0; i < holder.size(); i++) {
      sum += holder.at(i);
    }
    sum /= holder.size();

    for(size_t i = 0; i < holder.size(); i++) {
      holder.at(i) -= sum;
    }

    const std::vector<bool> signalRegions = FindSignalRegions(holder,fBaselineWindowSize,fBaselineSigBinBuffer,fBaselineSigThreshold);
    RunAdaptiveBaselinePass(holder,0,fBaselineWindowSize,fBaselineSigBinBuffer,fBaselineSigThreshold,signalRegions);
    RunAdaptiveBaselinePass(holder,1,fBaselineWindowSize,fBaselineSigBinBuffer,fBaselineSigThreshold);
    RunAdaptiveBaselinePass(holder,2,fBaselineWindowSize,fBaselineSigBinBuffer,fBaselineSigThreshold);

    return;
  }

  std::vector<bool> CalWireMicroBooNE::FindSignalRegions(const std::vector<float>& deconvVec, int windowSize, int sigBinBuffer, double sigThreshold)
  {
    int numBins = deconvVec.size();

    int window = TMath::Min(windowSize,numBins);
    window = TMath::Max(window,20);

    double baselineVec[numBins];
    double newValVec[numBins];

    double baselineVal = 0.0;
    int index;
    for(int j = -1*window/2; j <= window/2; j++) {
      if(j < 0)
        index = j+numBins;
      else if(j > numBins-1)
        index = j-numBins;
      else
        index = j;

      baselineVal += deconvVec[index];
    }

    baselineVec[0] = baselineVal/(window+1);
    newValVec[0] = deconvVec[0]-baselineVec[0];

    int oldIndex;
    int newIndex;
    for(int j = 1; j < numBins; j++){
      if(j-window/2-1 < 0)
        oldIndex = j-window/2-1+numBins;
      else
        oldIndex = j-window/2-1;

      if(j+window/2 > numBins-1)
        newIndex = j+window/2-numBins;
      else
        newIndex = j+window/2;

      baselineVal -= deconvVec[oldIndex];
      baselineVal += deconvVec[newIndex];

      baselineVec[j] = baselineVal/(window+1);
      newValVec[j] = deconvVec[j]-baselineVec[j];
    }

    std::vector<bool> isNearSigVec;

    bool baselineFlag;
    for(int j = 0; j < numBins; j++) {
      baselineFlag = false;
      for(int k = j-sigBinBuffer; k <= j+sigBinBuffer; k++) {
	if(k < 0)
          index = k+numBins;
        else if(k > numBins-1)
          index = k-numBins;
        else
          index = k;

        if(newValVec[index] > sigThreshold)
          baselineFlag = true;
      }
      isNearSigVec.push_back(baselineFlag);
    }

    return isNearSigVec;
  }

  void CalWireMicroBooNE::RunAdaptiveBaselinePass(std::vector<float>& deconvVec, int runMode, int windowSize, int sigBinBuffer, double sigThreshold, const std::vector<bool>& signalRegions)
  {
    if((runMode < 0) || (runMode > 2))
      return;

    int numBins = deconvVec.size();

    double baselineVec[numBins];
    bool isFilledVec[numBins];
    bool isNearSigVec[numBins];

    int index;

    if(signalRegions.size() == 0) {
      bool baselineFlag;
      for(int j = 0; j < numBins; j++) {
	baselineFlag = false;
	for(int k = j-sigBinBuffer; k <= j+sigBinBuffer; k++) {
	  if(k < 0)
	    index = k+numBins;
	  else if(k > numBins-1)
	     index = k-numBins;
	  else
	     index = k;

	  if(deconvVec[index] > sigThreshold)
	     baselineFlag = true;
	}
	isNearSigVec[j] = baselineFlag;
      }
    }
    else {
      for(int j = 0; j < numBins; j++) {
        isNearSigVec[j] = signalRegions[j];
      }
    }

    int window = TMath::Max(20,TMath::Min(windowSize,numBins));
    int minWindowBins = TMath::Max(10,TMath::Min(window/10,2*sigBinBuffer));
    if(runMode != 0) {
      int counter = 0;
      while(counter < numBins+minWindowBins) {
	if((isNearSigVec[counter % numBins] == true) && (isNearSigVec[(counter+1) % numBins] == false) && (isNearSigVec[(counter+minWindowBins) % numBins] == true)) {
	  for(int k = 1; k <= minWindowBins; k++) {
	    isNearSigVec[(counter+k) % numBins] = true;
       	  }
        }
        else {
          counter++;
	}
      }

      bool nearSigFlag = false;
      int nearSigCount = 0;
      int maxNearSig = 0;
      for(int j = 0; j < numBins; j++) {
	if(isNearSigVec[j] == true) {
	  if(nearSigFlag == false)
	    nearSigFlag = true;

	  nearSigCount++;
	}
        else {
          if(nearSigFlag == true) {
	    if(nearSigCount > maxNearSig)
	      maxNearSig = nearSigCount;

      	    nearSigFlag = false;
       	    nearSigCount = 0;
	  }
        }
      }

      int index = 0;
      while((nearSigFlag == true) && (index < numBins)) {
	if(isNearSigVec[index] == true) {
	  nearSigCount++;
	  index++;
	}
        else {
          nearSigFlag = false;
        }
      }
      int newWindowEstimate = TMath::Max(maxNearSig,nearSigCount);

      if((newWindowEstimate == 0) || (window < newWindowEstimate))
        return;

      if(runMode == 1)
	window = (window+newWindowEstimate)/2;
      else if(runMode == 2)
        window = newWindowEstimate;

      window = TMath::Max(20,TMath::Min(window,numBins));
      minWindowBins = TMath::Max(10,TMath::Min(window/10,2*sigBinBuffer));
    }

    double baselineVal = 0.0;
    int windowBins = 0;
    for(int j = -1*window/2; j <= window/2; j++) {
      if(j < 0)
        index = j+numBins;
      else if(j > numBins-1)
        index = j-numBins;
      else
        index = j;

      if(isNearSigVec[index] == false) {
        baselineVal += deconvVec[index];
        windowBins++;
      }
    }

    if(windowBins == 0)
      baselineVec[0] = 0.0;
    else
      baselineVec[0] = baselineVal/windowBins;

    if(windowBins < minWindowBins)
      isFilledVec[0] = false;
    else
      isFilledVec[0] = true;

    int oldIndex;
    int newIndex;
    for(int j = 1; j < numBins; j++) {
      if(j-window/2-1 < 0)
        oldIndex = j-window/2-1+numBins;
      else
        oldIndex = j-window/2-1;

      if(j+window/2 > numBins-1)
        newIndex = j+window/2-numBins;
      else
        newIndex = j+window/2;

      if(isNearSigVec[oldIndex] == false) {
	baselineVal -= deconvVec[oldIndex];
	windowBins--;
      }

      if(isNearSigVec[newIndex] == false)
      {
        baselineVal += deconvVec[newIndex];
        windowBins++;
      }

      if(windowBins == 0)
        baselineVec[j] = 0.0;
      else
        baselineVec[j] = baselineVal/windowBins;

      if(windowBins < minWindowBins)
        isFilledVec[j] = false;
      else
        isFilledVec[j] = true;
    }

    int downIndex;
    int upIndex;
    for(int j = 0; j < numBins; j++) {
      if(isFilledVec[j] == false) {
        downIndex = j;
	while(isFilledVec[downIndex] == false) {
	  downIndex--;

          if(downIndex < 0)
            downIndex = downIndex+numBins;
	}

	upIndex = j;
	while(isFilledVec[upIndex] == false) {
	  upIndex++;

	  if(upIndex > numBins-1)
	    upIndex = upIndex-numBins;
	}

	if((upIndex < j) && (downIndex > j))
	  baselineVec[j] = ((j-downIndex+numBins)*baselineVec[downIndex]+(upIndex+numBins-j)*baselineVec[upIndex])/((double)upIndex+numBins-downIndex+numBins);
        else if(upIndex < j)
          baselineVec[j] = ((j-downIndex)*baselineVec[downIndex]+(upIndex+numBins-j)*baselineVec[upIndex])/((double) upIndex+numBins-downIndex);
        else if(downIndex > j)
          baselineVec[j] = ((j-downIndex+numBins)*baselineVec[downIndex]+(upIndex-j)*baselineVec[upIndex])/((double) upIndex-downIndex+numBins);
        else
	  baselineVec[j] = ((j-downIndex)*baselineVec[downIndex]+(upIndex-j)*baselineVec[upIndex])/((double) upIndex-downIndex);
      }

      deconvVec[j] -= baselineVec[j];
    }
  }
}

template <class T> void caldata::CalWireMicroBooNE::DeconvoluteInducedCharge(size_t firstChannel, std::vector<std::vector<T> >& signal) const
{
  art::ServiceHandle<util::LArFFT> fft;
  art::ServiceHandle<util::SignalShapingServiceMicroBooNE> sss;
  art::ServiceHandle<geo::Geometry> geom;

  auto planeNum = (size_t)geom->View(firstChannel);

  int numBins = signal.at(0).size();
  int numWires = signal.size();
  int numResp = sss->GetNResponses().at(1).at(planeNum);

  std::vector<std::vector<TComplex> > signalFreqVecs;
  signalFreqVecs.resize(numWires);

  std::vector<std::vector<TComplex> > respFreqVecs;
  respFreqVecs.resize(numWires);

  std::vector<double> wireFactors;
  wireFactors.resize(numWires);

  fft->ReinitializeFFT(numBins,fft->FFTOptions(),fft->FFTFitBins());
      
  for(size_t k = 0; k < (size_t)numWires; k++) {
    signalFreqVecs[k].resize(numBins);
    fft->DoFFT(signal[k],signalFreqVecs[k]);
  }

  for(size_t k = 0; k < (size_t)numResp; k++) {
    respFreqVecs[numResp-1-k] = sss->GetConvKernel(firstChannel,k);
    respFreqVecs[numResp-1-k].resize(numBins);
    respFreqVecs[numResp-1+k].resize(numBins);
    for(size_t j = 1; j < (size_t)((numBins/2)+1); j++) {
      respFreqVecs[numResp-1-k][(numBins/2)+j] = respFreqVecs[numResp-1-k][(numBins/2)-j+1];
    }
    respFreqVecs[numResp-1+k] = respFreqVecs[numResp-1-k];
  }
  for(size_t k = 0; k < (size_t)numWires; k++) {
    if(k > (size_t)2*(numResp-1)) {
      respFreqVecs[k].resize(numBins);
    }
  }

  for(size_t k = 0; k < (size_t)numWires; k++) {
    wireFactors[k] = sss->Get2DFilterVal(planeNum,2,((double) k)/((double) numWires));
  }

  fft->ReinitializeFFT(numWires,fft->FFTOptions(),fft->FFTFitBins());

  TFFTComplex *fFFTComplex;
  fFFTComplex = new TFFTComplex(numWires,false);
  TFFTComplex *fInverseFFTComplex;
  fInverseFFTComplex = new TFFTComplex(numWires,false);
  int dummy[1] = {0};
  fFFTComplex->Init(fft->FFTOptions().c_str(),-1,dummy);
  fInverseFFTComplex->Init(fft->FFTOptions().c_str(),1,dummy);

  double normFactor = sss->Get2DFilterNorm(planeNum);

  std::vector<TComplex> signalFreqVecTemp;
  std::vector<TComplex> respFreqVecTemp;
  std::vector<TComplex> signalFreqVecTemp2;
  std::vector<TComplex> respFreqVecTemp2;
  std::vector<TComplex> resultFreqVecTemp;
  std::vector<TComplex> resultFreqVecTemp2;
  signalFreqVecTemp.resize(numWires);
  respFreqVecTemp.resize(numWires);
  signalFreqVecTemp2.resize(numWires);
  respFreqVecTemp2.resize(numWires);
  resultFreqVecTemp.resize(numWires);
  resultFreqVecTemp2.resize(numWires);
  for(size_t j = 0; j < (size_t)numBins; j++) {
    for(size_t k = 0; k < (size_t)numWires; k++) {
      signalFreqVecTemp[k] = signalFreqVecs[k][j];
      respFreqVecTemp[k] = respFreqVecs[k][j];
    }

    double real = 0.0;
    double imaginary = 0.0;
    
    for(size_t p = 0; p < (size_t)numWires; ++p) {
      fFFTComplex->SetPointComplex(p, signalFreqVecTemp[p]);
    }

    fFFTComplex->Transform();    

    for(size_t i = 0; i < (size_t)numWires; ++i) {
      fFFTComplex->GetPointComplex(i, real, imaginary);    
      signalFreqVecTemp2[i] = TComplex(real, imaginary);  
    }  

    for(size_t p = 0; p < (size_t)numWires; ++p) {
      fFFTComplex->SetPointComplex(p, respFreqVecTemp[p]);
    }

    fFFTComplex->Transform();    

    for(size_t i = 0; i < (size_t)numWires; ++i) {
      fFFTComplex->GetPointComplex(i, real, imaginary);    
      respFreqVecTemp2[i] = TComplex(real, imaginary);  
    }  

    double timeFactor = sss->Get2DFilterVal(planeNum,1,((double) j)/((double) numBins));
    for(size_t k = 0; k < (size_t)numWires; ++k) {
      double filterFactor = (timeFactor*wireFactors[k])/normFactor;
      resultFreqVecTemp[k] = TComplex(filterFactor*(signalFreqVecTemp2[k].Re()*respFreqVecTemp2[k].Re()+signalFreqVecTemp2[k].Im()*respFreqVecTemp2[k].Im())/(respFreqVecTemp2[k].Im()*respFreqVecTemp2[k].Im()+respFreqVecTemp2[k].Re()*respFreqVecTemp2[k].Re())/((double) numWires),filterFactor*(signalFreqVecTemp2[k].Im()*respFreqVecTemp2[k].Re()-signalFreqVecTemp2[k].Re()*respFreqVecTemp2[k].Im())/(respFreqVecTemp2[k].Im()*respFreqVecTemp2[k].Im()+respFreqVecTemp2[k].Re()*respFreqVecTemp2[k].Re())/((double) numWires));
    }

    for(size_t p = 0; p < (size_t)numWires; ++p) {
      fInverseFFTComplex->SetPointComplex(p, resultFreqVecTemp[p]);
    }

    fInverseFFTComplex->Transform();    

    for(size_t i = 0; i < (size_t)numWires; ++i) {
      fInverseFFTComplex->GetPointComplex(i, real, imaginary);    
      resultFreqVecTemp2[i] = TComplex(real,imaginary);
    }  

    int shift;
    for(size_t i = 0; i < (size_t)numWires; ++i) {
      shift = i-numResp-1;
      if(shift < 0)
        shift += numWires;

      signalFreqVecs[i][j] = TComplex(resultFreqVecTemp2[shift].Re()/((double) numBins),resultFreqVecTemp2[shift].Im()/((double) numBins));
    }
  }

  fft->ReinitializeFFT(numBins,fft->FFTOptions(),fft->FFTFitBins());

  int time_offset = sss->FieldResponseTOffset(firstChannel,1);
  for(size_t k = 0; k < (size_t)numWires; k++) {
    fft->DoInvFFT(signalFreqVecs[k],signal[k]);

    std::vector<T> temp;
    if (time_offset <= 0) {
      temp.assign(signal.at(k).end()+time_offset,signal.at(k).end());
      signal.at(k).erase(signal.at(k).end()+time_offset,signal.at(k).end());
      signal.at(k).insert(signal.at(k).begin(),temp.begin(),temp.end());
    }
    else {
      temp.assign(signal.at(k).begin(),signal.at(k).begin()+time_offset);
      signal.at(k).erase(signal.at(k).begin(),signal.at(k).begin()+time_offset);
      signal.at(k).insert(signal.at(k).end(),temp.begin(),temp.end());
    }
  }

  return;
}


   //  // subtract baseline using linear interpolation between regions defined
  //   // by the datasize and fBaseSampleBins

  //   // number of points to characterize the baseline
  //   unsigned short nBasePts = holder.size() / fBaseSampleBins;

  //   // the baseline offset vector
  //   std::vector<float> base;
  //   for(unsigned short ii = 0; ii < nBasePts; ++ii) base.push_back(0.);
  //   // find the average value in each region, using values that are
  //   // similar
  //   float fbins = fBaseSampleBins;
  //   unsigned short nfilld = 0;
  //   for(unsigned short ii = 0; ii < nBasePts; ++ii) {
  //     unsigned short loBin = ii * fBaseSampleBins;
  //     unsigned short hiBin = loBin + fBaseSampleBins;
  //     float ave = 0.;
  //     float sum = 0.;
  //     for(unsigned short bin = loBin; bin < hiBin; ++bin) {
  //       ave += holder[bin];
  //       sum += holder[bin] * holder[bin];
  //     } // jj
  //     ave = ave / fbins;
  //     float var = (sum - fbins * ave * ave) / (fbins - 1.);
  //     // Set the baseline for this region if the variance is small
  //     if(var < fBaseVarCut) {
  //       base[ii] = ave;
  //       ++nfilld;
  //     }
  //   } // ii
  //   // fill in any missing points if there aren't too many missing
  //   if(nfilld < nBasePts && nfilld > nBasePts / 2) {
  //     bool baseOK = true;
  //     // check the first region
  //     if(base[0] == 0) {
  //       unsigned short ii1 = 0;
  //       for(unsigned short ii = 1; ii < nBasePts; ++ii) {
  //         if(base[ii] != 0) {
  //           ii1 = ii;
  //           break;
  //         }
  //       } // ii
  //       unsigned short ii2 = 0;
  //       for(unsigned short ii = ii1 + 1; ii < nBasePts; ++ii) {
  //         if(base[ii] != 0) {
  //           ii2 = ii;
  //           break;
  //         }
  //       } // ii
  //       // failure
  //       if(ii2 > 0) {
  //         float slp = (base[ii2] - base[ii1]) / (float)(ii2 - ii1);
  //         base[0] = base[ii1] - slp * ii1;
  //       } else {
  //         baseOK = false;
  //       }
  //     } // base[0] == 0
  //     // check the last region
  //     if(baseOK && base[nBasePts] == 0) {
  //       unsigned short ii2 = 0;
  //       for(unsigned short ii = nBasePts - 1; ii > 0; --ii) {
  //         if(base[ii] != 0) {
  //           ii2 = ii;
  //           break;
  //         }
  //       } // ii
  //       baseOK = false; // assume the worst, hope for better
  //       unsigned short ii1 = 0;
  //       if (ii2 >= 1) {
  //         for(unsigned short ii = ii2 - 1; ii > 0; --ii) {
  //           if(base[ii] != 0) {
  //             ii1 = ii;
  //             baseOK = true;
  //             break;
  //           } // if base[ii]
  //         } // ii
  //       } // if ii2
  //       if (baseOK) {
  //         float slp = (base[ii2] - base[ii1]) / (float)(ii2 - ii1);
  //         base[nBasePts] = base[ii2] + slp * (nBasePts - ii2);
  //       }
  //     } // baseOK && base[nBasePts] == 0
  //     // now fill in any intermediate points
  //     for(unsigned short ii = 1; ii < nBasePts - 1; ++ii) {
  //       if(base[ii] == 0) {
  //         // find the next non-zero region
  //         for(unsigned short jj = ii + 1; jj < nBasePts; ++jj) {
  //           if(base[jj] != 0) {
  //             float slp = (base[jj] - base[ii - 1]) / (jj - ii + 1);
  //             base[ii] = base[ii - 1] + slp;
  //             break;
  //           }
  //         } // jj
  //       } // base[ii] == 0
  //     } // ii
  //   } // nfilld < nBasePts

  //   // interpolate and subtract
  //   float slp = (base[1] - base[0]) / (float)fBaseSampleBins;
  //   // bin offset to the origin (the center of the region)
  //   unsigned short bof = fBaseSampleBins / 2;
  //   unsigned short lastRegion = 0;
  //   for(unsigned short bin = 0; bin < holder.size(); ++bin) {
  //     // in a new region?
  //     unsigned short region = bin / fBaseSampleBins;
  //     if(region > lastRegion) {
  //       // update the slope and offset
  //       slp = (base[region] - base[lastRegion]) / (float)fBaseSampleBins;
  //       bof += fBaseSampleBins;
  //       lastRegion = region;
  //     }
  //     holder[bin] -= base[region] + (bin - bof) * slp;
  //   }
  // } // SubtractBaseline
  
// } // end namespace caldata
