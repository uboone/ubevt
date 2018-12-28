////////////////////////////////////////////////////////////////////////
// \file SpaceChargeMicroBooNE.cxx
//
// \brief implementation of class for storing/accessing space charge distortions for MicroBooNE
//
// \author mrmooney@bnl.gov
// 
////////////////////////////////////////////////////////////////////////

// C++ language includes
#include <string>
#include <vector>
#include <cassert>

// LArSoft includes
#include "ubevt/SpaceCharge/SpaceChargeMicroBooNE.h"
#include "lardataalg/DetectorInfo/DetectorProperties.h"

// Framework includes
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// ROOT includes
#include "TFile.h"
#include "TH3.h"
#include "TTree.h"
#include "TLeaf.h"


namespace {
  
  /// TGraph wrapper with RAII.
  class SortedTGraphFromFile {
    TGraph* graph = nullptr;
    
      public:
    SortedTGraphFromFile(TFile& file, char const* graphName)
      : graph(static_cast<TGraph*>(file.Get(graphName)))
      {
        if (!graph) {
          throw cet::exception("SpaceChargeMicroBooNE")
            << "Graph '" << graphName << "' not found in '" << file.GetPath()
            << "'\n";
        }
        graph->Sort();
      }
    
    ~SortedTGraphFromFile() { delete graph; }
    
    TGraph const& operator* () const { return *graph; }
    
  }; // class SortedTGraphFromFile
  
} // local namespace


//-----------------------------------------------
spacecharge::SpaceChargeMicroBooNE::SpaceChargeMicroBooNE(
  fhicl::ParameterSet const& pset
)
{
  //Configure(pset);
}

//-----------------------------------------------
spacecharge::SpaceChargeMicroBooNE::SpaceChargeMicroBooNE
  (fhicl::ParameterSet const& pset, providers_type providers)
{
  Configure(pset, providers.get<detinfo::DetectorProperties>());
}

//------------------------------------------------
bool spacecharge::SpaceChargeMicroBooNE::Configure(fhicl::ParameterSet const& pset, 
						   detinfo::DetectorProperties const* detprop)
{  
  assert(detprop); // detector properties service provider is required
  
  fEnableSimSpatialSCE = pset.get<bool>("EnableSimSpatialSCE");
  fEnableSimEfieldSCE = pset.get<bool>("EnableSimEfieldSCE");
  fEnableCalSpatialSCE = pset.get<bool>("EnableCalSpatialSCE");
  fEnableCalEfieldSCE = pset.get<bool>("EnableCalEfieldSCE");
   
  //auto const *detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
  fEfield = detprop->Efield();  

  if((fEnableSimSpatialSCE == true) || (fEnableSimEfieldSCE == true))
  {
    auto const reprTypeString = pset.get<std::string>("RepresentationType");
    fRepresentationType = ParseRepresentationType(reprTypeString);
    if (fRepresentationType == SpaceChargeRepresentation_t::kUnknown) {
      throw cet::exception("SpaceChargeMicroBooNE")
        << "Unknown space charge representation type: '" << reprTypeString
        << "'\n";
    }
    fInputFilename = pset.get<std::string>("InputFilename");

    std::string fname;
    cet::search_path sp("FW_SEARCH_PATH");
    sp.find_file(fInputFilename,fname);
    
    TFile infile(fname.c_str(), "READ");
    if(!infile.IsOpen()) throw cet::exception("SpaceChargeMicroBooNE") << "Could not find the space charge effect file '" << fname << "'!\n";

    switch (fRepresentationType) {
      case SpaceChargeRepresentation_t::kVoxelized:
        {
        //Load in files
        //TFile* fileSCE = new TFile("????");
		TTree* treeD = (TTree*)infile.Get("SpaCEtree_fwdDisp"); //PICK DIRECTION?
        TTree* treeE = (TTree*)infile.Get("SpaCEtree");
        
        //Build histograms
        SCEhistograms = Build_TH3(treeD,treeE,"x_true","y_true","z_true","fwd");
        //Histograms are Dx, Dy, Dz, dEx/E0, dEy/E0, dEz/E0
        
	break; //kVoxelized;
        }
      case SpaceChargeRepresentation_t::kParametric:
        for(int i = 0; i < 5; i++)
        {
          g1_x[i] = makeInterpolator(infile, Form("deltaX/g1_%d",i));
          g2_x[i] = makeInterpolator(infile, Form("deltaX/g2_%d",i));
          g3_x[i] = makeInterpolator(infile, Form("deltaX/g3_%d",i));   
          g4_x[i] = makeInterpolator(infile, Form("deltaX/g4_%d",i));
          g5_x[i] = makeInterpolator(infile, Form("deltaX/g5_%d",i));

          g1_y[i] = makeInterpolator(infile, Form("deltaY/g1_%d",i));
          g2_y[i] = makeInterpolator(infile, Form("deltaY/g2_%d",i));
          g3_y[i] = makeInterpolator(infile, Form("deltaY/g3_%d",i));   
          g4_y[i] = makeInterpolator(infile, Form("deltaY/g4_%d",i));
          g5_y[i] = makeInterpolator(infile, Form("deltaY/g5_%d",i));
          g6_y[i] = makeInterpolator(infile, Form("deltaY/g6_%d",i));

          g1_z[i] = makeInterpolator(infile, Form("deltaZ/g1_%d",i));
          g2_z[i] = makeInterpolator(infile, Form("deltaZ/g2_%d",i));
          g3_z[i] = makeInterpolator(infile, Form("deltaZ/g3_%d",i));   
          g4_z[i] = makeInterpolator(infile, Form("deltaZ/g4_%d",i));

          g1_Ex[i] = makeInterpolator(infile, Form("deltaExOverE/g1_%d",i));
          g2_Ex[i] = makeInterpolator(infile, Form("deltaExOverE/g2_%d",i));
          g3_Ex[i] = makeInterpolator(infile, Form("deltaExOverE/g3_%d",i));
          g4_Ex[i] = makeInterpolator(infile, Form("deltaExOverE/g4_%d",i));
          g5_Ex[i] = makeInterpolator(infile, Form("deltaExOverE/g5_%d",i));
          
          g1_Ey[i] = makeInterpolator(infile, Form("deltaEyOverE/g1_%d",i));
          g2_Ey[i] = makeInterpolator(infile, Form("deltaEyOverE/g2_%d",i));
          g3_Ey[i] = makeInterpolator(infile, Form("deltaEyOverE/g3_%d",i));
          g4_Ey[i] = makeInterpolator(infile, Form("deltaEyOverE/g4_%d",i));
          g5_Ey[i] = makeInterpolator(infile, Form("deltaEyOverE/g5_%d",i));
          g6_Ey[i] = makeInterpolator(infile, Form("deltaEyOverE/g6_%d",i));
          
          g1_Ez[i] = makeInterpolator(infile, Form("deltaEzOverE/g1_%d",i));
          g2_Ez[i] = makeInterpolator(infile, Form("deltaEzOverE/g2_%d",i));
          g3_Ez[i] = makeInterpolator(infile, Form("deltaEzOverE/g3_%d",i));
          g4_Ez[i] = makeInterpolator(infile, Form("deltaEzOverE/g4_%d",i));
        }

        g1_x[5] = makeInterpolator(infile, "deltaX/g1_5");
        g2_x[5] = makeInterpolator(infile, "deltaX/g2_5");
        g3_x[5] = makeInterpolator(infile, "deltaX/g3_5");   
        g4_x[5] = makeInterpolator(infile, "deltaX/g4_5");
        g5_x[5] = makeInterpolator(infile, "deltaX/g5_5");

        g1_y[5] = makeInterpolator(infile, "deltaY/g1_5");
        g2_y[5] = makeInterpolator(infile, "deltaY/g2_5");
        g3_y[5] = makeInterpolator(infile, "deltaY/g3_5");   
        g4_y[5] = makeInterpolator(infile, "deltaY/g4_5");
        g5_y[5] = makeInterpolator(infile, "deltaY/g5_5");
        g6_y[5] = makeInterpolator(infile, "deltaY/g6_5");
        
        g1_x[6] = makeInterpolator(infile, "deltaX/g1_6");
        g2_x[6] = makeInterpolator(infile, "deltaX/g2_6");
        g3_x[6] = makeInterpolator(infile, "deltaX/g3_6");
        g4_x[6] = makeInterpolator(infile, "deltaX/g4_6");
        g5_x[6] = makeInterpolator(infile, "deltaX/g5_6");

        g1_Ex[5] = makeInterpolator(infile, "deltaExOverE/g1_5");
        g2_Ex[5] = makeInterpolator(infile, "deltaExOverE/g2_5");
        g3_Ex[5] = makeInterpolator(infile, "deltaExOverE/g3_5");
        g4_Ex[5] = makeInterpolator(infile, "deltaExOverE/g4_5");
        g5_Ex[5] = makeInterpolator(infile, "deltaExOverE/g5_5");
        
        g1_Ey[5] = makeInterpolator(infile, "deltaEyOverE/g1_5");
        g2_Ey[5] = makeInterpolator(infile, "deltaEyOverE/g2_5");
        g3_Ey[5] = makeInterpolator(infile, "deltaEyOverE/g3_5");
        g4_Ey[5] = makeInterpolator(infile, "deltaEyOverE/g4_5");
        g5_Ey[5] = makeInterpolator(infile, "deltaEyOverE/g5_5");
        g6_Ey[5] = makeInterpolator(infile, "deltaEyOverE/g6_5");
        
        g1_Ex[6] = makeInterpolator(infile, "deltaExOverE/g1_6");
        g2_Ex[6] = makeInterpolator(infile, "deltaExOverE/g2_6");
        g3_Ex[6] = makeInterpolator(infile, "deltaExOverE/g3_6");
        g4_Ex[6] = makeInterpolator(infile, "deltaExOverE/g4_6");
        g5_Ex[6] = makeInterpolator(infile, "deltaExOverE/g5_6");
        break; // kParametric
      case SpaceChargeRepresentation_t::kUnknown:
	throw cet::exception("SpaceChargeMicroBooNE") << "Unknown space charge representation.";
    } // switch

    infile.Close();
  }
  
  if((fEnableCalSpatialSCE == true) || (fEnableCalEfieldSCE == true))
  {
    fCalInputFilename = pset.get<std::string>("CalibrationInputFilename");

    std::string fname;
    cet::search_path sp("FW_SEARCH_PATH");
    sp.find_file(fCalInputFilename,fname);
    
    TFile infile(fname.c_str(), "READ");
    if(!infile.IsOpen()) throw cet::exception("SpaceChargeMicroBooNE") << "Could not find the space charge effect file '" << fname << "'!\n";
    
    //Load in trees
    TTree* treeD = (TTree*)infile.Get("SpaCEtree_bkwdDisp");
    TTree* treeE = (TTree*)infile.Get("SpaCEtree");
    
    //Build histograms
    CalSCEhistograms = Build_TH3(treeD,treeE,"x_reco","y_reco","z_reco","bkwd");
    //histograms are Dx, Dy, Dz, dEx/E0, dEy/E0, dEz/E0
    
    infile.Close();
    
  }


  fSpatialOffsetScale = pset.get<double>("SpatialOffsetScale",1.0);
  fEfieldOffsetScale  = pset.get<double>("EfieldOffsetScale",1.0);

  fEnableDataSimSpatialCorrection = pset.get<bool>("EnableDataSimSpatialCorrection",false);
  if(fEnableDataSimSpatialCorrection){

    fDataSimCorrFunc_MinX = pset.get<double>("DataSimSpatialCorrection_MinX");
    fDataSimCorrFunc_MaxX = pset.get<double>("DataSimSpatialCorrection_MaxX");

    auto mctop_pset = pset.get<fhicl::ParameterSet>("DataSimSpatialCorrection_MCTop_Func");
    fDataSimCorrFunc_MCTop = TF1("fDataSimCorrFunc_MCTop",(mctop_pset.get<std::string>("Form")).c_str(),
				 fDataSimCorrFunc_MinX,fDataSimCorrFunc_MaxX);    
    auto mctop_pars = mctop_pset.get<std::vector<double>>("Pars");
    if(mctop_pars.size()!=(unsigned int)fDataSimCorrFunc_MCTop.GetNpar())
      throw cet::exception("SpaceChargeMicroBooNE") << "DataSpatialCorrection_MCTop_FuncPars has wrong size";
    for(size_t i_p=0; i_p<mctop_pars.size(); ++i_p)
      fDataSimCorrFunc_MCTop.SetParameter(i_p,mctop_pars[i_p]);

    auto mcbottom_pset = pset.get<fhicl::ParameterSet>("DataSimSpatialCorrection_MCBottom_Func");
    fDataSimCorrFunc_MCBottom = TF1("fDataSimCorrFunc_MCBottom",(mcbottom_pset.get<std::string>("Form")).c_str(),
				    fDataSimCorrFunc_MinX,fDataSimCorrFunc_MaxX);    
    auto mcbottom_pars = mcbottom_pset.get<std::vector<double>>("Pars");
    if(mcbottom_pars.size()!=(unsigned int)fDataSimCorrFunc_MCBottom.GetNpar())
      throw cet::exception("SpaceChargeMicroBooNE") << "DataSpatialCorrection_MCBottom_FuncPars has wrong size";
    for(size_t i_p=0; i_p<mcbottom_pars.size(); ++i_p)
      fDataSimCorrFunc_MCBottom.SetParameter(i_p,mcbottom_pars[i_p]);

    auto datatop_pset = pset.get<fhicl::ParameterSet>("DataSimSpatialCorrection_DataTop_Func");
    fDataSimCorrFunc_DataTop = TF1("fDataSimCorrFunc_DataTop",(datatop_pset.get<std::string>("Form")).c_str(),
				   fDataSimCorrFunc_MinX,fDataSimCorrFunc_MaxX);    
    auto datatop_pars = datatop_pset.get<std::vector<double>>("Pars");
    if(datatop_pars.size()!=(unsigned int)fDataSimCorrFunc_DataTop.GetNpar())
      throw cet::exception("SpaceChargeMicroBooNE") << "DataSpatialCorrection_DataTop_FuncPars has wrong size";
    for(size_t i_p=0; i_p<datatop_pars.size(); ++i_p)
      fDataSimCorrFunc_DataTop.SetParameter(i_p,datatop_pars[i_p]);

    auto databottom_pset = pset.get<fhicl::ParameterSet>("DataSimSpatialCorrection_DataBottom_Func");
    fDataSimCorrFunc_DataBottom = TF1("fDataSimCorrFunc_DataBottom",(databottom_pset.get<std::string>("Form")).c_str(),
				      fDataSimCorrFunc_MinX,fDataSimCorrFunc_MaxX);    
    auto databottom_pars = databottom_pset.get<std::vector<double>>("Pars");
    if(databottom_pars.size()!=(unsigned int)fDataSimCorrFunc_DataBottom.GetNpar())
      throw cet::exception("SpaceChargeMicroBooNE") << "DataSpatialCorrection_DataBottom_FuncPars has wrong size";
    for(size_t i_p=0; i_p<databottom_pars.size(); ++i_p)
      fDataSimCorrFunc_DataBottom.SetParameter(i_p,databottom_pars[i_p]);
  }

  return true;
}

//------------------------------------------------
bool spacecharge::SpaceChargeMicroBooNE::Update(uint64_t ts) 
{
  if (ts == 0) return false;

  return true;
}


//----------------------------------------------------------------------------
/// Return boolean indicating whether or not to turn simulation of SCE on for
/// spatial distortions
bool spacecharge::SpaceChargeMicroBooNE::EnableSimSpatialSCE() const
{
  return fEnableSimSpatialSCE;
}

//----------------------------------------------------------------------------
/// Return boolean indicating whether or not to turn simulation of SCE on for
/// E field distortions
bool spacecharge::SpaceChargeMicroBooNE::EnableSimEfieldSCE() const
{
  return fEnableSimEfieldSCE;
}

//----------------------------------------------------------------------------
/// Return boolean indicating whether or not to apply SCE corrections from 
/// calibration for spatial distortions
bool spacecharge::SpaceChargeMicroBooNE::EnableCalSpatialSCE() const
{
  return fEnableCalSpatialSCE;
}

//----------------------------------------------------------------------------
/// Return boolean indicating whether or not to apply SCE corrections from 
/// calibration for E field distortions
bool spacecharge::SpaceChargeMicroBooNE::EnableCalEfieldSCE() const
{
  return fEnableCalEfieldSCE;
}

//----------------------------------------------------------------------------
/// Primary working method of service that provides position offsets to be
/// used in ionization electron drift
geo::Vector_t spacecharge::SpaceChargeMicroBooNE::GetPosOffsets(geo::Point_t const& tmp_point) const
{
  geo::Vector_t thePosOffsets;
  geo::Point_t point = tmp_point;
  if (!EnableSimSpatialSCE()) return thePosOffsets;     // no correction, zero displacement
  if(IsTooFarFromBoundaries(point)) return thePosOffsets;  // zero-initialised
  if(!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)) point = PretendAtBoundary(point);
  switch (fRepresentationType) {
  
    case SpaceChargeRepresentation_t::kVoxelized:
      thePosOffsets = GetOffsetsVoxel(point,SCEhistograms.at(0),SCEhistograms.at(1),SCEhistograms.at(2));
      break;
    
    case SpaceChargeRepresentation_t::kParametric:
      thePosOffsets = GetPosOffsetsParametric(point);
      break;
    
    case SpaceChargeRepresentation_t::kUnknown:
      throw cet::exception("SpaceChargeMicroBooNE") << "Unknown space charge representation.";
    
  } // switch

  double data_corr_scale=1.;
  if(fEnableDataSimSpatialCorrection){
    if(point.X()<fDataSimCorrFunc_MinX || point.X()>fDataSimCorrFunc_MaxX) {
      thePosOffsets = geo::Vector_t();
      data_corr_scale=1.;
    }
    else data_corr_scale = 0.5*(fDataSimCorrFunc_DataTop.Eval(point.X())/fDataSimCorrFunc_MCTop.Eval(point.X()) + 
				fDataSimCorrFunc_DataBottom.Eval(point.X())/fDataSimCorrFunc_MCBottom.Eval(point.X()));
  }
  thePosOffsets *= data_corr_scale*fSpatialOffsetScale;
  return thePosOffsets;
}

/// Primary working method of service that provides position offsets to be
/// used in ionization electron drift for correction in reconstruction
geo::Vector_t spacecharge::SpaceChargeMicroBooNE::GetCalPosOffsets(geo::Point_t const& tmp_point) const
{
  geo::Vector_t thePosOffsets;
  geo::Point_t point = tmp_point;
  if (!EnableCalSpatialSCE()) return thePosOffsets;     // no correction, zero displacement
  if (IsTooFarFromBoundaries(point)) return thePosOffsets;  // zero-initialised
  if (!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)) point = PretendAtBoundary(point);
  
  thePosOffsets = GetOffsetsVoxel(point,CalSCEhistograms.at(0),CalSCEhistograms.at(1),CalSCEhistograms.at(2));
    
  return thePosOffsets;
}

//----------------------------------------------------------------------------
/// Provides position offsets using voxelized interpolation
///
/// FIXME: The try-catch is a kludge until appropriate handling can be
/// developed for situations in which the transformed point lies
/// outside of the bounds of interpolation.
geo::Vector_t spacecharge::SpaceChargeMicroBooNE::GetOffsetsVoxel
  (geo::Point_t const& point,TH3F* hX, TH3F* hY, TH3F* hZ) const
try {
  geo::Point_t transformedPoint = Transform(point);
  return {
  	hX->Interpolate(transformedPoint.X(),transformedPoint.Y(),transformedPoint.Z()),
  	hY->Interpolate(transformedPoint.X(),transformedPoint.Y(),transformedPoint.Z()),
  	hZ->Interpolate(transformedPoint.X(),transformedPoint.Y(),transformedPoint.Z())
  	};
}
catch (art::Exception const& e) {
  	
  if (e.categoryCode() == art::errors::FatalRootError) {
    mf::LogAbsolute("RootError") << "BEGIN SUPPRESSED ERROR\n"
                                 << e.what()
                                 << "END SUPPRESSED ERROR";
    return geo::Vector_t{};
  }
  throw;
}
catch (...) {
  throw;
 }
 
/// Build 3D histograms for voxelized interpolation
std::vector<TH3F*> spacecharge::SpaceChargeMicroBooNE::Build_TH3
  (TTree* tree, TTree* eTree, std::string xvar, std::string yvar, std::string zvar, std::string posLeaf) const
{

  //Define the MicroBooNE detector 
  //MAKE THESE VARIABLE CONFIGURABLE
  double Lx = 2.5, Ly = 2.5, Lz = 10.0;
  double numDivisions_x = 25.0;
  double cell_size = Lx/numDivisions_x;
  double numDivisions_y = TMath::Nint((Ly/Lx)*((Double_t)numDivisions_x));
  double numDivisions_z = TMath::Nint((Lz/Lx)*((Double_t)numDivisions_x));

  double E_numDivisions_x = 20.0;
  double E_cell_size = Lx/E_numDivisions_x;
  double E_numDivisions_y = TMath::Nint((Ly/Lx)*((Double_t)E_numDivisions_x));
  double E_numDivisions_z = TMath::Nint((Lz/Lx)*((Double_t)E_numDivisions_x)); 

  //initialized histograms for Dx, Dy, Dz, and electric field
  TH3F* hDx = new TH3F("hDx", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1 ,-0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  TH3F* hDy = new TH3F("hDy", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1, -0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  TH3F* hDz = new TH3F("hDz", "", numDivisions_x+1, -0.5*cell_size, Lx+0.5*cell_size, numDivisions_y+1, -0.5*cell_size, Ly+0.5*cell_size, numDivisions_z+1, -0.5*cell_size, Lz+0.5*cell_size);
  
  TH3F* hEx = new TH3F("hEx", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
  TH3F* hEy = new TH3F("hEy", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
  TH3F* hEz = new TH3F("hEz", "", E_numDivisions_x+1, -0.5*E_cell_size, Lx+0.5*E_cell_size, E_numDivisions_y+1, -0.5*E_cell_size, Ly+0.5*E_cell_size, E_numDivisions_z+1, -0.5*E_cell_size, Lz+0.5*E_cell_size);
 
  //For each event, read the tree and fill each histogram
  for (int ii = 0; ii<tree->GetEntries(); ii++){

    //Read the trees
    tree->GetEntry(ii);
    Double_t x = tree->GetBranch(xvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t y = tree->GetBranch(yvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t z = tree->GetBranch(zvar.c_str())->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dx = tree->GetBranch("Dx")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dy = tree->GetBranch("Dy")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
    Double_t dz = tree->GetBranch("Dz")->GetLeaf(Form("data_%sDisp",posLeaf.c_str()))->GetValue();
   
    //Fill the histograms		
    hDx->Fill(x,y,z,100.0*dx);
    hDy->Fill(x,y,z,100.0*dy);
    hDz->Fill(x,y,z,100.0*dz);
  }
  
  //For each event, read the tree and fill each histogram
  for (int ii = 0; ii<eTree->GetEntries(); ii++){

		
    eTree->GetEntry(ii);
    Double_t x = eTree->GetBranch("xpoint")->GetLeaf("data")->GetValue();
    Double_t y = eTree->GetBranch("ypoint")->GetLeaf("data")->GetValue();
    Double_t z = eTree->GetBranch("zpoint")->GetLeaf("data")->GetValue();
    Double_t Ex = eTree->GetBranch("Ex")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
    Double_t Ey = eTree->GetBranch("Ey")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
    Double_t Ez = eTree->GetBranch("Ez")->GetLeaf("data")->GetValue() / (100000.0*fEfield);
   
    //Fill the histograms	
    hEx->Fill(x,y,z,Ex);
    hEy->Fill(x,y,z,Ey);
    hEz->Fill(x,y,z,Ez);
  }
  
  return  {hDx, hDy, hDz, hEx, hEy, hEz};

}

//----------------------------------------------------------------------------
/// Provides position offsets using a parametric representation
geo::Vector_t spacecharge::SpaceChargeMicroBooNE::GetPosOffsetsParametric
  (geo::Point_t const& point) const
{
  geo::Point_t transformedPoint = Transform(point);
  return {
    GetOnePosOffsetParametricX(transformedPoint),
    GetOnePosOffsetParametricY(transformedPoint),
    GetOnePosOffsetParametricZ(transformedPoint)
    };
}

//----------------------------------------------------------------------------
double spacecharge::SpaceChargeMicroBooNE::GetOnePosOffsetParametricX
  (geo::Point_t const& point) const
{
  
  double parA[5][7];
  double const zValNew = point.Z();
  for(size_t j = 0; j < 7; j++) 
  {
    parA[0][j] = g1_x[j].Eval(zValNew);
    parA[1][j] = g2_x[j].Eval(zValNew);
    parA[2][j] = g3_x[j].Eval(zValNew);
    parA[3][j] = g4_x[j].Eval(zValNew);
    parA[4][j] = g5_x[j].Eval(zValNew);
  }

  f1_x_poly_t f1_x;
  f2_x_poly_t f2_x;
  f3_x_poly_t f3_x;
  f4_x_poly_t f4_x;
  f5_x_poly_t f5_x;
  
  f1_x.SetParameters(parA[0]);
  f2_x.SetParameters(parA[1]);
  f3_x.SetParameters(parA[2]);
  f4_x.SetParameters(parA[3]);
  f5_x.SetParameters(parA[4]);

  double const aValNew = point.Y()-1.25;
  double const parB[] = {
    f1_x.Eval(aValNew),
    f2_x.Eval(aValNew),
    f3_x.Eval(aValNew),
    f4_x.Eval(aValNew),
    f5_x.Eval(aValNew)
  };
  
  double const bValNew = point.X()-1.25;
  return 100.0*fFinal_x_poly_t::Eval(bValNew, parB);
}

//----------------------------------------------------------------------------
/// Provides one position offset using a parametric representation, for Y axis
double spacecharge::SpaceChargeMicroBooNE::GetOnePosOffsetParametricY
  (geo::Point_t const& point) const
{
  
  double parA[6][6];
  double const zValNew = point.Z();
  for(size_t j = 0; j < 6; j++)
  {
    parA[0][j] = g1_y[j].Eval(zValNew);
    parA[1][j] = g2_y[j].Eval(zValNew);
    parA[2][j] = g3_y[j].Eval(zValNew);
    parA[3][j] = g4_y[j].Eval(zValNew);
    parA[4][j] = g5_y[j].Eval(zValNew);
    parA[5][j] = g6_y[j].Eval(zValNew);
  }
  
  f1_y_poly_t f1_y;
  f2_y_poly_t f2_y;
  f3_y_poly_t f3_y;
  f4_y_poly_t f4_y;
  f5_y_poly_t f5_y;
  f6_y_poly_t f6_y;
  
  f1_y.SetParameters(parA[0]);
  f2_y.SetParameters(parA[1]);
  f3_y.SetParameters(parA[2]);
  f4_y.SetParameters(parA[3]);
  f5_y.SetParameters(parA[4]);
  f6_y.SetParameters(parA[5]);
  
  double const aValNew = point.X()-1.25;
  
  double const parB[] = {
    f1_y.Eval(aValNew),
    f2_y.Eval(aValNew),
    f3_y.Eval(aValNew),
    f4_y.Eval(aValNew),
    f5_y.Eval(aValNew),
    f6_y.Eval(aValNew)
  };
  
  double const bValNew = point.Y()-1.25;
  return 100.0*fFinal_y_poly_t::Eval(bValNew, parB);
} // spacecharge::SpaceChargeMicroBooNE::GetOnePosOffsetParametricY()


//----------------------------------------------------------------------------
/// Provides one position offset using a parametric representation, for a given
/// axis
double spacecharge::SpaceChargeMicroBooNE::GetOnePosOffsetParametricZ
  (geo::Point_t const& point) const
{
  
  double parA[4][5];
  double const zValNew = point.Z();
  for(size_t j = 0; j < 5; j++)
  {
    parA[0][j] = g1_z[j].Eval(zValNew);
    parA[1][j] = g2_z[j].Eval(zValNew);
    parA[2][j] = g3_z[j].Eval(zValNew);
    parA[3][j] = g4_z[j].Eval(zValNew);
  }

  f1_z_poly_t f1_z;
  f2_z_poly_t f2_z;
  f3_z_poly_t f3_z;
  f4_z_poly_t f4_z;
  
  f1_z.SetParameters(parA[0]);
  f2_z.SetParameters(parA[1]);
  f3_z.SetParameters(parA[2]);
  f4_z.SetParameters(parA[3]);
  
  double const aValNew = point.Y()-1.25;
  double const parB[] = {
    f1_z.Eval(aValNew),
    f2_z.Eval(aValNew),
    f3_z.Eval(aValNew),
    f4_z.Eval(aValNew)
  };
  
  double const bValNew = point.X()-1.25;
  return 100.0*fFinal_z_poly_t::Eval(bValNew, parB);
  
} // spacecharge::SpaceChargeMicroBooNE::GetOnePosOffsetParametricZ()


//----------------------------------------------------------------------------
/// Primary working method of service that provides E field offsets to be
/// used in charge/light yield calculation (e.g.)
geo::Vector_t spacecharge::SpaceChargeMicroBooNE::GetEfieldOffsets(geo::Point_t const& tmp_point) const
{
  geo::Vector_t theEfieldOffsets;
  geo::Point_t point = tmp_point;
  if (!EnableSimEfieldSCE()) return theEfieldOffsets;      // no correction, zero distortion
  if(IsTooFarFromBoundaries(point)) return theEfieldOffsets;  // zero-initialised
  if(!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)) point = PretendAtBoundary(point);
  
  switch (fRepresentationType) {
    
    case SpaceChargeRepresentation_t::kVoxelized:
      theEfieldOffsets = -1.0*GetOffsetsVoxel(point,SCEhistograms.at(3),SCEhistograms.at(4),SCEhistograms.at(5));
      break;
    
    case SpaceChargeRepresentation_t::kParametric:
      theEfieldOffsets = -1.0*GetEfieldOffsetsParametric(point);
      break;
    
    case SpaceChargeRepresentation_t::kUnknown:
      throw cet::exception("SpaceChargeMicroBooNE") << "Unknown space charge representation.";
    
  } // switch
  
  theEfieldOffsets *= fEfieldOffsetScale;

  return theEfieldOffsets;
}

geo::Vector_t spacecharge::SpaceChargeMicroBooNE::GetCalEfieldOffsets(geo::Point_t const& tmp_point) const
{
  geo::Vector_t theEfieldOffsets;
  geo::Point_t point = tmp_point;
  if (!EnableCalEfieldSCE()) return theEfieldOffsets;      // no correction, zero distortion
  if(IsTooFarFromBoundaries(point)) return theEfieldOffsets;  // zero-initialised
  if(!IsInsideBoundaries(point)&&!IsTooFarFromBoundaries(point)) point = PretendAtBoundary(point);
  
  theEfieldOffsets = -1.0*GetOffsetsVoxel(point, CalSCEhistograms.at(3), CalSCEhistograms.at(4), CalSCEhistograms.at(5));
  
  std::cout<< "   in service: (" << point.X() << ", " << point.Y() << ", " << point.Z() << ") -> (" << theEfieldOffsets.X() << ", " << theEfieldOffsets.Y() << ", " << theEfieldOffsets.Z() << ")" << std::endl;

  theEfieldOffsets *= fEfieldOffsetScale;

  return theEfieldOffsets;
}

//----------------------------------------------------------------------------
/// Provides E field offsets using a parametric representation
geo::Vector_t spacecharge::SpaceChargeMicroBooNE::GetEfieldOffsetsParametric
  (geo::Point_t const& point) const
{
  geo::Point_t const transformedPoint = Transform(point);
  return {
    GetOneEfieldOffsetParametricX(transformedPoint),
    GetOneEfieldOffsetParametricY(transformedPoint),
    GetOneEfieldOffsetParametricZ(transformedPoint)
    };
}

//----------------------------------------------------------------------------
/// Provides one E field offset using a parametric representation, for x axis
double spacecharge::SpaceChargeMicroBooNE::GetOneEfieldOffsetParametricX
  (geo::Point_t const& point) const
{
  
  double parA[5][7];
  double const zValNew = point.Z();
  for(size_t j = 0; j < 7; j++)
  {
    parA[0][j] = g1_Ex[j].Eval(zValNew);
    parA[1][j] = g2_Ex[j].Eval(zValNew);
    parA[2][j] = g3_Ex[j].Eval(zValNew);
    parA[3][j] = g4_Ex[j].Eval(zValNew);
    parA[4][j] = g5_Ex[j].Eval(zValNew);
  }

  f1_Ex_poly_t f1_Ex;
  f2_Ex_poly_t f2_Ex;
  f3_Ex_poly_t f3_Ex;
  f4_Ex_poly_t f4_Ex;
  f5_Ex_poly_t f5_Ex;
  
  f1_Ex.SetParameters(parA[0]);
  f2_Ex.SetParameters(parA[1]);
  f3_Ex.SetParameters(parA[2]);
  f4_Ex.SetParameters(parA[3]);
  f5_Ex.SetParameters(parA[4]);
  
  double const aValNew = point.Y()-1.25;
  double const parB[] = {
    f1_Ex.Eval(aValNew),
    f2_Ex.Eval(aValNew),
    f3_Ex.Eval(aValNew),
    f4_Ex.Eval(aValNew),
    f5_Ex.Eval(aValNew)
  };
  
  double const bValNew = point.X()-1.25;
  return fFinal_Ex_poly_t::Eval(bValNew, parB);
  
} // spacecharge::SpaceChargeMicroBooNE::GetOneEfieldOffsetParametricX()

//----------------------------------------------------------------------------
/// Provides one E field offset using a parametric representation, for y axis
double spacecharge::SpaceChargeMicroBooNE::GetOneEfieldOffsetParametricY
  (geo::Point_t const& point) const
{
  
  double parA[6][6];
  double const zValNew = point.Z();
  for(size_t j = 0; j < 6; j++)
  {
    parA[0][j] = g1_Ey[j].Eval(zValNew);
    parA[1][j] = g2_Ey[j].Eval(zValNew);
    parA[2][j] = g3_Ey[j].Eval(zValNew);
    parA[3][j] = g4_Ey[j].Eval(zValNew);
    parA[4][j] = g5_Ey[j].Eval(zValNew);
    parA[5][j] = g6_Ey[j].Eval(zValNew);
  }

  f1_Ey_poly_t f1_Ey;
  f2_Ey_poly_t f2_Ey;
  f3_Ey_poly_t f3_Ey;
  f4_Ey_poly_t f4_Ey;
  f5_Ey_poly_t f5_Ey;
  f6_Ey_poly_t f6_Ey;
  
  f1_Ey.SetParameters(parA[0]);
  f2_Ey.SetParameters(parA[1]);
  f3_Ey.SetParameters(parA[2]);
  f4_Ey.SetParameters(parA[3]);
  f5_Ey.SetParameters(parA[4]);
  f6_Ey.SetParameters(parA[5]);

  double const aValNew = point.X()-1.25;
  double const parB[] = {
    f1_Ey.Eval(aValNew),
    f2_Ey.Eval(aValNew),
    f3_Ey.Eval(aValNew),
    f4_Ey.Eval(aValNew),
    f5_Ey.Eval(aValNew),
    f6_Ey.Eval(aValNew)
  };
  
  double const bValNew = point.Y()-1.25;
  return fFinal_Ey_poly_t::Eval(bValNew, parB);
  
} // spacecharge::SpaceChargeMicroBooNE::GetOneEfieldOffsetParametricY()


//----------------------------------------------------------------------------
/// Provides one E field offset using a parametric representation, for z axis
double spacecharge::SpaceChargeMicroBooNE::GetOneEfieldOffsetParametricZ
  (geo::Point_t const& point) const
{
  
  double parA[4][5];
  double const zValNew = point.Z();
  for(size_t j = 0; j < 5; j++)
  {
    parA[0][j] = g1_Ez[j].Eval(zValNew);
    parA[1][j] = g2_Ez[j].Eval(zValNew);
    parA[2][j] = g3_Ez[j].Eval(zValNew);
    parA[3][j] = g4_Ez[j].Eval(zValNew);
  }

  f1_Ez_poly_t f1_Ez;
  f2_Ez_poly_t f2_Ez;
  f3_Ez_poly_t f3_Ez;
  f4_Ez_poly_t f4_Ez;
  
  f1_Ez.SetParameters(parA[0]);
  f2_Ez.SetParameters(parA[1]);
  f3_Ez.SetParameters(parA[2]);
  f4_Ez.SetParameters(parA[3]);

  double const aValNew = point.Y()-1.25;
  double const parB[] = {
    f1_Ez.Eval(aValNew),
    f2_Ez.Eval(aValNew),
    f3_Ez.Eval(aValNew),
    f4_Ez.Eval(aValNew)
  };
  
  double const bValNew = point.X()-1.25;
  return fFinal_Ez_poly_t::Eval(bValNew, parB);
} // spacecharge::SpaceChargeMicroBooNE::GetOneEfieldOffsetParametricZ()


//----------------------------------------------------------------------------
/// Transform X to SCE X coordinate:  [2.56,0.0] --> [0.0,2.50]
double spacecharge::SpaceChargeMicroBooNE::TransformX(double xVal) const
{
  double xValNew;
  xValNew = 2.50 - (2.50/2.56)*(xVal/100.0);
  //xValNew -= 1.25;

  return xValNew;
}

//----------------------------------------------------------------------------
/// Transform Y to SCE Y coordinate:  [-1.165,1.165] --> [0.0,2.50]
double spacecharge::SpaceChargeMicroBooNE::TransformY(double yVal) const
{
  double yValNew;
  yValNew = (2.50/2.33)*((yVal/100.0)+1.165);
  //yValNew -= 1.25;

  return yValNew;
}

//----------------------------------------------------------------------------
/// Transform Z to SCE Z coordinate:  [0.0,10.37] --> [0.0,10.0]
double spacecharge::SpaceChargeMicroBooNE::TransformZ(double zVal) const
{
  double zValNew;
  zValNew = (10.0/10.37)*(zVal/100.0);

  return zValNew;
}

//----------------------------------------------------------------------------
geo::Point_t spacecharge::SpaceChargeMicroBooNE::Transform
  (geo::Point_t const& point) const
{
  return
    { TransformX(point.X()), TransformY(point.Y()), TransformZ(point.Z()) };
}

//----------------------------------------------------------------------------
/// Check to see if point is inside boundaries of map (allow to go slightly out of range)
bool spacecharge::SpaceChargeMicroBooNE::IsInsideBoundaries(geo::Point_t const& point) const
{
  return !(
       (point.X() <    0.0) || (point.X() >  256.0)
    || (point.Y() < -116.5) || (point.Y() >  116.5)
    || (point.Z() <    0.0) || (point.Z() > 1037.0)
    );
}

bool spacecharge::SpaceChargeMicroBooNE::IsTooFarFromBoundaries(geo::Point_t const& point) const
{
  return (
       (point.X() <    0.0) || (point.X() > 266.6)
    || (point.Y() < -126.5) || (point.Y() > 126.5)
    || (point.Z() <    -10) || (point.Z() > 1047.0)
    );
}

geo::Point_t spacecharge::SpaceChargeMicroBooNE::PretendAtBoundary(geo::Point_t const& point) const
{ 
  double x = point.X(), y = point.Y(), z = point.Z();
  
  if      (point.X() <   0.0) x =   0.0;
  else if (point.X() > 256.0) x = 256.0;
  
  if      (point.Y() < -116.5) y = -116.5;
  else if (point.Y() >  116.5) y =  116.5;

  if      (point.Z() <    0.0) z =    0.0;
  else if (point.Z() > 1037.0) z = 1037.0;
   
  return {x,y,z};
}
  

//----------------------------------------------------------------------------

spacecharge::SpaceChargeMicroBooNE::SpaceChargeRepresentation_t
spacecharge::SpaceChargeMicroBooNE::ParseRepresentationType
  (std::string repr_str)
{
  
  if (repr_str == "Parametric")      return SpaceChargeRepresentation_t::kParametric;
  else if (repr_str == "Voxelized") return SpaceChargeRepresentation_t::kVoxelized;
  else                               return SpaceChargeRepresentation_t::kUnknown;
  
} // spacecharge::SpaceChargeMicroBooNE::ParseRepresentationType()


//----------------------------------------------------------------------------
gsl::Interpolator spacecharge::SpaceChargeMicroBooNE::makeInterpolator
  (TFile& file, char const* graphName)
{
  SortedTGraphFromFile graph(file, graphName);
  return gsl::Interpolator(*graph);
} // spacecharge::SpaceChargeMicroBooNE::makeInterpolator()


//----------------------------------------------------------------------------
