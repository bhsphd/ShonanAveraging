/**
 * Frobenius prior minimizes Frobenius error to a given "prior mean" matrix.
 */

#pragma once

#include "SOn.h"
#include "ceres/ceres.h"

namespace shonan {

/// Calculates the Frobenius norm between a given matrix and element of SO(n).
class FrobeniusPrior : public ceres::CostFunction {
  Matrix mean_;
  size_t n_, nn_, dim_;

public:
  /// Construct from rotation variable index and prior mean, as an nxn matrix.
  explicit FrobeniusPrior(const Matrix &mean)
      : mean_(mean), n_(mean.rows()), nn_(n_ * n_), dim_(SOn::Dimension(n_)) {
    assert(mean.cols() == n_);
    mutable_parameter_block_sizes()->push_back(nn_); // elements in SO(n) matrix
    set_num_residuals(nn_); // elements in Frobenius norm
  }

  virtual ~FrobeniusPrior() {}

  // Evaluate Frobenius error
  bool Evaluate(double const *const *values, double *error,
                double **jacobians) const override {
    Eigen::Map<const Matrix> R(values[0], n_, n_);
    // TODO(frank): use vec
    for (size_t j = 0, r = 0; j < n_; j++) {
      for (size_t i = 0; i < n_; i++) {
        error[r++] = R(i, j) - mean_(i, j);
      }
    }

    // Compute the Jacobian if asked for.
    if (jacobians != nullptr && jacobians[0] != nullptr) {
      using Jacobian = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>;
      Eigen::Map<Jacobian> H(jacobians[0], n_, n_);
      H = SOn::RetractJacobian<Eigen::RowMajor>(3);
    }
    return true;
  }
};

} // namespace shonan
