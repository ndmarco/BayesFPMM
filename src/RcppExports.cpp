// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <RcppArmadillo.h>
#include <Rcpp.h>

using namespace Rcpp;

// computeMi
void computeMi(const arma::field<arma::mat>& S_obs, const arma::mat& Z, const arma::cube& phi, const int i, arma::mat& M);
RcppExport SEXP _BayesFOC_computeMi(SEXP S_obsSEXP, SEXP ZSEXP, SEXP phiSEXP, SEXP iSEXP, SEXP MSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::field<arma::mat>& >::type S_obs(S_obsSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type Z(ZSEXP);
    Rcpp::traits::input_parameter< const arma::cube& >::type phi(phiSEXP);
    Rcpp::traits::input_parameter< const int >::type i(iSEXP);
    Rcpp::traits::input_parameter< arma::mat& >::type M(MSEXP);
    computeMi(S_obs, Z, phi, i, M);
    return R_NilValue;
END_RCPP
}
// computeM
void computeM(const arma::field<arma::mat>& S_obs, const arma::mat& Z, const arma::cube& phi, arma::field<arma::mat>& M);
RcppExport SEXP _BayesFOC_computeM(SEXP S_obsSEXP, SEXP ZSEXP, SEXP phiSEXP, SEXP MSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::field<arma::mat>& >::type S_obs(S_obsSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type Z(ZSEXP);
    Rcpp::traits::input_parameter< const arma::cube& >::type phi(phiSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type M(MSEXP);
    computeM(S_obs, Z, phi, M);
    return R_NilValue;
END_RCPP
}
// compute_mi
void compute_mi(const arma::field<arma::mat>& S_obs, const arma::mat& Z, const arma::cube& phi, const arma::mat& nu, const int i, arma::vec& m);
RcppExport SEXP _BayesFOC_compute_mi(SEXP S_obsSEXP, SEXP ZSEXP, SEXP phiSEXP, SEXP nuSEXP, SEXP iSEXP, SEXP mSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::field<arma::mat>& >::type S_obs(S_obsSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type Z(ZSEXP);
    Rcpp::traits::input_parameter< const arma::cube& >::type phi(phiSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type nu(nuSEXP);
    Rcpp::traits::input_parameter< const int >::type i(iSEXP);
    Rcpp::traits::input_parameter< arma::vec& >::type m(mSEXP);
    compute_mi(S_obs, Z, phi, nu, i, m);
    return R_NilValue;
END_RCPP
}
// compute_m
void compute_m(const arma::field<arma::mat>& S_obs, const arma::mat& Z, const arma::cube& phi, const arma::mat& nu, arma::field<arma::vec>& m);
RcppExport SEXP _BayesFOC_compute_m(SEXP S_obsSEXP, SEXP ZSEXP, SEXP phiSEXP, SEXP nuSEXP, SEXP mSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::field<arma::mat>& >::type S_obs(S_obsSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type Z(ZSEXP);
    Rcpp::traits::input_parameter< const arma::cube& >::type phi(phiSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type nu(nuSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::vec>& >::type m(mSEXP);
    compute_m(S_obs, Z, phi, nu, m);
    return R_NilValue;
END_RCPP
}
// updatePi
void updatePi(const arma::mat& Z, const double alpha, const int K, const int iter, arma::mat& pi);
RcppExport SEXP _BayesFOC_updatePi(SEXP ZSEXP, SEXP alphaSEXP, SEXP KSEXP, SEXP iterSEXP, SEXP piSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::mat& >::type Z(ZSEXP);
    Rcpp::traits::input_parameter< const double >::type alpha(alphaSEXP);
    Rcpp::traits::input_parameter< const int >::type K(KSEXP);
    Rcpp::traits::input_parameter< const int >::type iter(iterSEXP);
    Rcpp::traits::input_parameter< arma::mat& >::type pi(piSEXP);
    updatePi(Z, alpha, K, iter, pi);
    return R_NilValue;
END_RCPP
}
// g_ldet
double g_ldet(const arma::mat& M);
RcppExport SEXP _BayesFOC_g_ldet(SEXP MSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::mat& >::type M(MSEXP);
    rcpp_result_gen = Rcpp::wrap(g_ldet(M));
    return rcpp_result_gen;
END_RCPP
}
// lpdf_z
double lpdf_z(const arma::mat& M, const arma::vec& m, const arma::mat& tilde_M, const arma::vec& tilde_m, const arma::vec& f_obs, const arma::vec& f_star, const double pi_l, const double z_il, arma::mat pinv_M, arma::mat pinv_tilde_M);
RcppExport SEXP _BayesFOC_lpdf_z(SEXP MSEXP, SEXP mSEXP, SEXP tilde_MSEXP, SEXP tilde_mSEXP, SEXP f_obsSEXP, SEXP f_starSEXP, SEXP pi_lSEXP, SEXP z_ilSEXP, SEXP pinv_MSEXP, SEXP pinv_tilde_MSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::mat& >::type M(MSEXP);
    Rcpp::traits::input_parameter< const arma::vec& >::type m(mSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type tilde_M(tilde_MSEXP);
    Rcpp::traits::input_parameter< const arma::vec& >::type tilde_m(tilde_mSEXP);
    Rcpp::traits::input_parameter< const arma::vec& >::type f_obs(f_obsSEXP);
    Rcpp::traits::input_parameter< const arma::vec& >::type f_star(f_starSEXP);
    Rcpp::traits::input_parameter< const double >::type pi_l(pi_lSEXP);
    Rcpp::traits::input_parameter< const double >::type z_il(z_ilSEXP);
    Rcpp::traits::input_parameter< arma::mat >::type pinv_M(pinv_MSEXP);
    Rcpp::traits::input_parameter< arma::mat >::type pinv_tilde_M(pinv_tilde_MSEXP);
    rcpp_result_gen = Rcpp::wrap(lpdf_z(M, m, tilde_M, tilde_m, f_obs, f_star, pi_l, z_il, pinv_M, pinv_tilde_M));
    return rcpp_result_gen;
END_RCPP
}
// updateZ_i
void updateZ_i(const arma::field<arma::vec>& f_obs, const arma::field<arma::vec>& f_star, const arma::vec& pi, const int iter, const arma::field<arma::mat>& S_obs, const arma::field<arma::mat>& S_star, const arma::cube& phi, const arma::mat& nu, arma::field<arma::mat>& M, arma::field<arma::mat>& M_ph, arma::field<arma::mat>& pinv_M, arma::field<arma::vec>& m, arma::field<arma::vec>& m_ph, arma::mat& Z_ph, arma::field<arma::mat>& tilde_M, arma::field<arma::mat>& tilde_M_ph, arma::field<arma::mat>& pinv_tilde_M, arma::field<arma::vec>& tilde_m, arma::field<arma::vec>& tilde_m_ph, arma::field<arma::mat>& Z_plus, arma::field<arma::mat>& A_plus, arma::field<arma::mat>& C, arma::cube& Z);
RcppExport SEXP _BayesFOC_updateZ_i(SEXP f_obsSEXP, SEXP f_starSEXP, SEXP piSEXP, SEXP iterSEXP, SEXP S_obsSEXP, SEXP S_starSEXP, SEXP phiSEXP, SEXP nuSEXP, SEXP MSEXP, SEXP M_phSEXP, SEXP pinv_MSEXP, SEXP mSEXP, SEXP m_phSEXP, SEXP Z_phSEXP, SEXP tilde_MSEXP, SEXP tilde_M_phSEXP, SEXP pinv_tilde_MSEXP, SEXP tilde_mSEXP, SEXP tilde_m_phSEXP, SEXP Z_plusSEXP, SEXP A_plusSEXP, SEXP CSEXP, SEXP ZSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const arma::field<arma::vec>& >::type f_obs(f_obsSEXP);
    Rcpp::traits::input_parameter< const arma::field<arma::vec>& >::type f_star(f_starSEXP);
    Rcpp::traits::input_parameter< const arma::vec& >::type pi(piSEXP);
    Rcpp::traits::input_parameter< const int >::type iter(iterSEXP);
    Rcpp::traits::input_parameter< const arma::field<arma::mat>& >::type S_obs(S_obsSEXP);
    Rcpp::traits::input_parameter< const arma::field<arma::mat>& >::type S_star(S_starSEXP);
    Rcpp::traits::input_parameter< const arma::cube& >::type phi(phiSEXP);
    Rcpp::traits::input_parameter< const arma::mat& >::type nu(nuSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type M(MSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type M_ph(M_phSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type pinv_M(pinv_MSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::vec>& >::type m(mSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::vec>& >::type m_ph(m_phSEXP);
    Rcpp::traits::input_parameter< arma::mat& >::type Z_ph(Z_phSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type tilde_M(tilde_MSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type tilde_M_ph(tilde_M_phSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type pinv_tilde_M(pinv_tilde_MSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::vec>& >::type tilde_m(tilde_mSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::vec>& >::type tilde_m_ph(tilde_m_phSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type Z_plus(Z_plusSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type A_plus(A_plusSEXP);
    Rcpp::traits::input_parameter< arma::field<arma::mat>& >::type C(CSEXP);
    Rcpp::traits::input_parameter< arma::cube& >::type Z(ZSEXP);
    updateZ_i(f_obs, f_star, pi, iter, S_obs, S_star, phi, nu, M, M_ph, pinv_M, m, m_ph, Z_ph, tilde_M, tilde_M_ph, pinv_tilde_M, tilde_m, tilde_m_ph, Z_plus, A_plus, C, Z);
    return R_NilValue;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_BayesFOC_computeMi", (DL_FUNC) &_BayesFOC_computeMi, 5},
    {"_BayesFOC_computeM", (DL_FUNC) &_BayesFOC_computeM, 4},
    {"_BayesFOC_compute_mi", (DL_FUNC) &_BayesFOC_compute_mi, 6},
    {"_BayesFOC_compute_m", (DL_FUNC) &_BayesFOC_compute_m, 5},
    {"_BayesFOC_updatePi", (DL_FUNC) &_BayesFOC_updatePi, 5},
    {"_BayesFOC_g_ldet", (DL_FUNC) &_BayesFOC_g_ldet, 1},
    {"_BayesFOC_lpdf_z", (DL_FUNC) &_BayesFOC_lpdf_z, 10},
    {"_BayesFOC_updateZ_i", (DL_FUNC) &_BayesFOC_updateZ_i, 23},
    {NULL, NULL, 0}
};

RcppExport void R_init_BayesFOC(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
