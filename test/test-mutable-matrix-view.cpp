#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <utils-micro/matrix.hpp>

namespace {

namespace matrix = utils_micro::matrix;

constexpr int HEIGHT = 30;
constexpr int WIDTH = 30;
using MatrixDataT = float;

TEST_CASE("Create fixed shape mutable matrix view", "[matrix]") {

}

TEST_CASE("Create fixed width mutable matrix view", "[matrix]") {
}

TEMPLATE_TEST_CASE("Create mutable matrix view and get data", "[matrix]", (matrix::FixedShapeMatrix<float, HEIGHT, WIDTH>), (matrix::FixedWidthMatrix<float, WIDTH>), (utils_micro::matrix::FlatMatrix<float>)) {

}

TEMPLATE_TEST_CASE("Create mutable matrix view and set data", "[matrix]", (matrix::FixedShapeMatrix<float, HEIGHT, WIDTH>), (matrix::FixedWidthMatrix<float, WIDTH>), (utils_micro::matrix::FlatMatrix<float>)) {

}

// TODO nested view

}
