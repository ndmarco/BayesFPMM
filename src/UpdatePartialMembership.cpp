#include <RcppArmadillo.h>
#include <cmath>

// [[Rcpp::depends(RcppArmadillo)]]


//' Gets log-pdf of z_i given zeta_{-z_i}
//'
//' @name lpdf_z
//' @param y_obs Vector containing y at observed time points
//' @param y_star Vector containing y at unobserved time points
//' @param B_obs Matrix containing basis functions evaluated at observed time points
//' @param B_star Matrix containing basis functions evaluated at unobserved time points
//' @param Phi Cube containing Phi parameters
//' @param nu Matrix containing nu parameters
//' @param pi vector containing the elements of pi
//' @param Z Vector containing the ith row of Z
//' @param sigma_sq double containing the sigma_sq variable
//' @return lpdf_z double contianing the log-pdf

double lpdf_z(const arma::vec& y_obs,
              const arma::mat& y_star,
              const arma::mat& B_obs,
              const arma::mat& B_star,
              const arma::cube& Phi,
              const arma::mat& nu,
              const arma::rowvec& chi,
              const arma::vec& pi,
              const arma::rowvec& Z,
              const double& alpha_3,
              const int& num,
              const double& sigma_sq){
  double lpdf = 0;
  double mean = 0;

  for(int l = 0; l < pi.n_elem; l++){
    lpdf = lpdf + Z(l) * ((alpha_3* pi(l) - 1) * std::log(Z(l)));
  }

  for(int l = 0; l < B_obs.n_rows; l++){
    mean = 0;
    for(int k = 0; k < pi.n_elem; k++){
      mean = mean + Z(k) * arma::dot(nu.row(k), B_obs.row(l).t());
      for(int n = 0; n < Phi.n_slices; n++){
        mean = mean + Z(k) * chi(n) * arma::dot(Phi.slice(n).row(k),
                        B_obs.row(l).t());
      }
    }
    lpdf = lpdf - (std::pow(y_obs(l) - mean, 2.0) / (2 * sigma_sq));
  }

  // Check to see if there are unobserved time points of interest
  if(B_star.n_rows > 0){
    for(int l = 0; l < B_star.n_rows; l++){
      mean = 0;
      for(int k = 0; k < pi.n_elem; k++){
        mean = mean + Z(k) * arma::dot(nu.row(k), B_star.row(l).t());
        for(int n = 0; n < Phi.n_slices; n++){
          mean = mean + Z(k) * chi(n) * arma::dot(Phi.slice(n).row(k),
                          B_star.row(l).t());
        }
      }
      lpdf = lpdf - (std::pow(y_star(num,l) - mean, 2.0) / (2 * sigma_sq));
    }
  }

  return lpdf;
}

//' Gets log-pdf of z_i given zeta_{-z_i} using tempered trasitions
//'
//' @name lpdf_zTempered
//' @param beta_i Double containing current temperature
//' @param y_obs Vector containing y at observed time points
//' @param y_star Vector containing y at unobserved time points
//' @param B_obs Matrix containing basis functions evaluated at observed time points
//' @param B_star Matrix containing basis functions evaluated at unobserved time points
//' @param Phi Cube containing Phi parameters
//' @param nu Matrix containing nu parameters
//' @param pi vector containing the elements of pi
//' @param Z Vector containing the ith row of Z
//' @param sigma_sq double containing the sigma_sq variable
//' @return lpdf_z double contianing the log-pdf

double lpdf_zTempered(const double& beta_i,
                      const arma::vec& y_obs,
                      const arma::mat& y_star,
                      const arma::mat& B_obs,
                      const arma::mat& B_star,
                      const arma::cube& Phi,
                      const arma::mat& nu,
                      const arma::rowvec& chi,
                      const arma::vec& pi,
                      const arma::rowvec& Z,
                      const double& alpha_3,
                      const int& num,
                      const double& sigma_sq){
  double lpdf = 0;
  double mean = 0;

  for(int l = 0; l < pi.n_elem; l++){
    lpdf = lpdf + ((alpha_3* pi(l) - 1) * std::log(Z(l)));
  }

  for(int l = 0; l < B_obs.n_rows; l++){
    mean = 0;
    for(int k = 0; k < pi.n_elem; k++){
      mean = mean + Z(k) * arma::dot(nu.row(k), B_obs.row(l).t());
      for(int n = 0; n < Phi.n_slices; n++){
        mean = mean + Z(k) * chi(n) * arma::dot(Phi.slice(n).row(k),
                        B_obs.row(l).t());
      }
    }
    lpdf = lpdf - (beta_i * (std::pow(y_obs(l) - mean, 2.0) / (2 * sigma_sq)));
  }

  // Check to see if there are unobserved time points of interest
  if(B_star.n_rows > 0){
    for(int l = 0; l < B_star.n_rows; l++){
      mean = 0;
      for(int k = 0; k < pi.n_elem; k++){
        mean = mean + Z(k) * arma::dot(nu.row(k), B_star.row(l).t());
        for(int n = 0; n < Phi.n_slices; n++){
          mean = mean + Z(k) * chi(n) * arma::dot(Phi.slice(n).row(k),
                          B_star.row(l).t());
        }
      }
      lpdf = lpdf - (beta_i * (std::pow(y_star(num,l) - mean, 2.0) / (2 * sigma_sq)));
    }
  }

  return lpdf;
}

//' Converts from the transformed space to the original parameter space
//'
//' @name convert_Z_tilde_Z
//' @param Z_tilde Row Vector containing parameters in the transformed space
//' @param Z Row Vector containing placeholder for variables in the untransformed space
//' @export
// [[Rcpp::export]]
void convert_Z_tilde_Z(const arma::vec& Z_tilde,
                       arma::vec& Z)
{
  double total_sum = 0;
  for(int i = 0; i < Z.n_elem; i++){
    total_sum = total_sum + std::exp(Z_tilde(i));
  }
  for(int i = 0; i < Z.n_elem; i++){
    Z(i) = std::exp(Z_tilde(i)) / total_sum;
  }
}

//' Updates the Z Matrix
//'
//' @name UpdateZ
//' @param y_obs Field of Vectors containing y at observed time points
//' @param y_star Field of Matrices containing y at unobserved time points at all mcmc iterations
//' @param B_obs Field of Matrices containing basis functions evaluated at observed time points
//' @param B_star Field of Matrices containing basis functions evaluated at unobserved time points
//' @param Phi Cube containing Phi parameters
//' @param nu Matrix containing nu parameters
//' @param pi Vector containing the elements of pi
//' @param sigma_sq Double containing the sigma_sq variable
//' @param rho Double containing hyperparameter for proposal of new z_i state
//' @param iter Int containing current mcmc iteration
//' @param tot_mcmc_iters Int containing total number of mcmc iterations
//' @param Z_ph Matrix that acts as a placeholder for Z
//' @param Z Cube that contains all past, current, and future MCMC draws

void updateZ_PM(const arma::field<arma::vec>& y_obs,
                const arma::field<arma::mat>& y_star,
                const arma::field<arma::mat>& B_obs,
                const arma::field<arma::mat>& B_star,
                const arma::cube& Phi,
                const arma::mat& nu,
                const arma::mat& chi,
                const arma::vec& pi,
                const double& sigma_sq,
                const int& iter,
                const int& tot_mcmc_iters,
                const double& alpha_3,
                const double& sigma_Z,
                arma::cube& Z_tilde,
                arma::vec& Z_tilde_ph,
                arma::vec& Z_ph,
                arma::cube& Z){
  double z_lpdf = 0;
  double z_new_lpdf = 0;
  double acceptance_prob = 0;
  double rand_unif_var = 0;

  for(int i = 0; i < Z.n_rows; i++){
    for(int l = 0; l < Z.n_cols; l++){
      // Propose new state
      Z_tilde_ph(l) = Z_tilde.slice(iter)(i,l) + R::rnorm(0, sigma_Z);
    }
    convert_Z_tilde_Z(Z_tilde_ph, Z_ph);

    if(i == 0){
      Rcpp::Rcout << Z_ph(0) << Z_ph(1) << Z_ph(2) << "\n";
    }
    // Get old state log pdf
    z_lpdf = lpdf_z(y_obs(i,0), y_star(i,0), B_obs(i,0), B_star(i,0),
                    Phi, nu, chi.row(i), pi, Z.slice(iter).row(i), alpha_3, i,
                    sigma_sq);

    // Get new state log pdf
    z_new_lpdf = lpdf_z(y_obs(i,0), y_star(i,0), B_obs(i,0), B_star(i,0), Phi,
                        nu, chi.row(i), pi, Z_ph.t(), alpha_3, i, sigma_sq);
    acceptance_prob = z_new_lpdf - z_lpdf;
    rand_unif_var = R::runif(0,1);

    if(log(rand_unif_var) < acceptance_prob){
      // Accept new state and update parameters
      Rcpp::Rcout << "Acccept" << "\n";
      Z.slice(iter).row(i) = Z_ph.t();
      Z_tilde.slice(iter).row(i) = Z_tilde_ph.t();
    }
  }
  // Update next iteration
  if(iter < (tot_mcmc_iters - 1)){
    Z.slice(iter + 1) = Z.slice(iter);
    Z_tilde.slice(iter + 1) = Z_tilde.slice(iter);
  }
}

//' Updates the Z Matrix using Tempered Transitions
//'
//' @name UpdateZTempered
//' @param beta_i Double containing current temperature
//' @param y_obs Field of Vectors containing y at observed time points
//' @param y_star Field of Matrices containing y at unobserved time points at all mcmc iterations
//' @param B_obs Field of Matrices containing basis functions evaluated at observed time points
//' @param B_star Field of Matrices containing basis functions evaluated at unobserved time points
//' @param Phi Cube containing Phi parameters
//' @param nu Matrix containing nu parameters
//' @param pi Vector containing the elements of pi
//' @param sigma_sq Double containing the sigma_sq variable
//' @param rho Double containing hyperparameter for proposal of new z_i state
//' @param iter Int containing current mcmc iteration
//' @param tot_mcmc_iters Int containing total number of mcmc iterations
//' @param Z_ph Matrix that acts as a placeholder for Z
//' @param Z Cube that contains all past, current, and future MCMC draws

void updateZTempered_PM(const double& beta_i,
                        const arma::field<arma::vec>& y_obs,
                        const arma::field<arma::mat>& y_star,
                        const arma::field<arma::mat>& B_obs,
                        const arma::field<arma::mat>& B_star,
                        const arma::cube& Phi,
                        const arma::mat& nu,
                        const arma::mat& chi,
                        const arma::vec& pi,
                        const double& sigma_sq,
                        const int& iter,
                        const int& tot_mcmc_iters,
                        const double& alpha_3,
                        const double& sigma_Z,
                        arma::cube& Z_tilde,
                        arma::vec& Z_tilde_ph,
                        arma::vec& Z_ph,
                        arma::cube& Z){
  double z_lpdf = 0;
  double z_new_lpdf = 0;
  double acceptance_prob = 0;
  double rand_unif_var = 0;

  for(int i = 0; i < Z.n_rows; i++){
    for(int l = 0; l < Z.n_cols; l++){
      // Propose new state
      Z_tilde_ph(l) = Z_tilde.slice(iter)(i,l) + R::rnorm(0, sigma_Z);
    }
    convert_Z_tilde_Z(Z_tilde_ph, Z_ph);

    // Get old state log pdf
    z_lpdf = lpdf_zTempered(beta_i, y_obs(i,0), y_star(i,0), B_obs(i,0),
                            B_star(i,0), Phi, nu, chi.row(i), pi,
                            Z.slice(iter).row(i), alpha_3, i, sigma_sq);

    // Get new state log pdf
    z_new_lpdf = lpdf_zTempered(beta_i, y_obs(i,0), y_star(i,0), B_obs(i,0),
                                B_star(i,0), Phi,  nu, chi.row(i), pi,
                                Z_ph, alpha_3, i, sigma_sq);
    acceptance_prob = z_new_lpdf - z_lpdf;
    rand_unif_var = R::runif(0,1);

    if(log(rand_unif_var) < acceptance_prob){
      // Accept new state and update parameters
      Z.slice(iter).row(i) = Z_ph.t();
      Z_tilde.slice(iter).row(i) = Z_tilde_ph.t();
    }
  }

  // Update next iteration
  if(iter < (tot_mcmc_iters - 1)){
    Z.slice(iter + 1) = Z.slice(iter);
    Z_tilde.slice(iter + 1) = Z_tilde.slice(iter);
  }
}