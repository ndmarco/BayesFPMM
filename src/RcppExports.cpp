// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <RcppArmadillo.h>
#include <Rcpp.h>

using namespace Rcpp;

// getCov
void getCov(const arma::rowvec& Z, const arma::cube& Phi, const arma::mat& Rho, const double& sigma_phi, arma::mat& Cov);
RcppExport SEXP _BayesFOC_getCov(SEXP ZSEXP, SEXP PhiSEXP, SEXP RhoSEXP, SEXP sigma_phiSEXP, SEXP CovSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::rowvec& >::type Z(ZSEXP);
    Rcpp::traits::input_parameter< const arma::cube& >::type Phi(PhiSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type Rho(RhoSEXP);
    Rcpp::traits::input_parameter< const double& >::type sigma_phi(sigma_phiSEXP);
    Rcpp::traits::input_parameter< arma::mat& >::type Cov(CovSEXP);
    getCov(Z, Phi, Rho, sigma_phi, Cov);
    return R_NilValue;
END_RCPP
}
// TestUpdateZ
Rcpp::List TestUpdateZ();
RcppExport SEXP _BayesFOC_TestUpdateZ() {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    rcpp_result_gen = Rcpp::wrap(TestUpdateZ());
    return rcpp_result_gen;
END_RCPP
}
// TestUpdatePi
Rcpp::List TestUpdatePi();
RcppExport SEXP _BayesFOC_TestUpdatePi() {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    rcpp_result_gen = Rcpp::wrap(TestUpdatePi());
    return rcpp_result_gen;
END_RCPP
}
// TestField
Rcpp::List TestField();
RcppExport SEXP _BayesFOC_TestField() {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    rcpp_result_gen = Rcpp::wrap(TestField());
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_BayesFOC_getCov", (DL_FUNC) &_BayesFOC_getCov, 5},
    {"_BayesFOC_TestUpdateZ", (DL_FUNC) &_BayesFOC_TestUpdateZ, 0},
    {"_BayesFOC_TestUpdatePi", (DL_FUNC) &_BayesFOC_TestUpdatePi, 0},
    {"_BayesFOC_TestField", (DL_FUNC) &_BayesFOC_TestField, 0},
    {NULL, NULL, 0}
};

RcppExport void R_init_BayesFOC(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
