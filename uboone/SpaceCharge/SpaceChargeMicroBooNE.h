////////////////////////////////////////////////////////////////////////
// \file SpaceChargeMicroBooNE.h
//
// \brief header of class for storing/accessing space charge distortions for MicroBooNE
//
// \author mrmooney@bnl.gov
// 
////////////////////////////////////////////////////////////////////////
#ifndef SPACECHARGE_SPACECHARGEMICROBOONE_H
#define SPACECHARGE_SPACECHARGEMICROBOONE_H

// LArSoft libraries
#include "larevt/SpaceCharge/SpaceCharge.h"

// FHiCL libraries
#include "fhiclcpp/ParameterSet.h"

// ROOT includes
#include "TGraph.h"
#include "TF1.h"
#include "TFile.h"

// C/C++ standard libraries
#include <string>
#include <vector>
#include <array>
#include <algorithm> // std::copy_n()



namespace spacecharge {
  
  /// GSL polynomial evaluation wrapper.
  template <unsigned int N>
  struct SimplePolynomialBase {
    static constexpr unsigned int Degree = N; ///< Degree of the polynomial.
    
    /// Number of parameters.
    static constexpr unsigned int NParams = Degree + 1;
    
    using Params_t = std::array<double, NParams>; ///< Set of parameters.
    
    /// Evaluates the polynomial with specified parameters at point `x`.
    static double Eval(double x, double const* params);
    
    /// Evaluates the polynomial with specified parameters at point `x`.
    static double Eval(double x, Params_t const& params)
      { return Eval(x, params.data()); }
    
  }; // class SimplePolynomialBase<>
  
  
  /// Implementation of polynomial evaluations mimicking some of TF1 I/F.
  template <unsigned int N>
  class SimplePolynomial: public SimplePolynomialBase<N> {
    using PolyImpl_t = SimplePolynomialBase<N>;
      public:
    static constexpr unsigned int Degree = PolyImpl_t::Degree;
    static constexpr unsigned int NParams = PolyImpl_t::NParams;
    
    using Params_t = typename PolyImpl_t::Params_t;
    
    /// Default constructor: coefficients are *not initialised*.
    SimplePolynomial() = default;
    
    /// Copies the specified parameters into this object.
    void SetParameters(double const* params)
      { std::copy_n(params, NParams, fParams.begin()); }
    
    
    using SimplePolynomialBase<N>::Eval;
    
    /// Evaluates the polynomial at the specified point `x`.
    double Eval(double x) const
      { return PolyImpl_t::Eval(x, fParams.data()); }
    
      protected:
    Params_t fParams; ///< Parameters of the polynomial.
    
  }; // class SimplePolynomial<>
  
  
  class SpaceChargeMicroBooNE : public SpaceCharge {
 
    public:

      explicit SpaceChargeMicroBooNE(fhicl::ParameterSet const& pset);
      SpaceChargeMicroBooNE(SpaceChargeMicroBooNE const&) = delete;
      virtual ~SpaceChargeMicroBooNE() = default;
      
      bool Configure(fhicl::ParameterSet const& pset);
      bool Update(uint64_t ts=0);
      
      bool EnableSimSpatialSCE() const override;
      bool EnableSimEfieldSCE() const override;
      bool EnableCorrSCE() const override;
      std::vector<double> GetPosOffsets(double xVal, double yVal, double zVal) const override;
      std::vector<double> GetEfieldOffsets(double xVal, double yVal, double zVal) const override;
 
    private:
    protected:

      std::vector<double> GetPosOffsetsParametric(double xVal, double yVal, double zVal) const;
      double GetOnePosOffsetParametric(double xVal, double yVal, double zVal, std::string axis) const;
      std::vector<double> GetEfieldOffsetsParametric(double xVal, double yVal, double zVal) const;
      double GetOneEfieldOffsetParametric(double xVal, double yVal, double zVal, std::string axis) const;
      double TransformX(double xVal) const;
      double TransformY(double yVal) const;
      double TransformZ(double zVal) const;
      bool IsInsideBoundaries(double xVal, double yVal, double zVal) const;

      bool fEnableSimSpatialSCE;
      bool fEnableSimEfieldSCE;
      bool fEnableCorrSCE;
      
      std::string fRepresentationType;
      std::string fInputFilename;
      
      std::vector<TGraph *> g1_x { 7, nullptr };
      std::vector<TGraph *> g2_x { 7, nullptr };
      std::vector<TGraph *> g3_x { 7, nullptr };
      std::vector<TGraph *> g4_x { 7, nullptr };
      std::vector<TGraph *> g5_x { 7, nullptr };
      
      std::vector<TGraph *> g1_y { 7, nullptr };
      std::vector<TGraph *> g2_y { 7, nullptr };
      std::vector<TGraph *> g3_y { 7, nullptr };
      std::vector<TGraph *> g4_y { 7, nullptr };
      std::vector<TGraph *> g5_y { 7, nullptr };
      std::vector<TGraph *> g6_y { 7, nullptr };
      
      std::vector<TGraph *> g1_z { 7, nullptr };
      std::vector<TGraph *> g2_z { 7, nullptr };
      std::vector<TGraph *> g3_z { 7, nullptr };
      std::vector<TGraph *> g4_z { 7, nullptr };
      
      typedef SimplePolynomial<6> f1_x_poly_t;
      typedef SimplePolynomial<6> f2_x_poly_t;
      typedef SimplePolynomial<6> f3_x_poly_t;
      typedef SimplePolynomial<6> f4_x_poly_t;
      typedef SimplePolynomial<6> f5_x_poly_t;
      typedef SimplePolynomialBase<4> fFinal_x_poly_t;
      
      typedef SimplePolynomial<5> f1_y_poly_t;
      typedef SimplePolynomial<5> f2_y_poly_t;
      typedef SimplePolynomial<5> f3_y_poly_t;
      typedef SimplePolynomial<5> f4_y_poly_t;
      typedef SimplePolynomial<5> f5_y_poly_t;
      typedef SimplePolynomial<5> f6_y_poly_t;
      typedef SimplePolynomialBase<5> fFinal_y_poly_t;
      
      typedef SimplePolynomial<4> f1_z_poly_t;
      typedef SimplePolynomial<4> f2_z_poly_t;
      typedef SimplePolynomial<4> f3_z_poly_t;
      typedef SimplePolynomial<4> f4_z_poly_t;
      typedef SimplePolynomialBase<3> fFinal_z_poly_t;
    
      std::vector<TGraph *> g1_Ex { 7, nullptr };
      std::vector<TGraph *> g2_Ex { 7, nullptr };
      std::vector<TGraph *> g3_Ex { 7, nullptr };
      std::vector<TGraph *> g4_Ex { 7, nullptr };
      std::vector<TGraph *> g5_Ex { 7, nullptr };
      
      std::vector<TGraph *> g1_Ey { 7, nullptr };
      std::vector<TGraph *> g2_Ey { 7, nullptr };
      std::vector<TGraph *> g3_Ey { 7, nullptr };
      std::vector<TGraph *> g4_Ey { 7, nullptr };
      std::vector<TGraph *> g5_Ey { 7, nullptr };
      std::vector<TGraph *> g6_Ey { 7, nullptr };
      
      std::vector<TGraph *> g1_Ez { 7, nullptr };
      std::vector<TGraph *> g2_Ez { 7, nullptr };
      std::vector<TGraph *> g3_Ez { 7, nullptr };
      std::vector<TGraph *> g4_Ez { 7, nullptr };
      
      typedef SimplePolynomial<6> f1_Ex_poly_t;
      typedef SimplePolynomial<6> f2_Ex_poly_t;
      typedef SimplePolynomial<6> f3_Ex_poly_t;
      typedef SimplePolynomial<6> f4_Ex_poly_t;
      typedef SimplePolynomial<6> f5_Ex_poly_t;
      typedef SimplePolynomialBase<4> fFinal_Ex_poly_t;
      
      typedef SimplePolynomial<5> f1_Ey_poly_t;
      typedef SimplePolynomial<5> f2_Ey_poly_t;
      typedef SimplePolynomial<5> f3_Ey_poly_t;
      typedef SimplePolynomial<5> f4_Ey_poly_t;
      typedef SimplePolynomial<5> f5_Ey_poly_t;
      typedef SimplePolynomial<5> f6_Ey_poly_t;
      typedef SimplePolynomialBase<5> fFinal_Ey_poly_t;
      
      typedef SimplePolynomial<4> f1_Ez_poly_t;
      typedef SimplePolynomial<4> f2_Ez_poly_t;
      typedef SimplePolynomial<4> f3_Ez_poly_t;
      typedef SimplePolynomial<4> f4_Ez_poly_t;
      typedef SimplePolynomialBase<3> fFinal_Ez_poly_t;
    
    /// Sorts specified graphs.
    void SortGraphs(std::vector<TGraph*> const& graphs);
    
    /// Sorts all graphs currently loaded.
    void SortAllGraphs();
    
  }; // class SpaceChargeMicroBooNE
} //namespace spacecharge



//------------------------------------------------------------------------------
//--- Template implementation
//---
template <unsigned int N>
double spacecharge::SimplePolynomialBase<N>::Eval
  (double x, double const* params)
{
  // gave up using GSL because I cant figure out how to convince CET build to link to it
  double const* param = params + NParams;
  double p = *--param;
  while (param != params) {
    p *= x;
    p += *--param;
  }
  return p;
} // spacecharge::SimplePolynomialBase<N>::Eval()

//------------------------------------------------------------------------------

#endif // SPACECHARGE_SPACECHARGEMICROBOONE_H
