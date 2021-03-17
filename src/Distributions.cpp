#include <RcppArmadillo.h>
#include <cmath>

//' Generates (degenerate) multivariate normal random variable
//'
//' @name Rmvnormal
//' @param mu Vector containing mean vector
//' @param sigma Matrix containing covariance matrix
arma::vec Rmvnormal(arma::vec mu,
                    arma::mat sigma){
  arma::vec Y = arma::randn(sigma.n_cols);
  return mu + arma::chol(sigma) * Y;
}

