////////////////////////////////////////////////////////////////////////
// Class:       RandomPrescale
// Module Type: filter
// File:        RandomPrescale_module.cc
//
// This is a filter module that accepts events with some specified
// probability.  Random number seed and engine are managed by NuRandomService
//               and RandomNumberGenerator service.
//
// FCL Parameters:
//
// Probability - Probability that each event will pass filter.
// Seed        - Specify seed.
//
// Generated at Mon Oct 12 15:10:20 2015 by Herbert Greenlee using artmod
// from cetpkgsupport v1_08_06.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "nurandom/RandomUtils/NuRandomService.h"
#include "CLHEP/Random/RandomEngine.h"

namespace util
{

  class RandomPrescale : public art::EDFilter {
  public:
    explicit RandomPrescale(fhicl::ParameterSet const & pset);

    // Plugins should not be copied or assigned.
    RandomPrescale(RandomPrescale const &) = delete;
    RandomPrescale(RandomPrescale &&) = delete;
    RandomPrescale & operator = (RandomPrescale const &) = delete;
    RandomPrescale & operator = (RandomPrescale &&) = delete;

  private:

    bool filter(art::Event & e) override;
    void endJob() override;

    // Fcl parameters.
    double fProb;           // Accept propability

    CLHEP::HepRandomEngine& fEngine;

    // Statistics.
    unsigned int fTotal{};    // Total events.
    unsigned int fPass{};     // Passing events.
  };
}

// Constructor.

util::RandomPrescale::RandomPrescale(fhicl::ParameterSet const & pset) : EDFilter{pset},
  fProb{pset.get<double>("Probability")},
  // Create a default random engine.
  // Obain the random seed from NuRandomService, unless overridden in configuration
  // with key "Seed."
  fEngine(art::ServiceHandle<rndm::NuRandomService>{}
          ->registerAndSeedEngine(createEngine(0, "HepJamesRandom", "RandomPrescale"),
                                  "HepJamesRandom", "RandomPrescale",pset, "Seed"))
{
  mf::LogInfo("RandomPrescale")
    << "RandomPrescale module configured with pass probability " << fProb;
}

bool util::RandomPrescale::filter(art::Event & e)
{
  ++fTotal;

  // Did this event pass?
  bool const pass = fEngine.flat() < fProb;
  if(pass)
    ++fPass;
  return pass;
}

// End job method.  Print statistics.

void util::RandomPrescale::endJob()
{
  float ratio = 0.;
  if(fTotal != 0)
    ratio = float(fPass) / float(fTotal);

  mf::LogInfo("RandomPrescale")
    << "RandomPrescale statistics:\n"
    << fPass << " events passed\n"
    << fTotal << " total events\n"
    << "Passing fraction = " << ratio;
}

DEFINE_ART_MODULE(util::RandomPrescale)
