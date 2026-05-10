#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <utils-micro/matrix.hpp>

namespace {

namespace matrix = utils_micro::matrix;

constexpr int HEIGHT = 30;
constexpr int WIDTH = 30;
using MatrixDataT = float;

TEST_CASE("Create fixed shape matrix view", "[matrix]") {
    using MatrixT = matrix::FixedShapeMatrix<float, HEIGHT, WIDTH>;
    using ViewT = matrix::Traits<MatrixT>::View;

    std::vector< std::array<MatrixDataT, WIDTH> > data(HEIGHT);
    const auto * raw_data = data.data()->data();

    SECTION("Create matrix view from fixed size span") {
        utils_micro::span<const MatrixDataT, HEIGHT * WIDTH> data_fixed_span{raw_data, HEIGHT * WIDTH};
        ViewT{data_fixed_span};
    }

    SECTION("Create matrix view from fixed size span and shape") {
        utils_micro::span<const MatrixDataT, HEIGHT * WIDTH> data_fixed_span{raw_data, HEIGHT * WIDTH};
        ViewT{{matrix::Height{HEIGHT}, matrix::Width{WIDTH}}, data_fixed_span};
    }

    SECTION("Create matrix view from span") {
        utils_micro::span<const MatrixDataT> data_span{raw_data, HEIGHT * WIDTH};
        ViewT{data_span};
    }

    SECTION("Create matrix view from raw data") {
        ViewT{raw_data};
    }
}

TEST_CASE("Create fixed width matrix view", "[matrix]") {
    using MatrixT = matrix::FixedWidthMatrix<float, WIDTH>;
    using ViewT = matrix::Traits<MatrixT>::View;

    std::vector< std::array<MatrixDataT, WIDTH> > data(HEIGHT);
    const auto * raw_data = data.data()->data();

    SECTION("Create matrix view from span") {
        utils_micro::span<const MatrixDataT> data_span{raw_data, HEIGHT * WIDTH};
        ViewT{HEIGHT, data_span};
    }

    SECTION("Create matrix view from raw data") {
        ViewT{HEIGHT, raw_data};
    }
}

TEMPLATE_TEST_CASE("Create matrix view and get data", "[matrix]", (matrix::FixedShapeMatrix<float, HEIGHT, WIDTH>), (matrix::FixedWidthMatrix<float, WIDTH>), (utils_micro::matrix::FlatMatrix<float>)) {
    std::vector< std::array<MatrixDataT, WIDTH> > data(HEIGHT);
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            data[i][j] = rand()%10; // TODO better rand
        }
    }
    // TODO split out input

    using MatrixT = TestType;
    using ViewT = typename matrix::Traits<MatrixT>::View;

    const auto * raw_data = data.data()->data();

    matrix::Shape shape{matrix::Height{HEIGHT}, matrix::Width{WIDTH}};

    SECTION("Create matrix view from span") {
        utils_micro::span<const MatrixDataT> data_span{raw_data, HEIGHT * WIDTH};
        ViewT data_view{shape, data_span};
    }

    ViewT data_view(shape, data.data()->data());

    SECTION("Get shape") {
        CHECK(shape == data_view.getShape());
        CHECK(HEIGHT == data_view.getHeight());
        CHECK(WIDTH == data_view.getWidth());
        CHECK(HEIGHT * WIDTH == data_view.getSize());
    }

    SECTION("Get underlying data of view") {
        CHECK(data.data()->data() == data_view.getData().data());

        // TODO getRowData
    }

    SECTION("Get values") {
        const std::vector<matrix::Coordinates> all_coordinates{
                {matrix::Row{5}, matrix::Column{12}},
                {matrix::Row{0}, matrix::Column{0}},
                {matrix::Row{1}, matrix::Column{0}},
                {matrix::Row{HEIGHT-1}, matrix::Column{WIDTH-1}},
            };

        std::vector<MatrixDataT> expected_values;
        for (auto [r, c] : all_coordinates) { expected_values.push_back(data[r][c]); }

        std::vector<MatrixDataT> values;
        for (auto [r, c] : all_coordinates) { values.push_back(data_view.get(r, c)); };

        for (std::size_t i = 0; i != all_coordinates.size(); i++) {
            CHECK(expected_values[i] == values[i]);
        }
    }

    SECTION("Get values at fixed (constexpr) positions") {
        const auto value = data_view.template get<matrix::TemplateRowT{5}, matrix::TemplateColumnT{12}>();
        const auto expected_value = data[5][12];
        CHECK(expected_value == value);
    }
}

// TODO nested view

}
