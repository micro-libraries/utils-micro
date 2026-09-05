#include CONFIGURATION
#include <utils-micro.h>
#include <utils-micro/matrix.hpp>
#include <utils-micro/fourier.hpp>
#include <utils-micro/arithmetic/complex-number-arithmetic.hpp>

extern "C" {

Matrix createMatrix(const Shape shape) {
    return {static_cast<void *>(new InternalMatrixT{{
        .height = utils_micro::matrix::Height{shape.height},
        .width = utils_micro::matrix::Width{shape.width}
    }})};
}

MatrixView createMatrixViewFromData(const Shape shape, const ScalarT ** data) {
    return {static_cast<void *>(new InternalMatrixViewT{{
        .height = utils_micro::matrix::Height{shape.height},
        .width = utils_micro::matrix::Width{shape.width}
    }, data})};
}

Matrix cloneMatrix(const MatrixView view) {
    auto result = static_cast<const InternalMatrixViewT *>(view.ptr)->clone();
    return {static_cast<void *>(
        new InternalMatrixT{std::move(result)}
    )};
}

void destroyMatrix(Matrix matrix) {
    delete static_cast<const InternalMatrixT *>(matrix.ptr);
    matrix.ptr = nullptr;
}

void destroyMatrixView(MatrixView view) {
    delete static_cast<const InternalMatrixViewT *>(view.ptr);
    view.ptr = nullptr;
}

MatrixView useMatrixAsView(const Matrix matrix) {
    return {static_cast<void *>(
        static_cast<InternalMatrixViewT *>(
            static_cast<InternalMatrixT *>(matrix.ptr)
        )
    )};
}

ScalarT getMatrixElement(const MatrixView view, const int row, const int column) {
    return static_cast<const InternalMatrixViewT *>(view.ptr)->get(utils_micro::matrix::Row{row}, utils_micro::matrix::Column{column});
}

void setMatrixElement(const Matrix matrix, const int row, const int column, const ScalarT value) {
    static_cast<InternalMatrixT *>(matrix.ptr)->set(utils_micro::matrix::Row{row}, utils_micro::matrix::Column{column}, value);
}

FourierTransformResult calculateFourierTransform(const MatrixView view, const bool inverse) {
    using ArithmeticT = utils_micro::fourier::RingArithmetic<utils_micro::arithmetic::ComplexNumberArithmetic<float>>;
    utils_micro::fourier::FastFourierTransform<10, ArithmeticT> fourier_transform;
    auto transformed_matrix = fourier_transform.run<InternalMatrixT>(*static_cast<const InternalMatrixViewT *>(view.ptr), inverse);
    return {};
    // return {.magnitude = utils_micro::matrix::operations::AbsoluteValue<>::abs(transformed_matrix), .phase = utils_micro::matrix::operations::Phase<>::phase(transformed_matrix)};
}

}
