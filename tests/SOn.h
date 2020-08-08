/**
 * @file SOn.h
 * @brief Class to represent rtations in n-dimensions SO(n)
 * @author Frank Dellaert
 * @date August 2020
 */

#pragma once

#include <Eigen/Dense>

namespace shonan {
using Vector = Eigen::VectorXd;
using Matrix = Eigen::MatrixXd;

class SOn {
public:
protected:
  Matrix matrix_; ///< Rotation matrix

public:
  /// @name Constructors
  /// @{

  /// Constructor from Eigen Matrix, dynamic version
  template <typename Derived>
  explicit SOn(const Eigen::MatrixBase<Derived> &R) : matrix_(R.eval()) {}

  /// @}
  /// @name Standard methods
  /// @{

  /// Return matrix
  const Matrix &matrix() const { return matrix_; }

  /// Return vectorized rotation matrix in column order.
  Vector vec(double **H = nullptr) const {
    const size_t n = matrix_.rows();
    const size_t n2 = n * n;

    // Vectorize
    Vector X(n2);
    X << Eigen::Map<const Matrix>(matrix_.data(), n2, 1);

    // If requested, calculate H as (I \oplus Q) * P,
    // where Q is the N*N rotation matrix, and P is calculated below.
    if (H) {
      throw std::runtime_error("SOn::vec jacobian not implemented.");
    }
    return X;
  }

  /// @}
  /// @name Manifold
  /// @{

  // Calculate ambient dimension n from manifold dimensionality d.
  static size_t AmbientDim(size_t d) { return (1 + std::sqrt(1 + 8 * d)) / 2; }

  /**
   * Hat operator creates Lie algebra element corresponding to d-vector, where d
   * is the dimensionality of the manifold. This function is implemented
   * recursively, and the d-vector is assumed to laid out such that the last
   * element corresponds to so(2), the last 3 to so(3), the last 6 to so(4)
   * etc... For example, the vector-space isomorphic to so(5) is laid out as:
   *   a b c d | u v w | x y | z
   * where the latter elements correspond to "telescoping" sub-algebras:
   *   0 -z  y  w -d
   *   z  0 -x -v  c
   *  -y  x  0  u -b
   *  -w  v -u  0  a
   *   d -c  b -a  0
   * This scheme behaves exactly as expected for SO(2) and SO(3).
   */
  static Matrix Hat(const Vector &xi) {
    size_t n = AmbientDim(xi.size());
    if (n < 2)
      throw std::invalid_argument("SO<N>::Hat: n<2 not supported");

    Matrix X(n, n); // allocate space for n*n skew-symmetric matrix
    X.setZero();
    if (n == 2) {
      // Handle SO(2) case as recursion bottom
      assert(xi.size() == 1);
      X << 0, -xi(0), xi(0), 0;
    } else {
      // Recursively call SO(n-1) call for top-left block
      const size_t dmin = (n - 1) * (n - 2) / 2;
      X.topLeftCorner(n - 1, n - 1) = Hat(xi.tail(dmin));

      // determine sign of last element (signs alternate)
      double sign = pow(-1.0, xi.size());
      // Now fill last row and column
      for (size_t i = 0; i < n - 1; i++) {
        const size_t j = n - 2 - i;
        X(n - 1, j) = -sign * xi(i);
        X(j, n - 1) = -X(n - 1, j);
        sign = -sign;
      }
    }
    return X;
  }

  static SOn Retract(const Vector &xi, double **H = nullptr) {
    if (H) {
      throw std::runtime_error("SOn::Retract jacobian not implemented.");
    }
    const Matrix X = Hat(xi / 2.0);
    size_t n = AmbientDim(xi.size());
    const auto I = Eigen::MatrixXd::Identity(n, n);
    return SOn((I + X) * (I - X).inverse());
  }
  /// @}
}; // SOn

} // namespace shonan
