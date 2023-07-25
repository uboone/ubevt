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

// LArSoft includes
#include "ubevt/SpaceChargeServices/SpaceChargeServiceMicroBooNE.h"
#include "larcore/CoreUtils/ServiceUtil.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"

// ROOT includes
#include "TMath.h"

// Framework includes
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib_except/exception.h"

//-----------------------------------------------
spacecharge::SpaceChargeServiceMicroBooNE::SpaceChargeServiceMicroBooNE(fhicl::ParameterSet const& pset, art::ActivityRegistry &reg)
{
  TH1::AddDirectory(kFALSE);
  auto const detprop = art::ServiceHandle<detinfo::DetectorPropertiesService>()->DataForJob();
  fProp.reset(new spacecharge::SpaceChargeMicroBooNE(pset, detprop));

  reg.sPreBeginRun.watch(this, &SpaceChargeServiceMicroBooNE::preBeginRun);
}

//----------------------------------------------
void spacecharge::SpaceChargeServiceMicroBooNE::preBeginRun(const art::Run& run)
{
  fProp->Update(run.id().run());
}

//------------------------------------------------
