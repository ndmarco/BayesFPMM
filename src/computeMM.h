#ifndef COMPUTEMM_H
#define COMPUTEMM_H

#include <RcppArmadillo.h>

void computeMi(const arma::field<arma::mat>& S_obs, const arma::mat& Z,
               const arma::cube& phi, const int i, arma::mat& M);
void computeM(const arma::field<arma::mat>& S_obs, const arma::mat& Z,
              const arma::cube& phi, arma::cube& M);
void compute_mi(const arma::field<arma::mat>& S_obs, const arma::mat& Z,
                const arma::cube& phi, const arma::mat& nu, const int i,
                arma::vec& m);
void compute_m(const arma::field<arma::mat>& S_obs, const arma::mat& Z,
               const arma::cube& phi, const arma::mat& nu, arma::mat& m);

#endif
