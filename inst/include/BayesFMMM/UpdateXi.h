#ifndef BayesFMMM_UPDATE_XI_H
#define BayesFMMM_UPDATE_XI_H

#include <RcppArmadillo.h>
#include <cmath>

namespace BayesFMMM{
// Updates the xi parameters for the functional covariate adjusted model
//
// @name updateXiCovariateAdj
// @param y_obs Field of Vectors containing observed time points
// @param B_obs Field of Matrices containing basis functions evaluated at observed time points
// @param nu Matrix containing current nu parameters
// @param eta Cube containing current eta parameters
// @param gamma_xi Field of Cubes containing current gamma_xi parameters
// @param tilde_tau_xi Cube containing current tilde_tau_xi parameters
// @param Phi Cubes containing current Phi parameters
// @param Z Matrix containing current Z parameters
// @param sigma_sq double containing the sigma_sq variable
// @param X Matrix containing covariates
// @param chi Matrix containing chi values
// @param iter int containing current mcmc sample
// @param m_1 Vector acting as a placeholder for m in mean vector
// @param M_1 Matrix acting as a placeholder for M in covariance
// @param xi Field of Cubes containing all mcmc samples of xi
inline void updateXiCovariateAdj(const arma::field<arma::vec>& y_obs,
                                 const arma::field<arma::mat>& B_obs,
                                 const arma::mat& nu,
                                 const arma::cube& eta,
                                 const arma::field<arma::cube>& gamma_xi,
                                 const arma::cube& tilde_tau_xi,
                                 const arma::cube& Phi,
                                 const arma::mat& Z,
                                 const arma::mat& chi,
                                 const double& sigma_sq,
                                 const arma::mat& X,
                                 const int& iter,
                                 const int& tot_mcmc_iters,
                                 arma::vec& m_1,
                                 arma::mat& M_1,
                                 arma::field<arma::cube>& xi){
  m_1.zeros();
  M_1.zeros();
  double ph = 0;

  for(int j =  0; j < Z.n_rows; j ++){
    for(int m = 0; m < xi(iter,0).n_slices; m++){
      for(int d = 0; d < X.n_cols; d++){
        m_1.zeros();
        M_1.zeros();
        for(int i = 0; i < Z.n_rows; i++){
          if(Z(i,j) != 0){
            for(int l = 0; l < y_obs(i,0).n_elem; l++){
              ph = 0;
              ph = y_obs(i,0)(l) - Z(i,j) * (arma::dot(nu.row(j),
                              B_obs(i,0).row(l)) + arma::dot(eta.slice(j) * X.row(i).t(),
                              B_obs(i,0).row(l)));
              M_1 = M_1 + Z(i,j) *  Z(i,j) * X(i,d) * X(i,d) * (chi(i,m) * chi(i,m) *
                B_obs(i,0).row(l).t() * B_obs(i,0).row(l));
              for(int k = 0; k < nu.n_rows; k++){
                for(int n = 0; n < xi(iter,0).n_slices; n++){
                  if(k == j){
                    if(n != m){
                      for(int r = 0; r < X.n_cols; r++){
                        ph = ph - (Z(i,j) * chi(i,n) * X(i,r) *
                          arma::dot(xi(iter,k).slice(n).col(r), B_obs(i,0).row(l)));
                      }
                    }
                    ph = ph - (Z(i,j) * chi(i,n) *
                      arma::dot(Phi.slice(n).row(k),B_obs(i,0).row(l)));
                    for(int r = 0; r < X.n_cols; r++){
                      if(r != d){
                        ph = ph - (Z(i,j) * chi(i,n) * X(i,r) *
                          arma::dot(xi(iter,k).slice(n).col(r), B_obs(i,0).row(l)));
                      }
                    }
                  }else{
                    ph = ph - (Z(i,k) * chi(i,n) *
                      arma::dot((Phi.slice(n).row(k) +
                      (xi(iter,k).slice(n) * X.row(i).t()).t()),  B_obs(i,0).row(l)));
                  }
                }
                if(k != j){
                  ph = ph - Z(i,k) * (arma::dot(nu.row(k), B_obs(i,0).row(l)) +
                    arma::dot(eta.slice(k) * X.row(i).t(), B_obs(i,0).row(l)));
                }
              }
              m_1 = m_1 + Z(i,j) * chi(i,m) * X(i,d) * B_obs(i,0).row(l).t() * ph;
            }
          }
        }
        m_1 = m_1 * (1 / sigma_sq);
        M_1 = M_1 * (1 / sigma_sq);

        //Add on diagonal component
        for(int k = 0; k < M_1.n_rows; k++){
          M_1(k,k) = M_1(k,k) + tilde_tau_xi(j,m,d) * gamma_xi(iter,j)(k,d,m);
        }
        arma::inv(M_1, M_1);

        //generate new sample
        xi(iter,j).slice(m).col(d) =  arma::mvnrnd(M_1 * m_1, M_1).t();
      }
    }
  }
  // Update next iteration
  if(iter < (tot_mcmc_iters - 1)){
    for(int k = 0; k < Z.n_rows; k++){
      xi(iter + 1,k) = xi(iter,k);
    }
  }
}

// Updates the xi parameters for the functional covariate adjusted model using tempered transitions
//
// @name updateXiTemperedCovariateAdj
// @param beta_i Double containing the current temperature
// @param y_obs Field of Vectors containing observed time points
// @param B_obs Field of Matrices containing basis functions evaluated at observed time points
// @param nu Matrix containing current nu parameters
// @param eta Cube containing current eta parameters
// @param gamma_xi Field of Cubes containing current gamma_xi parameters
// @param tilde_tau_xi Cube containing current tilde_tau_xi parameters
// @param Phi Cubes containing current Phi parameters
// @param Z Matrix containing current Z parameters
// @param sigma_sq double containing the sigma_sq variable
// @param X Matrix containing covariates
// @param chi Matrix containing chi values
// @param iter int containing current mcmc sample
// @param m_1 Vector acting as a placeholder for m in mean vector
// @param M_1 Matrix acting as a placeholder for M in covariance
// @param xi Field of Cubes containing all mcmc samples of xi
inline void updateXiTemperedCovariateAdj(const double& beta_i,
                                         const arma::field<arma::vec>& y_obs,
                                         const arma::field<arma::mat>& B_obs,
                                         const arma::mat& nu,
                                         const arma::cube& eta,
                                         const arma::field<arma::cube>& gamma_xi,
                                         const arma::cube& tilde_tau_xi,
                                         const arma::cube& Phi,
                                         const arma::mat& Z,
                                         const arma::mat& chi,
                                         const double& sigma_sq,
                                         const arma::mat& X,
                                         const int& iter,
                                         const int& tot_mcmc_iters,
                                         arma::vec& m_1,
                                         arma::mat& M_1,
                                         arma::field<arma::cube>& xi){
  m_1.zeros();
  M_1.zeros();
  double ph = 0;

  for(int j =  0; j < Z.n_rows; j ++){
    for(int m = 0; m < xi(iter,0).n_slices; m++){
      for(int d = 0; d < X.n_cols; d++){
        m_1.zeros();
        M_1.zeros();
        for(int i = 0; i < Z.n_rows; i++){
          if(Z(i,j) != 0){
            for(int l = 0; l < y_obs(i,0).n_elem; l++){
              ph = 0;
              ph = y_obs(i,0)(l) - Z(i,j) * (arma::dot(nu.row(j),
                              B_obs(i,0).row(l)) + arma::dot(eta.slice(j) * X.row(i).t(),
                              B_obs(i,0).row(l)));
              M_1 = M_1 + Z(i,j) *  Z(i,j) * X(i,d) * X(i,d) * (chi(i,m) * chi(i,m) *
                B_obs(i,0).row(l).t() * B_obs(i,0).row(l));
              for(int k = 0; k < nu.n_rows; k++){
                for(int n = 0; n < xi(iter,0).n_slices; n++){
                  if(k == j){
                    if(n != m){
                      for(int r = 0; r < X.n_cols; r++){
                        ph = ph - (Z(i,j) * chi(i,n) * X(i,r) *
                          arma::dot(xi(iter,k).slice(n).col(r), B_obs(i,0).row(l)));
                      }
                    }
                    ph = ph - (Z(i,j) * chi(i,n) *
                      arma::dot(Phi.slice(n).row(k),B_obs(i,0).row(l)));
                    for(int r = 0; r < X.n_cols; r++){
                      if(r != d){
                        ph = ph - (Z(i,j) * chi(i,n) * X(i,r) *
                          arma::dot(xi(iter,k).slice(n).col(r), B_obs(i,0).row(l)));
                      }
                    }
                  }else{
                    ph = ph - (Z(i,k) * chi(i,n) *
                      arma::dot((Phi.slice(n).row(k) +
                      (xi(iter,k).slice(n) * X.row(i).t()).t()),  B_obs(i,0).row(l)));
                  }
                }
                if(k != j){
                  ph = ph - Z(i,k) * (arma::dot(nu.row(k), B_obs(i,0).row(l)) +
                    arma::dot(eta.slice(k) * X.row(i).t(), B_obs(i,0).row(l)));
                }
              }
              m_1 = m_1 + Z(i,j) * chi(i,m) * X(i,d) * B_obs(i,0).row(l).t() * ph;
            }
          }
        }
        m_1 = m_1 * (beta_i / sigma_sq);
        M_1 = M_1 * (beta_i / sigma_sq);

        //Add on diagonal component
        for(int k = 0; k < M_1.n_rows; k++){
          M_1(k,k) = M_1(k,k) + tilde_tau_xi(j,m,d) * gamma_xi(iter, j)(k,d,m);
        }
        arma::inv(M_1, M_1);

        //generate new sample
        xi(iter,j).slice(m).col(d) =  arma::mvnrnd(M_1 * m_1, M_1).t();
      }
    }
  }
  // Update next iteration
  if(iter < (tot_mcmc_iters - 1)){
    for(int k = 0; k < Z.n_rows; k++){
      xi(iter + 1, k) = xi(iter, k);
    }
  }
}

// Updates the Phi parameters for the multivariate covariate adjusted multivariate model
//
// @name UpdatePhiMVCovariateAdj
// @param y_obs Matrix containing observed vectors
// @param nu Matrix containing current nu parameters
// @param eta Cube containing current eta parameters
// @param gamma_phi Cube containing current gamma_phi parameters
// @param tilde_tau_phi vector containing current tilde_tau_phi parameters
// @param xi Field of cubes containing all of the xi parameters
// @param Z Matrix containing current Z parameters
// @param sigma_sq double containing the sigma_sq variable
// @param X matrix containing the covariates
// @param chi Matrix containing chi values
// @param iter int containing current mcmc sample
// @param m_1 Vector acting as a placeholder for m in mean vector
// @param M_1 Matrix acting as a placeholder for M in covariance
// @param Phi Field of Cubes containing all mcmc samples of Phi
inline void updateXiMVCovariateAdj(const arma::mat& y_obs,
                                    const arma::mat& nu,
                                    const arma::cube& eta,
                                    const arma::field<arma::cube>& gamma_xi,
                                    const arma::cube& tilde_tau_xi,
                                    const arma::cube& Phi,
                                    const arma::mat& Z,
                                    const arma::mat& chi,
                                    const double& sigma_sq,
                                    const arma::mat& X,
                                    const int& iter,
                                    const int& tot_mcmc_iters,
                                    arma::vec& m_1,
                                    arma::mat& M_1,
                                    arma::field<arma::cube>& xi){
  m_1.zeros();
  M_1.zeros();

  for(int j =  0; j < Z.n_rows; j ++){
    for(int m = 0; m < xi(iter,0).n_slices; m++){
      for(int d = 0; d < X.n_cols; d++){
        m_1.zeros();
        M_1.zeros();
        for(int i = 0; i < Z.n_rows; i++){
          if(Z(i,j) != 0){
            arma::vec ph = arma::zeros(y_obs.n_cols);
            ph = y_obs.row(i).t() - Z(i,j) * (nu.row(j).t() +
              (eta.slice(j) * X.row(i).t()));
            M_1 = M_1 + Z(i,j) *  Z(i,j) * X(i,d) * X(i,d) * (chi(i,m) * chi(i,m) *
              arma::eye(y_obs.n_cols, y_obs.n_cols));
            for(int k = 0; k < nu.n_rows; k++){
              for(int n = 0; n < xi(iter,0).n_slices; n++){
                if(k == j){
                  if(n != m){
                    for(int r = 0; r < X.n_cols; r++){
                      ph = ph - (Z(i,j) * chi(i,n) * X(i,r) * xi(iter,k).slice(n).col(r));
                    }
                  }
                  ph = ph - (Z(i,j) * chi(i,n) * Phi.slice(n).row(k).t());
                  for(int r = 0; r < X.n_cols; r++){
                    if(r != d){
                      ph = ph - (Z(i,j) * chi(i,n) * X(i,r) * xi(iter,k).slice(n).col(r));
                    }
                  }

                }else{
                  ph = ph - (Z(i,k) * chi(i,n) * (Phi.slice(n).row(k).t() +
                    (xi(iter,k).slice(n) * X.row(i).t())));
                }
              }
              if(k != j){
                ph = ph - Z(i,k) * (nu.row(k).t() +  (eta.slice(k) * X.row(i).t()));
              }
            }
            m_1 = m_1 + Z(i,j) * chi(i,m) * ph;
          }
        }
        m_1 = m_1 * (1 / sigma_sq);
        M_1 = M_1 * (1 / sigma_sq);

        //Add on diagonal component
        for(int k = 0; k < M_1.n_rows; k++){
          M_1(k,k) = M_1(k,k) + tilde_tau_xi(j,m,d) * gamma_xi(iter, j)(k,d,m);
        }
        arma::inv(M_1, M_1);

        //generate new sample
        xi(iter,j).slice(m).col(d) =  arma::mvnrnd(M_1 * m_1, M_1).t();
      }
    }
  }
  // Update next iteration
  if(iter < (tot_mcmc_iters - 1)){
    for(int k = 0; k < Z.n_rows; k++){
      xi(iter + 1, k) = xi(iter, k);
    }
  }
}

// Updates the Phi parameters for the multivariate covariate adjusted multivariate
// using tempered transitions
//
// @name UpdatePhiTemperedMVCovariateAdj
// @param y_obs Matrix containing observed vectors
// @param nu Matrix containing current nu parameters
// @param eta Cube containing current eta parameters
// @param gamma_phi Cube containing current gamma_phi parameters
// @param tilde_tau_phi vector containing current tilde_tau_phi parameters
// @param xi Field of cubes containing all of the xi parameters
// @param Z Matrix containing current Z parameters
// @param sigma_sq double containing the sigma_sq variable
// @param X matrix containing the covariates
// @param chi Matrix containing chi values
// @param iter int containing current mcmc sample
// @param m_1 Vector acting as a placeholder for m in mean vector
// @param M_1 Matrix acting as a placeholder for M in covariance
// @param Phi Field of Cubes containing all mcmc samples of Phi
inline void updateXiTemperedMVCovariateAdj(const double& beta_i,
                                           const arma::mat& y_obs,
                                           const arma::mat& nu,
                                           const arma::cube& eta,
                                           const arma::field<arma::cube>& gamma_xi,
                                           const arma::cube& tilde_tau_xi,
                                           const arma::cube& Phi,
                                           const arma::mat& Z,
                                           const arma::mat& chi,
                                           const double& sigma_sq,
                                           const arma::mat& X,
                                           const int& iter,
                                           const int& tot_mcmc_iters,
                                           arma::vec& m_1,
                                           arma::mat& M_1,
                                           arma::field<arma::cube>& xi){
  m_1.zeros();
  M_1.zeros();

  for(int j =  0; j < Z.n_rows; j ++){
    for(int m = 0; m < xi(iter,0).n_slices; m++){
      for(int d = 0; d < X.n_cols; d++){
        m_1.zeros();
        M_1.zeros();
        for(int i = 0; i < Z.n_rows; i++){
          if(Z(i,j) != 0){
            arma::vec ph = arma::zeros(y_obs.n_cols);
            ph = y_obs.row(i).t() - Z(i,j) * (nu.row(j).t() +
              (eta.slice(j) * X.row(i).t()));
            M_1 = M_1 + Z(i,j) *  Z(i,j) * X(i,d) * X(i,d) * (chi(i,m) * chi(i,m) *
              arma::eye(y_obs.n_cols, y_obs.n_cols));
            for(int k = 0; k < nu.n_rows; k++){
              for(int n = 0; n < xi(iter,0).n_slices; n++){
                if(k == j){
                  if(n != m){
                    for(int r = 0; r < X.n_cols; r++){
                      ph = ph - (Z(i,j) * chi(i,n) * X(i,r) * xi(iter,k).slice(n).col(r));
                    }
                  }
                  ph = ph - (Z(i,j) * chi(i,n) * Phi.slice(n).row(k).t());
                  for(int r = 0; r < X.n_cols; r++){
                    if(r != d){
                      ph = ph - (Z(i,j) * chi(i,n) * X(i,r) * xi(iter,k).slice(n).col(r));
                    }
                  }

                }else{
                  ph = ph - (Z(i,k) * chi(i,n) * (Phi.slice(n).row(k).t() +
                    (xi(iter,k).slice(n) * X.row(i).t())));
                }
              }
              if(k != j){
                ph = ph - Z(i,k) * (nu.row(k).t() +  (eta.slice(k) * X.row(i).t()));
              }
            }
            m_1 = m_1 + Z(i,j) * chi(i,m) * ph;
          }
        }
        m_1 = m_1 * (beta_i / sigma_sq);
        M_1 = M_1 * (beta_i / sigma_sq);

        //Add on diagonal component
        for(int k = 0; k < M_1.n_rows; k++){
          M_1(k,k) = M_1(k,k) + tilde_tau_xi(j,m,d) * gamma_xi(iter, j)(k,d,m);
        }
        arma::inv(M_1, M_1);

        //generate new sample
        xi(iter,j).slice(m).col(d) =  arma::mvnrnd(M_1 * m_1, M_1).t();
      }
    }
  }
  // Update next iteration
  if(iter < (tot_mcmc_iters - 1)){
    for(int k = 0; k < Z.n_rows; k++){
      xi(iter + 1, k) = xi(iter, k);
    }
  }
}

}
#endif