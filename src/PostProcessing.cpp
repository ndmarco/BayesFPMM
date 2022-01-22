#include <RcppArmadillo.h>
#include <splines2Armadillo.h>
#include <cmath>
#include <BayesFPMM.h>

//' Calculates the credible interval for the mean (Functional Data)
//'
//' This function calculates a credible interval with the user specified coverage.
//' In order to run this function, the directory of the posterior samples needs
//' to be specified. The function will return the credible intervals and the median
//' posterior estimate of the mean function at the time points specified by the
//' user (\code{time} variable). The user can specify if they would like the algorithm
//' to automatically rescale the parameters for interpretability (suggested). If
//' the user chooses to rescale, then all class memberships will be rescaled so
//' that at least one observation is in only one class. The user can also specify
//' if they want pointwise credible intervals or simultaneous credible intervals.
//' The simultaneous intervals will likely be wider than the pointwise credible
//' intervals.
//'
//' @name FMeanCI
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Int containing the number of files per parameter
//' @param time Vector containing time points of interest
//' @param basis_degree Int containing the degree of B-splines used
//' @param boundary_knots Vector containing the boundary points of our index domain of interest
//' @param internal_knots Vector location of internal knots for B-splines
//' @param k Int containing the cluster group of which you want to get the credible interval for
//' @param alpha Double specifying the percentile of the credible interval ((1 - alpha) * 100 percent)
//' @param rescale Boolean indicating whether or not we should rescale the Z variables so that there is at least one observation almost completely in one group
//' @param simultaneous Boolean indicating whether or not the credible intervals should be simulatneous credible intevals or pointwise credible intervals
//' @return CI list containing the credible interval for the mean function, as well as the median posterior estimate of the mean function
//'
//' @section Warning:
//' The following must be true:
//' \describe{
//'   \item{\code{n_files}}{must be an integer larger than or equal to 1}
//'   \item{\code{basis_degree}}{must be an integer larger than or equal to 1}
//'   \item{\code{internal_knots}}{must lie in the range of \code{boundary_knots}}
//'   \item{\code{k}}{must be an integer larger than 1 and less than or equal to the number of clusters in the model}
//'   \item{\code{alpha}}{must be between 0 and 1}
//' }
//'
//' @examples
//' ## Set Hyperparameters
//' dir <- system.file("test-data","", package = "BayesFPMM")
//' n_files <- 1
//' time <- seq(0, 990, 10)
//' k <- 2
//' basis_degree <- 3
//' boundary_knots <- c(0, 1000)
//' internal_knots <- c(200, 400, 600, 800)
//'
//' ## Get Estimates of Z and nu
//' CI <- FMeanCI(dir, n_files, time, basis_degree, boundary_knots, internal_knots, k)
//'
//' @export
// [[Rcpp::export]]
Rcpp::List FMeanCI(const std::string dir,
                   const int n_files,
                   const arma::vec time,
                   const int basis_degree,
                   const arma::vec boundary_knots,
                   const arma::vec internal_knots,
                   const int k,
                   const double alpha = 0.05,
                   const bool rescale = true,
                   const bool simultaneous = false){
  arma::cube nu_i;
  nu_i.load(dir + "Nu0.txt");
  arma::cube nu_samp = arma::zeros(nu_i.n_rows, nu_i.n_cols, nu_i.n_slices * n_files);
  nu_samp.subcube(0, 0, 0, nu_i.n_rows-1, nu_i.n_cols-1, nu_i.n_slices-1) = nu_i;
  for(int i = 1; i < n_files; i++){
    nu_i.load(dir + "Nu" + std::to_string(i) +".txt");
    nu_samp.subcube(0, 0,  nu_i.n_slices*i, nu_i.n_rows-1, nu_i.n_cols-1,
                    (nu_i.n_slices)*(i+1) - 1) = nu_i;
  }
  splines2::BSpline bspline;

  bspline = splines2::BSpline(time, internal_knots, basis_degree,
                              boundary_knots);
  // Get Basis matrix (100 x 8)
  arma::mat bspline_mat{bspline.basis(true)};

  // Make B_obs
  arma::mat B = bspline_mat;
  arma::mat f_samp = arma::zeros(nu_samp.n_slices, time.n_elem);

  // Initialize placeholders
  arma::vec CI_Upper = arma::zeros(time.n_elem);
  arma::vec CI_50 = arma::zeros(time.n_elem);
  arma::vec CI_Lower = arma::zeros(time.n_elem);

  if(simultaneous == false){
    if(rescale == true){
      // Get Z matrix
      arma::cube Z_i;
      Z_i.load(dir + "Z0.txt");
      arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
      Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
      for(int i = 1; i < n_files; i++){
        Z_i.load(dir + "Z" + std::to_string(i) +".txt");
        Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
      }
      // rescale Z and nu
      arma::mat transform_mat;
      arma::vec ph = arma::zeros(Z_samp.n_rows);
      for(int j = 0; j < Z_samp.n_slices; j++){
        transform_mat = arma::zeros(Z_samp.n_cols, Z_samp.n_cols);
        int max_ind = 0;
        for(int i = 0; i < Z_samp.n_cols; i++){
          for(int l = 0; l < Z_samp.n_rows; l++){
            ph(l) = Z_samp(l,i,j);
          }
          max_ind = arma::index_max(ph);
          transform_mat.row(i) = Z_samp.slice(j).row(max_ind);
        }
        nu_samp.slice(j) = arma::pinv(transform_mat) * nu_samp.slice(j);
      }

      for(int i = 0; i < nu_samp.n_slices; i++){
        f_samp.row(i) = (B * nu_samp.slice(i).row(k-1).t()).t();
      }
    } else{
      for(int i = 0; i < nu_samp.n_slices; i++){
        f_samp.row(i) = (B * nu_samp.slice(i).row(k-1).t()).t();
      }
    }

    arma::vec p = {alpha/2, 0.5, 1 - (alpha/2)};
    arma::vec q = arma::zeros(3);

    for(int i = 0; i < time.n_elem; i++){
      q = arma::quantile(f_samp.col(i), p);
      CI_Lower(i) = q(0);
      CI_50(i) = q(1);
      CI_Upper(i) = q(2);
    }
  }else{
    if(rescale == true){
      // Get Z matrix
      arma::cube Z_i;
      Z_i.load(dir + "Z0.txt");
      arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
      Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
      for(int i = 1; i < n_files; i++){
        Z_i.load(dir + "Z" + std::to_string(i) +".txt");
        Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
      }
      // rescale Z and nu
      arma::mat transform_mat;
      arma::vec ph = arma::zeros(Z_samp.n_rows);
      for(int j = 0; j < Z_samp.n_slices; j++){
        transform_mat = arma::zeros(Z_samp.n_cols, Z_samp.n_cols);
        int max_ind = 0;
        for(int i = 0; i < Z_samp.n_cols; i++){
          for(int l = 0; l < Z_samp.n_rows; l++){
            ph(l) = Z_samp(l,i,j);
          }
          max_ind = arma::index_max(ph);
          transform_mat.row(i) = Z_samp.slice(j).row(max_ind);
        }
        nu_samp.slice(j) = arma::pinv(transform_mat) * nu_samp.slice(j);
      }
      arma::mat f_samp = arma::zeros(nu_samp.n_slices, time.n_elem);
      for(int i = 0; i < nu_samp.n_slices; i++){
        f_samp.row(i) = (B * nu_samp.slice(i).row(k-1).t()).t();
      }
      arma::rowvec f_mean = arma::mean(f_samp, 0);
      arma::rowvec f_sd = arma::stddev(f_samp, 0, 0);

      arma::vec C = arma::zeros(nu_samp.n_slices);
      arma::vec ph1 = arma::zeros(time.n_elem);
      for(int i = 0; i < nu_samp.n_slices; i++){
        for(int j = 0; j < time.n_elem; j++){
          ph1(j) = std::abs((f_samp(i,j) - f_mean(j)) / f_sd(j));
        }
        C(i) = arma::max(ph1);
      }

      arma::vec p = {1 - alpha};
      arma::vec q = arma::zeros(1);
      q = arma::quantile(C, p);

      for(int i = 0; i < time.n_elem; i++){
        CI_Lower(i) = f_mean(i) - q(0) * f_sd(i);
        CI_50(i) = f_mean(i);
        CI_Upper(i) =  f_mean(i) + q(0) * f_sd(i);
      }
    }else{
      arma::mat f_samp = arma::zeros(nu_samp.n_slices, time.n_elem);
      for(int i = 0; i < nu_samp.n_slices; i++){
        f_samp.row(i) = (B * nu_samp.slice(i).row(k-1).t()).t();
      }
      arma::rowvec f_mean = arma::mean(f_samp, 0);
      arma::rowvec f_sd = arma::stddev(f_samp, 0, 0);

      arma::vec C = arma::zeros(nu_samp.n_slices);
      arma::vec ph1 = arma::zeros(time.n_elem);
      for(int i = 0; i < nu_samp.n_slices; i++){
        for(int j = 0; j < time.n_elem; j++){
          ph1(j) = std::abs((f_samp(i,j) - f_mean(j)) / f_sd(j));
        }
        C(i) = arma::max(ph1);
      }

      arma::vec p = {1 - alpha};
      arma::vec q = arma::zeros(1);
      q = arma::quantile(C, p);

      for(int i = 0; i < time.n_elem; i++){
        CI_Lower(i) = f_mean(i) - q(0) * f_sd(i);
        CI_50(i) = f_mean(i);
        CI_Upper(i) =  f_mean(i) + q(0) * f_sd(i);
      }
    }
  }

  Rcpp::List CI =  Rcpp::List::create(Rcpp::Named("CI_Upper", CI_Upper),
                                      Rcpp::Named("CI_50", CI_50),
                                      Rcpp::Named("CI_Lower", CI_Lower));
  return(CI);
}


//' Calculates the credible interval for the covariance (Functional Data)
//'
//' @name FCovCI
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Int containing the number of files per parameter
//' @param n_MCMC Int containing the number of saved MCMC iterations per file
//' @param time1 Vector containing time points of interest for first cluster
//' @param time2 Vector containing time points of interest for second cluster
//' @param basis_degree Int containing the degree of B-splines used
//' @param boundary_knots Vector containing the boundary points of our index domain of interest
//' @param internal_knots Vector location of internal knots for B-splines
//' @param l Int containing the 1st cluster group of which you want to get the credible interval for
//' @param m Int containing the 2nd cluster group of which you want to get the credible interval for
//' @param alpha Double specifying the percentile of the credible interval ((1 - alpha) * 100 percent)
//' @param rescale Boolean indicating whether or not we should rescale the Z variables so that there is at least one observation almost completely in one group
//' @param simultaneous Boolean indicating whether or not the credible intervals should be simulatneous credible intevals or pointwise credible intervals
//' @return CI list containing the 97.5th , 50th, and 2.5th pointwise credible functions
//' @export
// [[Rcpp::export]]
Rcpp::List FCovCI(const std::string dir,
                       const int n_files,
                       const int n_MCMC,
                       const arma::vec time1,
                       const arma::vec time2,
                       const int basis_degree,
                       const arma::vec boundary_knots,
                       const arma::vec internal_knots,
                       const int l,
                       const int m,
                       const double alpha = 0.05,
                       const bool rescale = true,
                       const bool simultaneous = false){
  // Get Phi Paramters
  arma::field<arma::cube> phi_i;
  phi_i.load(dir + "Phi0.txt");
  arma::field<arma::cube> phi_samp(n_MCMC * n_files, 1);
  for(int i = 0; i < n_MCMC; i++){
    phi_samp(i,0) = phi_i(i,0);
  }

  for(int i = 1; i < n_files; i++){
    phi_i.load(dir + "Phi" + std::to_string(i) +".txt");
    for(int j = 0; j < n_MCMC; j++){
      phi_samp((i * n_MCMC) + j, 0) = phi_i(i,0);
    }
  }

  // Make spline basis 1
  splines2::BSpline bspline1;
  bspline1 = splines2::BSpline(time1, phi_samp(0,0).n_cols);
  // Get Basis matrix (time1 x Phi.n_cols)
  arma::mat bspline_mat1{bspline1.basis(true)};
  // Make B_obs
  arma::mat B1 = bspline_mat1;

  // Make spline basis 2
  splines2::BSpline bspline2;
  bspline2 = splines2::BSpline(time2, phi_samp(0,0).n_cols);
  // Get Basis matrix (time2 x Phi.n_cols)
  arma::mat bspline_mat2{bspline2.basis(true)};
  // Make B_obs
  arma::mat B2 = bspline_mat2;

  // Initialize placeholders
  arma::mat CI_Upper = arma::zeros(time1.n_elem, time2.n_elem);
  arma::mat CI_50 = arma::zeros(time1.n_elem, time2.n_elem);
  arma::mat CI_Lower = arma::zeros(time2.n_elem, time2.n_elem);

  if(simultaneous == false){
    if(rescale == true){
      // Get Z matrix
      arma::cube Z_i;
      Z_i.load(dir + "Z0.txt");
      arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
      Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
      for(int i = 1; i < n_files; i++){
        Z_i.load(dir + "Z" + std::to_string(i) +".txt");
        Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
      }
      // rescale Z and Phi
      arma::mat transform_mat;
      arma::vec ph = arma::zeros(Z_samp.n_rows);
      for(int j = 0; j < Z_samp.n_slices; j++){
        transform_mat = arma::zeros(Z_samp.n_cols, Z_samp.n_cols);
        int max_ind = 0;
        for(int i = 0; i < Z_samp.n_cols; i++){
          for(int a = 0; a < Z_samp.n_rows; a++){
            ph(a) = Z_samp(a,i,j);
          }
          max_ind = arma::index_max(ph);
          transform_mat.row(i) = Z_samp.slice(j).row(max_ind);
        }
        for(int b = 0; b < phi_samp.n_slices; b++){
          phi_samp(j,0).slice(b) = arma::pinv(transform_mat) * phi_samp(j,0).slice(b);
        }
      }
    }
    arma::cube cov_samp = arma::zeros(time1.n_elem, time2.n_elem, n_MCMC * n_files);
    for(int i = 0; i < n_MCMC * n_files; i++){
      for(int j = 0; j < phi_samp.n_slices; j++){
        cov_samp.slice(i) = cov_samp.slice(i) + (B1 * (phi_samp(i,0).slice(j).row(l-1)).t() *
          (B2 * phi_samp(i,0).slice(j).row(m-1).t()).t());
      }
    }

    arma::vec p = {alpha/2, 0.5, 1 - (alpha/2)};
    arma::vec q = arma::zeros(3);

    arma::vec ph1 = arma::zeros(cov_samp.n_slices);
    for(int i = 0; i < time1.n_elem; i++){
      for(int j = 0; j < time2.n_elem; j++){
        ph1 = cov_samp(arma::span(i), arma::span(j), arma::span::all);
        q = arma::quantile(ph1, p);
        CI_Upper(i,j) = q(0);
        CI_50(i,j) = q(1);
        CI_Lower(i,j) = q(2);
      }
    }
  }else{
    if(rescale == true){
      // Get Z matrix
      arma::cube Z_i;
      Z_i.load(dir + "Z0.txt");
      arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
      Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
      for(int i = 1; i < n_files; i++){
        Z_i.load(dir + "Z" + std::to_string(i) +".txt");
        Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
      }
      // rescale Z and Phi
      arma::mat transform_mat;
      arma::vec ph = arma::zeros(Z_samp.n_rows);
      for(int j = 0; j < Z_samp.n_slices; j++){
        transform_mat = arma::zeros(Z_samp.n_cols, Z_samp.n_cols);
        int max_ind = 0;
        for(int i = 0; i < Z_samp.n_cols; i++){
          for(int a = 0; a < Z_samp.n_rows; a++){
            ph(a) = Z_samp(a,i,j);
          }
          max_ind = arma::index_max(ph);
          transform_mat.row(i) = Z_samp.slice(j).row(max_ind);
        }
        for(int b = 0; b < phi_samp.n_slices; b++){
          phi_samp(j,0).slice(b) = arma::pinv(transform_mat) * phi_samp(j,0).slice(b);
        }
      }
    }
    arma::cube cov_samp = arma::zeros(time1.n_elem, time2.n_elem, n_MCMC * n_files);
    for(int i = 0; i < n_MCMC * n_files; i++){
      for(int j = 0; j < phi_samp.n_slices; j++){
        cov_samp.slice(i) = cov_samp.slice(i) + (B1 * (phi_samp(i,0).slice(j).row(l-1)).t() *
          (B2 * (phi_samp(i,0).slice(j).row(m-1)).t()).t());
      }
    }

    arma::mat cov_mean = arma::mean(cov_samp, 2);
    arma::mat cov_sd = arma::zeros(time1.n_elem, time2.n_elem);
    arma::vec ph2 = arma::zeros(cov_samp.n_slices);
    for(int i = 0; i < time1.n_elem; i++){
      for(int j = 0; j < time2.n_elem; j++){
        ph2 = cov_samp(arma::span(i), arma::span(j), arma::span::all);
        cov_sd(i,j) = arma::stddev(ph2);
      }
    }

    arma::vec C = arma::zeros(cov_samp.n_slices);
    arma::mat ph1 = arma::zeros(time1.n_elem, time2.n_elem);
    for(int i = 0; i < n_MCMC * n_files; i++){
      for(int j = 0; j < time1.n_elem; j++){
        for(int k = 0; k < time2.n_elem; k++){
          ph1(j,k) = std::abs((cov_samp(j,k,i) - cov_mean(j,k)) / cov_sd(j,k));
        }
      }
      C(i) = ph1.max();
    }

    arma::vec p = {1- alpha};
    arma::vec q = arma::zeros(1);
    q = arma::quantile(C, p);

    for(int i = 0; i < time1.n_elem; i++){
      for(int j = 0; j < time2.n_elem; j++){
        CI_Lower(i,j) = cov_mean(i,j) - q(0) * cov_sd(i,j);
        CI_50(i,j) = cov_mean(i,j);
        CI_Upper(i,j) =  cov_mean(i,j) + q(0) * cov_sd(i,j);;
      }
    }
  }
  Rcpp::List CI =  Rcpp::List::create(Rcpp::Named("CI_Upper", CI_Upper),
                                      Rcpp::Named("CI_50", CI_50),
                                      Rcpp::Named("CI_Lower", CI_Lower));
  return(CI);
}

//' Calculates the Simultaneous credible interval for the covariance function between two clusters
//'
//' @name GetCovCI_S
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Int containing the number of files per parameter
//' @param n_MCMC Int containing the number of saved MCMC iterations per file
//' @param time1 Vector containing time points of interest for first cluster
//' @param time2 Vector containing time points of interest for second cluster
//' @param l Int containing the 1st cluster group of which you want to get the credible interval for
//' @param m Int containing the 2nd cluster group of which you want to get the credible interval for
//' @return CI list containing the 97.5th , 50th, and 2.5th simultaneous credible functions
//' @export
// [[Rcpp::export]]
Rcpp::List GetCovCI_S(const std::string dir,
                      const int n_files,
                      const int n_MCMC,
                      const arma::vec time1,
                      const arma::vec time2,
                      const int l,
                      const int m){

  // Get Phi Paramters
  arma::field<arma::cube> phi_i;
  phi_i.load(dir + "Phi0.txt");
  arma::field<arma::cube> phi_samp(n_MCMC * n_files, 1);
  for(int i = 0; i < n_MCMC; i++){
    phi_samp(i,0) = phi_i(i,0);
  }

  for(int i = 1; i < n_files; i++){
    phi_i.load(dir + "Phi" + std::to_string(i) +".txt");
    for(int j = 0; j < n_MCMC; j++){
      phi_samp((i * n_MCMC) + j, 0) = phi_i(i,0);
    }
  }

  // Make spline basis 1
  splines2::BSpline bspline1;
  bspline1 = splines2::BSpline(time1, phi_samp(0,0).n_cols);
  // Get Basis matrix (time1 x Phi.n_cols)
  arma::mat bspline_mat1{bspline1.basis(true)};
  // Make B_obs
  arma::mat B1 = bspline_mat1;

  // Make spline basis 2
  splines2::BSpline bspline2;
  bspline2 = splines2::BSpline(time2, phi_samp(0,0).n_cols);
  // Get Basis matrix (time2 x Phi.n_cols)
  arma::mat bspline_mat2{bspline2.basis(true)};
  // Make B_obs
  arma::mat B2 = bspline_mat2;

  arma::cube cov_samp = arma::zeros(time1.n_elem, time2.n_elem, n_MCMC * n_files);
  for(int i = 0; i < n_MCMC * n_files; i++){
    for(int j = 0; j < phi_samp.n_slices; j++){
      cov_samp.slice(i) = cov_samp.slice(i) + (B1 * (phi_samp(i,0).slice(j).row(l-1)).t() *
        (B2 * (phi_samp(i,0).slice(j).row(m-1)).t()).t());
    }
  }

  arma::mat cov_mean = arma::mean(cov_samp, 2);
  arma::mat cov_sd = arma::zeros(time1.n_elem, time2.n_elem);
  arma::vec ph = arma::zeros(cov_samp.n_slices);
  for(int i = 0; i < time1.n_elem; i++){
    for(int j = 0; j < time2.n_elem; j++){
      ph = cov_samp(arma::span(i), arma::span(j), arma::span::all);
      cov_sd(i,j) = arma::stddev(ph);
    }
  }

  arma::vec C = arma::zeros(cov_samp.n_slices);
  arma::mat ph1 = arma::zeros(time1.n_elem, time2.n_elem);
  for(int i = 0; i < n_MCMC * n_files; i++){
    for(int j = 0; j < time1.n_elem; j++){
      for(int k = 0; k < time2.n_elem; k++){
        ph1(j,k) = std::abs((cov_samp(j,k,i) - cov_mean(j,k)) / cov_sd(j,k));
      }
    }
    C(i) = ph1.max();
  }

  arma::vec p = {0.95};
  arma::vec q = arma::zeros(1);
  q = arma::quantile(C, p);

  // Initialize placeholders
  arma::mat CI_975 = arma::zeros(time1.n_elem, time2.n_elem);
  arma::mat CI_50 = arma::zeros(time1.n_elem, time2.n_elem);
  arma::mat CI_025 = arma::zeros(time2.n_elem, time2.n_elem);

  for(int i = 0; i < time1.n_elem; i++){
    for(int j = 0; j < time2.n_elem; j++){
      CI_025(i,j) = cov_mean(i,j) - q(0) * cov_sd(i,j);
      CI_50(i,j) = cov_mean(i,j);
      CI_975(i,j) =  cov_mean(i,j) + q(0) * cov_sd(i,j);;
    }
  }

  Rcpp::List CI =  Rcpp::List::create(Rcpp::Named("CI_975", CI_975),
                                      Rcpp::Named("CI_50", CI_50),
                                      Rcpp::Named("CI_025", CI_025));
  return(CI);
}

//' Calculates the credible interval for sigma squared for all types of data
//'
//' @name GetSigmaCI
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Integer containing the number of MCMC files
//' @param uci Double containing the desired percentile for the upper bound of the credible interval
//' @param lci Double containing the desired percentile for the lower bound of the credible interval
//' @returns a List containing:
//' \describe{
//'   \item{\code{nu}}{Nu samples from the MCMC chain}
//'   \item{\code{chi}}{chi samples from the MCMC chain}
//'   \item{\code{pi}}{pi samples from the MCMC chain}
//'   \item{\code{alpha_3}}{alpha_3 samples from the MCMC chain}
//'   \item{\code{A}}{A samples from MCMC chain}
//'   \item{\code{delta}}{delta samples from the MCMC chain}
//'   \item{\code{sigma}}{sigma samples from the MCMC chain}
//'   \item{\code{tau}}{tau samples from the MCMC chain}
//'   \item{\code{gamma}}{gamma samples from the MCMC chain}
//'   \item{\code{Phi}}{Phi samples from the MCMC chain}
//'   \item{\code{Z}}{Z samples from the MCMC chain}
//'   \item{\code{loglik}}{Log-likelihood plot of best performing chain}
//' }
//' @export
// [[Rcpp::export]]
Rcpp::List GetSigmaCI(const std::string dir,
                      const int n_files,
                      const double uci = 0.975,
                      const double lci = 0.025){
  if(uci <= 0.5){
    Rcpp::stop("'uci' must be a double greater than 0.5 but less than 1");
  }
  if(uci >=  1){
    Rcpp::stop("'uci' must be a double greater than 0.5 but less than 1");
  }
  if(lci >= 0.5){
    Rcpp::stop("'lci' must be a double greater than 0 but less than 0.5");
  }
  if(lci <= 0){
    Rcpp::stop("'lci' must be a double greater than 0 but less than 0.5");
  }

  arma::vec sigma_i;
  sigma_i.load(dir + "Sigma0.txt");
  arma::vec sigma_samp = arma::zeros(sigma_i.n_elem * n_files);
  sigma_samp.subvec(0, sigma_i.n_elem - 1) = sigma_i;
  for(int i = 1; i < n_files; i++){
    sigma_i.load(dir + "Sigma" + std::to_string(i) +".txt");
    sigma_samp.subvec(sigma_i.n_elem *i, (sigma_i.n_elem *(i + 1)) - 1) = sigma_i;
  }

  arma::vec p = {lci, 0.5, uci};
  arma::vec q = arma::zeros(3);
  q = arma::quantile(sigma_samp, p);

  double CI_U = q(0);
  double CI_50 = q(1);
  double CI_L = q(2);

  Rcpp::List CI =  Rcpp::List::create(Rcpp::Named("CI_U", CI_U),
                                      Rcpp::Named("CI_50", CI_50),
                                      Rcpp::Named("CI_L", CI_L));

  return(CI);
}

//' Calculates the credible interval for membership parameters Z
//'
//' This function constructs credible intervals using the MCMC samples of the
//' parameters. This function will handle high dimensional functional data,
//' functional data, and multivariate data.
//'
//' @name GetZCI
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Integer containing the number of files per parameter
//' @param uci Double containing the desired percentile for the upper bound of the credible interval
//' @param lci Double containing the desired percentile for the lower bound of the credible interval
//' @return CI List containing the 97.5th , 50th, and 2.5th credible values
//' @export
// [[Rcpp::export]]
Rcpp::List GetZCI(const std::string dir,
                  const int n_files,
                  const double uci = 0.975,
                  const double lci = 0.025){
  arma::cube Z_i;
  Z_i.load(dir + "Z0.txt");
  arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
  Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
  for(int i = 1; i < n_files; i++){
    Z_i.load(dir + "Z" + std::to_string(i) +".txt");
    Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
  }

  arma::vec p = {0.025, 0.5, 0.975};
  arma::vec q = arma::zeros(3);

  arma::mat CI_975 = arma::zeros(Z_i.n_rows, Z_i.n_cols);
  arma::mat CI_50 = arma::zeros(Z_i.n_rows, Z_i.n_cols);
  arma::mat CI_025 = arma::zeros(Z_i.n_rows, Z_i.n_cols);

  arma::vec ph(Z_samp.n_slices, arma::fill::zeros);
  for(int i = 0; i < Z_i.n_rows; i++){
    for(int j = 0; j < Z_i.n_cols; j++){
      ph = Z_samp(arma::span(i), arma::span(j), arma::span::all);
      q = arma::quantile(ph, p);
      CI_975(i,j) = q(0);
      CI_50(i,j) = q(1);
      CI_025(i,j) = q(2);
    }
  }

  Rcpp::List CI =  Rcpp::List::create(Rcpp::Named("CI_975", CI_975),
                                      Rcpp::Named("CI_50", CI_50),
                                      Rcpp::Named("CI_025", CI_025));

  return(CI);
}

//' Calculates the DIC of a model
//'
//' @name Model_DIC
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Int containing the number of files per parameter
//' @param n_MCMC Int containing the number of saved MCMC iterations per file
//' @param time Field of vectors containing time points at which the function was observed
//' @param Y Field of vectors containing observed values of the function
//' @returns DIC Double containing DIC value
//' @export
// [[Rcpp::export]]
double Model_DIC(const std::string dir,
                 const int n_files,
                 const int n_MCMC,
                 const arma::field<arma::vec> time,
                 const arma::field<arma::vec> Y){
  // Get Nu parameters
  arma::cube nu_i;
  nu_i.load(dir + "Nu0.txt");
  arma::cube nu_samp = arma::zeros(nu_i.n_rows, nu_i.n_cols, nu_i.n_slices * n_files);
  nu_samp.subcube(0, 0, 0, nu_i.n_rows-1, nu_i.n_cols-1, nu_i.n_slices-1) = nu_i;
  for(int i = 1; i < n_files; i++){
    nu_i.load(dir + "Nu" + std::to_string(i) +".txt");
    nu_samp.subcube(0, 0,  nu_i.n_slices*i, nu_i.n_rows-1, nu_i.n_cols-1,
                    (nu_i.n_slices)*(i+1) - 1) = nu_i;
  }

  // Get Phi parameters
  arma::field<arma::cube> phi_i;
  phi_i.load(dir + "Phi0.txt");
  arma::field<arma::cube> phi_samp(n_MCMC * n_files, 1);
  for(int i = 0; i < n_MCMC; i++){
    phi_samp(i,0) = phi_i(i,0);
  }

  for(int i = 1; i < n_files; i++){
    phi_i.load(dir + "Phi" + std::to_string(i) +".txt");
    for(int j = 0; j < n_MCMC; j++){
      phi_samp((i * n_MCMC) + j, 0) = phi_i(i,0);
    }
  }

  // Get Z parameters
  arma::cube Z_i;
  Z_i.load(dir + "Z0.txt");
  arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
  Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
  for(int i = 1; i < n_files; i++){
    Z_i.load(dir + "Z" + std::to_string(i) +".txt");
    Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
  }

  // Get sigma parameters
  arma::vec sigma_i;
  sigma_i.load(dir + "Sigma0.txt");
  arma::vec sigma_samp = arma::zeros(sigma_i.n_elem * n_files);
  sigma_samp.subvec(0, sigma_i.n_elem - 1) = sigma_i;
  for(int i = 1; i < n_files; i++){
    sigma_i.load(dir + "Sigma" + std::to_string(i) +".txt");
    sigma_samp.subvec(sigma_i.n_elem *i, (sigma_i.n_elem *(i + 1)) - 1) = sigma_i;
  }

  // Get chi parameters
  arma::cube chi_i;
  chi_i.load(dir + "Chi0.txt");
  arma::cube chi_samp = arma::zeros(chi_i.n_rows, chi_i.n_cols, chi_i.n_slices * n_files);
  chi_samp.subcube(0, 0, 0, chi_i.n_rows-1, chi_i.n_cols-1, chi_i.n_slices-1) = chi_i;
  for(int i = 1; i < n_files; i++){
    chi_i.load(dir + "Chi" + std::to_string(i) +".txt");
    chi_samp.subcube(0, 0,  chi_i.n_slices*i, chi_i.n_rows-1, chi_i.n_cols-1,
                     (chi_i.n_slices)*(i+1) - 1) = chi_i;
  }

  // Make spline basis
  arma::field<arma::mat> B_obs(Z_samp.n_rows, 1);
  splines2::BSpline bspline2;
  for(int i = 0; i < Z_samp.n_rows; i++)
  {
    bspline2 = splines2::BSpline(time(i,0), nu_samp.n_cols);
    // Get Basis matrix (time2 x Phi.n_cols)
    arma::mat bspline_mat2{bspline2.basis(true)};
    // Make B_obs
    B_obs(i,0) = bspline_mat2;
  }

  double expected_log_f = 0;
  for(int i = 0; i < nu_samp.n_slices; i++){
    expected_log_f = expected_log_f + BayesFPMM::calcLikelihood(Y, B_obs, nu_samp.slice(i),
                                                                phi_samp(i,0), Z_samp.slice(i),
                                                                chi_samp.slice(i), sigma_samp(i));
  }
  expected_log_f = expected_log_f / nu_samp.n_slices;

  double f_hat = 0;
  double f_hat_ij = 0;
  for(int i = 0; i < Z_samp.n_rows; i++){
    for(int j = 0; j < time(i,0).n_elem; j++){
      f_hat_ij = 0;
      for(int n = 0; n < nu_samp.n_slices; n++){
        f_hat_ij = f_hat_ij + BayesFPMM::calcDIC2(Y(i,0), B_obs(i,0), nu_samp.slice(n), phi_samp(n,0),
                                                  Z_samp.slice(n), chi_samp.slice(n), i, j,
                                                  sigma_samp(n));
      }
      f_hat = f_hat + std::log(f_hat_ij / nu_samp.n_slices);
    }
  }

  double DIC = (2 * f_hat) - (4 * expected_log_f);
  return(DIC);
}

//' Calculates the AIC of a model
//'
//' @name Model_AIC
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Int containing the number of files per parameter
//' @param n_MCMC Int containing the number of saved MCMC iterations per file
//' @param time Field of vectors containing time points at which the function was observed
//' @param Y Field of vectors containing observed values of the function
//' @returns DIC Double containing DIC value
//' @export
// [[Rcpp::export]]
double Model_AIC(const std::string dir,
                 const int n_files,
                 const int n_MCMC,
                 const arma::field<arma::vec> time,
                 const arma::field<arma::vec> Y){
  // Get Nu parameters
  arma::cube nu_i;
  nu_i.load(dir + "Nu0.txt");
  arma::cube nu_samp = arma::zeros(nu_i.n_rows, nu_i.n_cols, nu_i.n_slices * n_files);
  nu_samp.subcube(0, 0, 0, nu_i.n_rows-1, nu_i.n_cols-1, nu_i.n_slices-1) = nu_i;
  for(int i = 1; i < n_files; i++){
    nu_i.load(dir + "Nu" + std::to_string(i) +".txt");
    nu_samp.subcube(0, 0,  nu_i.n_slices*i, nu_i.n_rows-1, nu_i.n_cols-1,
                    (nu_i.n_slices)*(i+1) - 1) = nu_i;
  }

  // Get Phi parameters
  arma::field<arma::cube> phi_i;
  phi_i.load(dir + "Phi0.txt");
  arma::field<arma::cube> phi_samp(n_MCMC * n_files, 1);
  for(int i = 0; i < n_MCMC; i++){
    phi_samp(i,0) = phi_i(i,0);
  }

  for(int i = 1; i < n_files; i++){
    phi_i.load(dir + "Phi" + std::to_string(i) +".txt");
    for(int j = 0; j < n_MCMC; j++){
      phi_samp((i * n_MCMC) + j, 0) = phi_i(i,0);
    }
  }

  // Get Z parameters
  arma::cube Z_i;
  Z_i.load(dir + "Z0.txt");
  arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
  Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
  for(int i = 1; i < n_files; i++){
    Z_i.load(dir + "Z" + std::to_string(i) +".txt");
    Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
  }

  // Get sigma parameters
  arma::vec sigma_i;
  sigma_i.load(dir + "Sigma0.txt");
  arma::vec sigma_samp = arma::zeros(sigma_i.n_elem * n_files);
  sigma_samp.subvec(0, sigma_i.n_elem - 1) = sigma_i;
  for(int i = 1; i < n_files; i++){
    sigma_i.load(dir + "Sigma" + std::to_string(i) +".txt");
    sigma_samp.subvec(sigma_i.n_elem *i, (sigma_i.n_elem *(i + 1)) - 1) = sigma_i;
  }

  // Get chi parameters
  arma::cube chi_i;
  chi_i.load(dir + "Chi0.txt");
  arma::cube chi_samp = arma::zeros(chi_i.n_rows, chi_i.n_cols, chi_i.n_slices * n_files);
  chi_samp.subcube(0, 0, 0, chi_i.n_rows-1, chi_i.n_cols-1, chi_i.n_slices-1) = chi_i;
  for(int i = 1; i < n_files; i++){
    chi_i.load(dir + "Chi" + std::to_string(i) +".txt");
    chi_samp.subcube(0, 0,  chi_i.n_slices*i, chi_i.n_rows-1, chi_i.n_cols-1,
                     (chi_i.n_slices)*(i+1) - 1) = chi_i;
  }

  // Make spline basis
  arma::field<arma::mat> B_obs(Z_samp.n_rows, 1);
  splines2::BSpline bspline2;
  for(int i = 0; i < Z_samp.n_rows; i++){
    bspline2 = splines2::BSpline(time(i,0), nu_samp.n_cols);
    // Get Basis matrix (time2 x Phi.n_cols)
    arma::mat bspline_mat2{bspline2.basis(true)};
    // Make B_obs
    B_obs(i,0) = bspline_mat2;
  }

  // Estimate individual curve fit
  arma::field<arma::mat> curve_fit(Z_i.n_rows, 1);
  arma::rowvec Z_ph(Z_i.n_cols, arma::fill::zeros);
  for(int i = 0; i < Z_i.n_rows; i++){
    curve_fit(i,0) = arma::zeros(sigma_i.n_elem * n_files, Y(i,0).n_elem);
    for(int j = 0; j < curve_fit(i,0).n_rows; j++){
      Z_ph = Z_samp(arma::span(i), arma::span::all, arma::span(j));
      curve_fit(i,0).row(j) = Z_ph * nu_samp.slice(j) * B_obs(i,0).t();
      for(int k = 0; k < chi_samp.n_cols; k++){
        curve_fit(i,0).row(j) = curve_fit(i,0).row(j) + (chi_samp(i, k, j) *
          Z_ph * phi_samp(j,0).slice(k) * B_obs(i,0).t());
      }
    }
  }

  // Get mean curve fit
  arma::field<arma::rowvec> mean_curve_fit(Z_i.n_rows, 1);
  for(int i = 0; i < Z_i.n_rows; i++){
    mean_curve_fit(i,0) = arma::mean(curve_fit(i,0), 0);
  }

  // Get posterior mean of sigma^2
  double mean_sigma = arma::mean(sigma_samp);

  // Calculate Log likelihood
  double log_lik = 0;
  for(int i = 0; i < Z_samp.n_rows; i++){
    for(int j = 0; j < Y(i,0).n_elem; j++){
      log_lik = log_lik + R::dnorm(Y(i,0)(j), mean_curve_fit(i,0)(j),
                                   std::sqrt(mean_sigma), true);
    }
  }

  // Calculate AIC
  double AIC = (2 * ((Z_samp.n_rows + phi_samp.n_cols) * Z_samp.n_cols +
                2 * phi_samp.n_cols * Z_samp.n_rows * Z_samp.n_cols +
                4 + 2 * Z_samp.n_cols + 2 * phi_samp.n_cols)) - (2 * log_lik);
  return(AIC);
}

//' Calculates the BIC of a model
//'
//' @name Model_BIC
//' @param dir String containing the directory where the MCMC files are located
//' @param n_files Int containing the number of files per parameter
//' @param n_MCMC Int containing the number of saved MCMC iterations per file
//' @param time Field of vectors containing time points at which the function was observed
//' @param Y Field of vectors containing observed values of the function
//' @returns DIC Double containing DIC value
//' @export
// [[Rcpp::export]]
double Model_BIC(const std::string dir,
                 const int n_files,
                 const int n_MCMC,
                 const arma::field<arma::vec> time,
                 const arma::field<arma::vec> Y){
  // Get Nu parameters
  arma::cube nu_i;
  nu_i.load(dir + "Nu0.txt");
  arma::cube nu_samp = arma::zeros(nu_i.n_rows, nu_i.n_cols, nu_i.n_slices * n_files);
  nu_samp.subcube(0, 0, 0, nu_i.n_rows-1, nu_i.n_cols-1, nu_i.n_slices-1) = nu_i;
  for(int i = 1; i < n_files; i++){
    nu_i.load(dir + "Nu" + std::to_string(i) +".txt");
    nu_samp.subcube(0, 0,  nu_i.n_slices*i, nu_i.n_rows-1, nu_i.n_cols-1,
                    (nu_i.n_slices)*(i+1) - 1) = nu_i;
  }

  // Get Phi parameters
  arma::field<arma::cube> phi_i;
  phi_i.load(dir + "Phi0.txt");
  arma::field<arma::cube> phi_samp(n_MCMC * n_files, 1);
  for(int i = 0; i < n_MCMC; i++){
    phi_samp(i,0) = phi_i(i,0);
  }

  for(int i = 1; i < n_files; i++){
    phi_i.load(dir + "Phi" + std::to_string(i) +".txt");
    for(int j = 0; j < n_MCMC; j++){
      phi_samp((i * n_MCMC) + j, 0) = phi_i(i,0);
    }
  }

  // Get Z parameters
  arma::cube Z_i;
  Z_i.load(dir + "Z0.txt");
  arma::cube Z_samp = arma::zeros(Z_i.n_rows, Z_i.n_cols, Z_i.n_slices * n_files);
  Z_samp.subcube(0, 0, 0, Z_i.n_rows-1, Z_i.n_cols-1, Z_i.n_slices-1) = Z_i;
  for(int i = 1; i < n_files; i++){
    Z_i.load(dir + "Z" + std::to_string(i) +".txt");
    Z_samp.subcube(0, 0,  Z_i.n_slices*i, Z_i.n_rows-1, Z_i.n_cols-1, (Z_i.n_slices)*(i+1) - 1) = Z_i;
  }

  // Get sigma parameters
  arma::vec sigma_i;
  sigma_i.load(dir + "Sigma0.txt");
  arma::vec sigma_samp = arma::zeros(sigma_i.n_elem * n_files);
  sigma_samp.subvec(0, sigma_i.n_elem - 1) = sigma_i;
  for(int i = 1; i < n_files; i++){
    sigma_i.load(dir + "Sigma" + std::to_string(i) +".txt");
    sigma_samp.subvec(sigma_i.n_elem *i, (sigma_i.n_elem *(i + 1)) - 1) = sigma_i;
  }

  // Get chi parameters
  arma::cube chi_i;
  chi_i.load(dir + "Chi0.txt");
  arma::cube chi_samp = arma::zeros(chi_i.n_rows, chi_i.n_cols, chi_i.n_slices * n_files);
  chi_samp.subcube(0, 0, 0, chi_i.n_rows-1, chi_i.n_cols-1, chi_i.n_slices-1) = chi_i;
  for(int i = 1; i < n_files; i++){
    chi_i.load(dir + "Chi" + std::to_string(i) +".txt");
    chi_samp.subcube(0, 0,  chi_i.n_slices*i, chi_i.n_rows-1, chi_i.n_cols-1,
                     (chi_i.n_slices)*(i+1) - 1) = chi_i;
  }

  // Make spline basis
  arma::field<arma::mat> B_obs(Z_samp.n_rows, 1);
  splines2::BSpline bspline2;
  for(int i = 0; i < Z_samp.n_rows; i++){
    bspline2 = splines2::BSpline(time(i,0), nu_samp.n_cols);
    // Get Basis matrix (time2 x Phi.n_cols)
    arma::mat bspline_mat2{bspline2.basis(true)};
    // Make B_obs
    B_obs(i,0) = bspline_mat2;
  }

  // Estimate individual curve fit
  arma::field<arma::mat> curve_fit(Z_i.n_rows, 1);
  arma::rowvec Z_ph(Z_i.n_cols, arma::fill::zeros);
  for(int i = 0; i < Z_i.n_rows; i++){
    curve_fit(i,0) = arma::zeros(sigma_i.n_elem * n_files, Y(i,0).n_elem);
    for(int j = 0; j < curve_fit(i,0).n_rows; j++){
      Z_ph = Z_samp(arma::span(i), arma::span::all, arma::span(j));
      curve_fit(i,0).row(j) = Z_ph * nu_samp.slice(j) * B_obs(i,0).t();
      for(int k = 0; k < chi_samp.n_cols; k++){
        curve_fit(i,0).row(j) = curve_fit(i,0).row(j) + (chi_samp(i, k, j) *
          Z_ph * phi_samp(j,0).slice(k) * B_obs(i,0).t());
      }
    }
  }

  // Get mean curve fit
  arma::field<arma::rowvec> mean_curve_fit(Z_i.n_rows, 1);
  for(int i = 0; i < Z_i.n_rows; i++){
    mean_curve_fit(i,0) = arma::mean(curve_fit(i,0), 0);
  }

  // Get posterior mean of sigma^2
  double mean_sigma = arma::mean(sigma_samp);

  // Calculate Log likelihood
  double log_lik = 0;
  for(int i = 0; i < Z_samp.n_rows; i++){
    for(int j = 0; j < Y(i,0).n_elem; j++){
      log_lik = log_lik + R::dnorm(Y(i,0)(j), mean_curve_fit(i,0)(j),
                                   std::sqrt(mean_sigma), true);
    }
  }

  // Calculate AIC
  double BIC = (2 * log_lik) - (std::log(Z_samp.n_rows * Z_samp.n_cols) *
                ((Z_samp.n_rows + phi_samp.n_cols) * Z_samp.n_cols +
                2 * phi_samp.n_cols * Z_samp.n_rows * Z_samp.n_cols +
                4 + 2 * Z_samp.n_cols + 2 * phi_samp.n_cols));
  return(BIC);
}
