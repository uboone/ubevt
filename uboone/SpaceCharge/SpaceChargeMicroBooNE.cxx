////////////////////////////////////////////////////////////////////////
// \file SpaceChargeMicroBooNE.cxx
//
// \brief implementation of class for storing/accessing space charge distortions for MicroBooNE
//
// \author mrmooney@bnl.gov
// 
////////////////////////////////////////////////////////////////////////

// C++ language includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "math.h"
#include "stdio.h"

// LArSoft includes
#include "uboone/SpaceCharge/SpaceChargeMicroBooNE.h"

// Framework includes
#include "cetlib/exception.h"

//-----------------------------------------------
spacecharge::SpaceChargeMicroBooNE::SpaceChargeMicroBooNE(
  fhicl::ParameterSet const& pset
)
{
  Configure(pset);
}

//------------------------------------------------
bool spacecharge::SpaceChargeMicroBooNE::Configure(fhicl::ParameterSet const& pset)
{  
  fEnableSimSpatialSCE = pset.get<bool>("EnableSimSpatialSCE");
  fEnableSimEfieldSCE = pset.get<bool>("EnableSimEfieldSCE");
  fEnableCorrSCE = pset.get<bool>("EnableCorrSCE");

  if((fEnableSimSpatialSCE == true) || (fEnableSimEfieldSCE == true))
  {
    fRepresentationType = pset.get<std::string>("RepresentationType");
    fInputFilename = pset.get<std::string>("InputFilename");

    std::string fname;
    cet::search_path sp("FW_SEARCH_PATH");
    sp.find_file(fInputFilename,fname);

    std::unique_ptr<TFile> infile(new TFile(fname.c_str(), "READ"));
    if(!infile->IsOpen()) throw cet::exception("SpaceChargeMicroBooNE") << "Could not find the space charge effect file '" << fname << "'!\n";

    if(fRepresentationType == "Parametric")
    {      
      for(int i = 0; i < 5; i++)
      {
        g1_x[i] = (TGraph*)infile->Get(Form("deltaX/g1_%d",i));
        g2_x[i] = (TGraph*)infile->Get(Form("deltaX/g2_%d",i));
        g3_x[i] = (TGraph*)infile->Get(Form("deltaX/g3_%d",i));   
        g4_x[i] = (TGraph*)infile->Get(Form("deltaX/g4_%d",i));
        g5_x[i] = (TGraph*)infile->Get(Form("deltaX/g5_%d",i));

        g1_y[i] = (TGraph*)infile->Get(Form("deltaY/g1_%d",i));
        g2_y[i] = (TGraph*)infile->Get(Form("deltaY/g2_%d",i));
        g3_y[i] = (TGraph*)infile->Get(Form("deltaY/g3_%d",i));   
        g4_y[i] = (TGraph*)infile->Get(Form("deltaY/g4_%d",i));
        g5_y[i] = (TGraph*)infile->Get(Form("deltaY/g5_%d",i));
        g6_y[i] = (TGraph*)infile->Get(Form("deltaY/g6_%d",i));

        g1_z[i] = (TGraph*)infile->Get(Form("deltaZ/g1_%d",i));
        g2_z[i] = (TGraph*)infile->Get(Form("deltaZ/g2_%d",i));
        g3_z[i] = (TGraph*)infile->Get(Form("deltaZ/g3_%d",i));   
        g4_z[i] = (TGraph*)infile->Get(Form("deltaZ/g4_%d",i));

	g1_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g1_%d",i));
	g2_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g2_%d",i));
	g3_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g3_%d",i));
	g4_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g4_%d",i));
	g5_Ex[i] = (TGraph*)infile->Get(Form("deltaExOverE/g5_%d",i));

	g1_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g1_%d",i));
	g2_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g2_%d",i));
	g3_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g3_%d",i));
	g4_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g4_%d",i));
	g5_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g5_%d",i));
	g6_Ey[i] = (TGraph*)infile->Get(Form("deltaEyOverE/g6_%d",i));

	g1_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g1_%d",i));
	g2_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g2_%d",i));
	g3_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g3_%d",i));
	g4_Ez[i] = (TGraph*)infile->Get(Form("deltaEzOverE/g4_%d",i));
      }

      g1_x[5] = (TGraph*)infile->Get("deltaX/g1_5");
      g2_x[5] = (TGraph*)infile->Get("deltaX/g2_5");
      g3_x[5] = (TGraph*)infile->Get("deltaX/g3_5");   
      g4_x[5] = (TGraph*)infile->Get("deltaX/g4_5");
      g5_x[5] = (TGraph*)infile->Get("deltaX/g5_5");

      g1_y[5] = (TGraph*)infile->Get("deltaY/g1_5");
      g2_y[5] = (TGraph*)infile->Get("deltaY/g2_5");
      g3_y[5] = (TGraph*)infile->Get("deltaY/g3_5");   
      g4_y[5] = (TGraph*)infile->Get("deltaY/g4_5");
      g5_y[5] = (TGraph*)infile->Get("deltaY/g5_5");
      g6_y[5] = (TGraph*)infile->Get("deltaY/g6_5");
      
      g1_x[6] = (TGraph*)infile->Get("deltaX/g1_6");
      g2_x[6] = (TGraph*)infile->Get("deltaX/g2_6");
      g3_x[6] = (TGraph*)infile->Get("deltaX/g3_6");
      g4_x[6] = (TGraph*)infile->Get("deltaX/g4_6");
      g5_x[6] = (TGraph*)infile->Get("deltaX/g5_6");

      g1_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g1_5");
      g2_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g2_5");
      g3_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g3_5");
      g4_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g4_5");
      g5_Ex[5] = (TGraph*)infile->Get("deltaExOverE/g5_5");

      g1_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g1_5");
      g2_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g2_5");
      g3_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g3_5");
      g4_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g4_5");
      g5_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g5_5");
      g6_Ey[5] = (TGraph*)infile->Get("deltaEyOverE/g6_5");

      g1_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g1_6");
      g2_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g2_6");
      g3_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g3_6");
      g4_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g4_6");
      g5_Ex[6] = (TGraph*)infile->Get("deltaExOverE/g5_6");
    }

    infile->Close();
  }
  
  SortAllGraphs();

  if(fEnableCorrSCE == true)
  {
    // Grab other parameters from pset  
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
/// Return boolean indicating whether or not to apply SCE corrections
bool spacecharge::SpaceChargeMicroBooNE::EnableCorrSCE() const
{
  return fEnableCorrSCE;
}

//----------------------------------------------------------------------------
/// Primary working method of service that provides position offsets to be
/// used in ionization electron drift
std::vector<double> spacecharge::SpaceChargeMicroBooNE::GetPosOffsets(double xVal, double yVal, double zVal) const
{
  std::vector<double> thePosOffsets;

  if(IsInsideBoundaries(xVal,yVal,zVal) == false)
  {
    thePosOffsets.resize(3,0.0);
  }
  else
  {
    if(fRepresentationType == "Parametric")
      thePosOffsets = GetPosOffsetsParametric(xVal,yVal,zVal);
    else
      thePosOffsets.resize(3,0.0);
  }

  return thePosOffsets;
}

//----------------------------------------------------------------------------
/// Provides position offsets using a parametric representation
std::vector<double> spacecharge::SpaceChargeMicroBooNE::GetPosOffsetsParametric(double xVal, double yVal, double zVal) const
{
  std::vector<double> thePosOffsetsParametric;

  double xValNew = TransformX(xVal);
  double yValNew = TransformY(yVal);
  double zValNew = TransformZ(zVal);

  thePosOffsetsParametric.push_back(GetOnePosOffsetParametric(xValNew,yValNew,zValNew,"X"));
  thePosOffsetsParametric.push_back(GetOnePosOffsetParametric(xValNew,yValNew,zValNew,"Y"));
  thePosOffsetsParametric.push_back(GetOnePosOffsetParametric(xValNew,yValNew,zValNew,"Z"));

  return thePosOffsetsParametric;
}

//----------------------------------------------------------------------------
/// Provides one position offset using a parametric representation, for a given
/// axis
double spacecharge::SpaceChargeMicroBooNE::GetOnePosOffsetParametric(double xValNew, double yValNew, double zValNew, std::string axis) const
{
  
  f1_x_poly_t f1_x;
  f2_x_poly_t f2_x;
  f3_x_poly_t f3_x;
  f4_x_poly_t f4_x;
  f5_x_poly_t f5_x;
  
  f1_y_poly_t f1_y;
  f2_y_poly_t f2_y;
  f3_y_poly_t f3_y;
  f4_y_poly_t f4_y;
  f5_y_poly_t f5_y;
  f6_y_poly_t f6_y;
  
  f1_z_poly_t f1_z;
  f2_z_poly_t f2_z;
  f3_z_poly_t f3_z;
  f4_z_poly_t f4_z;
  
  if(axis == "X")
  {
    double parA[5][7];
    for(int j = 0; j < 7; j++)
    {
      parA[0][j] = g1_x[j]->Eval(zValNew, nullptr, nullptr);
      parA[1][j] = g2_x[j]->Eval(zValNew, nullptr, nullptr);
      parA[2][j] = g3_x[j]->Eval(zValNew, nullptr, nullptr);
      parA[3][j] = g4_x[j]->Eval(zValNew, nullptr, nullptr);
      parA[4][j] = g5_x[j]->Eval(zValNew, nullptr, nullptr);
    }
  
    f1_x.SetParameters(parA[0]);
    f2_x.SetParameters(parA[1]);
    f3_x.SetParameters(parA[2]);
    f4_x.SetParameters(parA[3]);
    f5_x.SetParameters(parA[4]);
  }
  else if(axis == "Y")
  {
    double parA[6][6];
    for(int j = 0; j < 6; j++)
    {
      parA[0][j] = g1_y[j]->Eval(zValNew, nullptr, nullptr);
      parA[1][j] = g2_y[j]->Eval(zValNew, nullptr, nullptr);
      parA[2][j] = g3_y[j]->Eval(zValNew, nullptr, nullptr);
      parA[3][j] = g4_y[j]->Eval(zValNew, nullptr, nullptr);
      parA[4][j] = g5_y[j]->Eval(zValNew, nullptr, nullptr);
      parA[5][j] = g6_y[j]->Eval(zValNew, nullptr, nullptr);
    }
  
    f1_y.SetParameters(parA[0]);
    f2_y.SetParameters(parA[1]);
    f3_y.SetParameters(parA[2]);
    f4_y.SetParameters(parA[3]);
    f5_y.SetParameters(parA[4]);
    f6_y.SetParameters(parA[5]);
  }
  else if(axis == "Z")
  {
    double parA[4][5];
    for(int j = 0; j < 5; j++)
    {
      parA[0][j] = g1_z[j]->Eval(zValNew, nullptr, nullptr);
      parA[1][j] = g2_z[j]->Eval(zValNew, nullptr, nullptr);
      parA[2][j] = g3_z[j]->Eval(zValNew, nullptr, nullptr);
      parA[3][j] = g4_z[j]->Eval(zValNew, nullptr, nullptr);
    }
  
    f1_z.SetParameters(parA[0]);
    f2_z.SetParameters(parA[1]);
    f3_z.SetParameters(parA[2]);
    f4_z.SetParameters(parA[3]);
  }
  
  double aValNew;
  double bValNew;
  
  if(axis == "Y")
  {
    aValNew = xValNew;
    bValNew = yValNew;
  }
  else
  {
    aValNew = yValNew;
    bValNew = xValNew;
  }
  
  double offsetValNew = 0.0;
  if(axis == "X")
  {
    double const parB[] = {
      f1_x.Eval(aValNew),
      f2_x.Eval(aValNew),
      f3_x.Eval(aValNew),
      f4_x.Eval(aValNew),
      f5_x.Eval(aValNew)
    };
  
    offsetValNew = 100.0*fFinal_x_poly_t::Eval(bValNew, parB);
  }
  else if(axis == "Y")
  {
    double const parB[] = {
      f1_y.Eval(aValNew),
      f2_y.Eval(aValNew),
      f3_y.Eval(aValNew),
      f4_y.Eval(aValNew),
      f5_y.Eval(aValNew),
      f6_y.Eval(aValNew)
    };
  
    offsetValNew = 100.0*fFinal_y_poly_t::Eval(bValNew, parB);
  }
  else if(axis == "Z")
  {
    double const parB[] = {
      f1_z.Eval(aValNew),
      f2_z.Eval(aValNew),
      f3_z.Eval(aValNew),
      f4_z.Eval(aValNew)
    };
  
    offsetValNew = 100.0*fFinal_z_poly_t::Eval(bValNew, parB);
  }
  
  return offsetValNew;
}

//----------------------------------------------------------------------------
/// Primary working method of service that provides E field offsets to be
/// used in charge/light yield calculation (e.g.)
std::vector<double> spacecharge::SpaceChargeMicroBooNE::GetEfieldOffsets(double xVal, double yVal, double zVal) const
{
  std::vector<double> theEfieldOffsets;

  if(IsInsideBoundaries(xVal,yVal,zVal) == false)
  {
    theEfieldOffsets.resize(3,0.0);
  }
  else
  {
    if(fRepresentationType == "Parametric")
      theEfieldOffsets = GetEfieldOffsetsParametric(xVal,yVal,zVal);
    else
      theEfieldOffsets.resize(3,0.0);
  }

  theEfieldOffsets.at(0) = -1.0*theEfieldOffsets.at(0);
  theEfieldOffsets.at(1) = -1.0*theEfieldOffsets.at(1);
  theEfieldOffsets.at(2) = -1.0*theEfieldOffsets.at(2);

  return theEfieldOffsets;
}

//----------------------------------------------------------------------------
/// Provides E field offsets using a parametric representation
std::vector<double> spacecharge::SpaceChargeMicroBooNE::GetEfieldOffsetsParametric(double xVal, double yVal, double zVal) const
{
  std::vector<double> theEfieldOffsetsParametric;

  double xValNew = TransformX(xVal);
  double yValNew = TransformY(yVal);
  double zValNew = TransformZ(zVal);

  theEfieldOffsetsParametric.push_back(GetOneEfieldOffsetParametric(xValNew,yValNew,zValNew,"X"));
  theEfieldOffsetsParametric.push_back(GetOneEfieldOffsetParametric(xValNew,yValNew,zValNew,"Y"));
  theEfieldOffsetsParametric.push_back(GetOneEfieldOffsetParametric(xValNew,yValNew,zValNew,"Z"));

  return theEfieldOffsetsParametric;
}

//----------------------------------------------------------------------------
/// Provides one E field offset using a parametric representation, for a given
/// axis
double spacecharge::SpaceChargeMicroBooNE::GetOneEfieldOffsetParametric(double xValNew, double yValNew, double zValNew, std::string axis) const
{
  
  f1_Ex_poly_t f1_Ex;
  f2_Ex_poly_t f2_Ex;
  f3_Ex_poly_t f3_Ex;
  f4_Ex_poly_t f4_Ex;
  f5_Ex_poly_t f5_Ex;
  
  f1_Ey_poly_t f1_Ey;
  f2_Ey_poly_t f2_Ey;
  f3_Ey_poly_t f3_Ey;
  f4_Ey_poly_t f4_Ey;
  f5_Ey_poly_t f5_Ey;
  f6_Ey_poly_t f6_Ey;
  
  f1_Ez_poly_t f1_Ez;
  f2_Ez_poly_t f2_Ez;
  f3_Ez_poly_t f3_Ez;
  f4_Ez_poly_t f4_Ez;
  
  if(axis == "X")
  {
    double parA[5][7];
    for(int j = 0; j < 7; j++)
    {
      parA[0][j] = g1_Ex[j]->Eval(zValNew, nullptr, nullptr);
      parA[1][j] = g2_Ex[j]->Eval(zValNew, nullptr, nullptr);
      parA[2][j] = g3_Ex[j]->Eval(zValNew, nullptr, nullptr);
      parA[3][j] = g4_Ex[j]->Eval(zValNew, nullptr, nullptr);
      parA[4][j] = g5_Ex[j]->Eval(zValNew, nullptr, nullptr);
    }
  
    f1_Ex.SetParameters(parA[0]);
    f2_Ex.SetParameters(parA[1]);
    f3_Ex.SetParameters(parA[2]);
    f4_Ex.SetParameters(parA[3]);
    f5_Ex.SetParameters(parA[4]);
  }
  else if(axis == "Y")
  {
    double parA[6][6];
    for(int j = 0; j < 6; j++)
    {
      parA[0][j] = g1_Ey[j]->Eval(zValNew, nullptr, nullptr);
      parA[1][j] = g2_Ey[j]->Eval(zValNew, nullptr, nullptr);
      parA[2][j] = g3_Ey[j]->Eval(zValNew, nullptr, nullptr);
      parA[3][j] = g4_Ey[j]->Eval(zValNew, nullptr, nullptr);
      parA[4][j] = g5_Ey[j]->Eval(zValNew, nullptr, nullptr);
      parA[5][j] = g6_Ey[j]->Eval(zValNew, nullptr, nullptr);
    }
  
    f1_Ey.SetParameters(parA[0]);
    f2_Ey.SetParameters(parA[1]);
    f3_Ey.SetParameters(parA[2]);
    f4_Ey.SetParameters(parA[3]);
    f5_Ey.SetParameters(parA[4]);
    f6_Ey.SetParameters(parA[5]);
  }
  else if(axis == "Z")
  {
    double parA[4][5];
    for(int j = 0; j < 5; j++)
    {
      parA[0][j] = g1_Ez[j]->Eval(zValNew, nullptr, nullptr);
      parA[1][j] = g2_Ez[j]->Eval(zValNew, nullptr, nullptr);
      parA[2][j] = g3_Ez[j]->Eval(zValNew, nullptr, nullptr);
      parA[3][j] = g4_Ez[j]->Eval(zValNew, nullptr, nullptr);
    }
  
    f1_Ez.SetParameters(parA[0]);
    f2_Ez.SetParameters(parA[1]);
    f3_Ez.SetParameters(parA[2]);
    f4_Ez.SetParameters(parA[3]);
  }
  
  double aValNew;
  double bValNew;
  
  if(axis == "Y")
  {
    aValNew = xValNew;
    bValNew = yValNew;
  }
  else
  {
    aValNew = yValNew;
    bValNew = xValNew;
  }
  
  double offsetValNew = 0.0;
  if(axis == "X")
  {
    double const parB[] = {
      f1_Ex.Eval(aValNew),
      f2_Ex.Eval(aValNew),
      f3_Ex.Eval(aValNew),
      f4_Ex.Eval(aValNew),
      f5_Ex.Eval(aValNew)
    };
  
    offsetValNew = fFinal_Ex_poly_t::Eval(bValNew, parB);
  }
  else if(axis == "Y")
  {
    double const parB[] = {
      f1_Ey.Eval(aValNew),
      f2_Ey.Eval(aValNew),
      f3_Ey.Eval(aValNew),
      f4_Ey.Eval(aValNew),
      f5_Ey.Eval(aValNew),
      f6_Ey.Eval(aValNew)
    };
  
    offsetValNew = fFinal_Ey_poly_t::Eval(bValNew, parB);
  }
  else if(axis == "Z")
  {
    double const parB[] = {
      f1_Ez.Eval(aValNew),
      f2_Ez.Eval(aValNew),
      f3_Ez.Eval(aValNew),
      f4_Ez.Eval(aValNew)
    };
  
    offsetValNew = fFinal_Ez_poly_t::Eval(bValNew, parB);
  }
  
  return offsetValNew;
}

//----------------------------------------------------------------------------
/// Transform X to SCE X coordinate:  [2.56,0.0] --> [0.0,2.50]
double spacecharge::SpaceChargeMicroBooNE::TransformX(double xVal) const
{
  double xValNew;
  xValNew = 2.50 - (2.50/2.56)*(xVal/100.0);
  xValNew -= 1.25;

  return xValNew;
}

//----------------------------------------------------------------------------
/// Transform Y to SCE Y coordinate:  [-1.165,1.165] --> [0.0,2.50]
double spacecharge::SpaceChargeMicroBooNE::TransformY(double yVal) const
{
  double yValNew;
  yValNew = (2.50/2.33)*((yVal/100.0)+1.165);
  yValNew -= 1.25;

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
/// Check to see if point is inside boundaries of map (allow to go slightly out of range)
bool spacecharge::SpaceChargeMicroBooNE::IsInsideBoundaries(double xVal, double yVal, double zVal) const
{
  bool isInside = true;

  if((xVal < 0.0) || (xVal > 260.0) || (yVal < -120.0) || (yVal > 120.0) || (zVal < 0.0) || (zVal > 1050.0))
  {
    isInside = false;
  }

  return isInside;
}



//----------------------------------------------------------------------------
void spacecharge::SpaceChargeMicroBooNE::SortGraphs(std::vector<TGraph*> const& graphs) {
  for (TGraph* graph: graphs) if (graph) graph->Sort();
} // spacecharge::SpaceChargeMicroBooNE::SortGraphs()


//----------------------------------------------------------------------------
void spacecharge::SpaceChargeMicroBooNE::SortAllGraphs() {
  SortGraphs(g1_x);
  SortGraphs(g2_x);
  SortGraphs(g3_x);
  SortGraphs(g4_x);
  SortGraphs(g5_x);
  SortGraphs(g1_y);
  SortGraphs(g2_y);
  SortGraphs(g3_y);
  SortGraphs(g4_y);
  SortGraphs(g5_y);
  SortGraphs(g6_y);
  SortGraphs(g1_z);
  SortGraphs(g2_z);
  SortGraphs(g3_z);
  SortGraphs(g4_z);
  SortGraphs(g1_Ex);
  SortGraphs(g2_Ex);
  SortGraphs(g3_Ex);
  SortGraphs(g4_Ex);
  SortGraphs(g5_Ex);
  SortGraphs(g1_Ey);
  SortGraphs(g2_Ey);
  SortGraphs(g3_Ey);
  SortGraphs(g4_Ey);
  SortGraphs(g5_Ey);
  SortGraphs(g6_Ey);
  SortGraphs(g1_Ez);
  SortGraphs(g2_Ez);
  SortGraphs(g3_Ez);
  SortGraphs(g4_Ez);
} // spacecharge::SpaceChargeMicroBooNE::SortAllGraphs()
