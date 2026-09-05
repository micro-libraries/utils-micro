#ifndef UTILS_MICRO_CONFIG_H
#define UTILS_MICRO_CONFIG_H
#include <utils-micro/matrix.hpp>

/* To create some FFI for a template library, it's necessary to constrain it to manipulation of some complete types.
 * A config file specifies these types through their template parameters.
 *
 * For the library utils-micro, the following types and functions are provided in FFI:
 * - a config-specified type InternalMatrixT (such as flat matrix or nested matrix over vector container) of a config-specified ScalarT
 * - a corresponding config-specified type MatrixViewT of ScalarT
 * - empty matrix constructor
 * - matrix view constructor over given data & clone
 * - matrix getters and setters
 * - Fourier transform of complex arrays or matrices
 */

typedef float ScalarT;

typedef utils_micro::matrix::NestedMatrix<ScalarT, std::vector> InternalMatrixT;

typedef utils_micro::matrix::Traits<InternalMatrixT>::View InternalMatrixViewT;

#endif  // UTILS_MICRO_CONFIG_H
