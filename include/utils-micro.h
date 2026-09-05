#ifndef UTILS_MICRO_H
#define UTILS_MICRO_H

extern "C" {

typedef int SizeT;

typedef struct Shape {
    SizeT height;
    SizeT width;
} Shape;

typedef struct Matrix {
    void * ptr;
} Matrix;

typedef struct MatrixView {
    void * ptr;
} MatrixView;

typedef struct FourierTransformResult {
    Matrix magnitude;
    Matrix phase;
} FourierTransformResult;

Matrix createMatrix(Shape shape);

MatrixView createMatrixViewFromData(Shape shape, const ScalarT ** data);

Matrix cloneMatrix(MatrixView view);

void destroyMatrix(Matrix matrix);

void destroyMatrixView(MatrixView view);

MatrixView useMatrixAsView(Matrix matrix);

ScalarT getMatrixElement(MatrixView view, int row, int column);

void setMatrixElement(Matrix matrix, int row, int column, ScalarT value);

// fourier transform: MatrixView<ScalarT> -> 2 matrices of magnitude and phase

FourierTransformResult calculateFourierTransform(MatrixView view, bool inverse);

}

#endif  // UTILS_MICRO_H
