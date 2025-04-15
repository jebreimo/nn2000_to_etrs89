//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-04-15.
//
// This file is distributed under the MIT License.
// License text is included with the source distribution.
//****************************************************************************
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <Argos/Argos.hpp>
#include <GridLib/ReadGrid.hpp>
#include "ReadOnlyStreamBuffer.hpp"
#include "Geoid.hpp"

std::string get_geoid_info();

[[nodiscard]] argos::ParsedArguments parse_args(int argc, char** argv)
{
    using namespace argos;
    return ArgumentParser()
        .about("Convert elevations from the NN2000 geoid to the ETRS-89 ellipsoid.")
        .add(Arg("input file")
            .help("An input file in XYZ format. Each line in the file"
                " must have a coordinate consisting of three floating point"
                " numbers separated by spaces: latitude and longitude in"
                " degrees and elevation in meters. If the name is '-',"
                " the input is read from stdin."))
        .add(Arg("output file")
            .optional()
            .help("A XYZ output file where the elevations have been"
                " converted."))
        .text(TextId::FINAL_TEXT, get_geoid_info)
        .parse(argc, argv);
}

/**
 * Reads the geoid grid from the embedded resource.
 */
[[nodiscard]] const GridLib::Grid& get_geoid_grid()
{
    static GridLib::Grid geoid_grid;
    if (geoid_grid.empty())
    {
        Utilities::ReadOnlyStreamBuffer buffer(GEOID_TIFF, GEOID_TIFF_SIZE);
        std::istream stream(&buffer);
        geoid_grid = read_grid(stream, GridLib::GridFileType::GEOTIFF);
    }
    return geoid_grid;
}

/**
 * Returns a string with information about the geoid grid.
 */
std::string get_geoid_info()
{
    std::stringstream txt;
    const auto rows = get_geoid_grid().row_count();
    const auto cols = get_geoid_grid().col_count();
    txt << "INFO ABOUT THE GEOID\n"
        << "  rows:    " << rows << "\n"
        << "  columns: " << cols << "\n";
    auto& grid = get_geoid_grid();
    auto pos0 = grid_pos_to_model_pos(grid.view(), {0, 0});
    auto pos1 = grid_pos_to_model_pos(grid.view(), {double(rows), double(cols)});
    txt << "  min. latitude:  " << std::min(pos0[1], pos1[1]) << "\n"
        << "  max. latitude:  " << std::max(pos0[1], pos1[1]) << "\n"
        << "  min. longitude: " << std::min(pos0[0], pos1[0]) << "\n"
        << "  max. longitude: " << std::max(pos0[0], pos1[0]) << "\n";
    return txt.str();
}

/**
 * Manages the input stream. If the input file is not specified or "-", it
 * reads from stdin.
 */
class Input
{
public:
    explicit Input(const std::string& path)
    {
        if (path.empty() || path == "-")
            return;

        file.open(path);
        if (!file)
            throw std::runtime_error("Could not open input file: '" + path + "'");
    }

    std::istream& stream()
    {
        return file.is_open() ? file : std::cin;
    }

private:
    std::ifstream file;
};

/**
 * Manages the output stream. If the output file is not specified, it writes
 * to stdout.
 */
class Output
{
public:
    explicit Output(const std::string& path)
    {
        if (path.empty())
            return;

        file.open(path);
        if (!file)
            throw std::runtime_error("Could not create output file: '" + path + "'");
    }

    std::ostream& stream()
    {
        return file.is_open() ? file : std::cout;
    }

private:
    std::ofstream file;
};

/**
 * Converts elevations from the NN2000 geoid to the ETRS-89 ellipsoid.
 * The input is read from the specified stream and the output is written
 * to the specified stream.
 */
void convert_elevations(GridLib::Grid& grid,
                        std::istream& input,
                        std::ostream& output)
{
    int line_no = 0;
    std::string line;
    while (std::getline(input, line))
    {
        ++line_no;
        std::istringstream iss(line);
        double lat, lon, elevation;
        if (!(iss >> lat >> lon >> elevation))
            throw std::runtime_error("Invalid input format on line " + std::to_string(line_no));

        auto grid_pos = model_pos_to_grid_pos(grid.view(), {lon, lat, 0});
        double ellipsoid_elevation = elevation + get_elevation(grid.view(), grid_pos);

        // The ellipsoid elevation is NaN when there are no data available.
        if (std::isnan(ellipsoid_elevation))
            throw std::runtime_error(
                "No geoid data for point on line " + std::to_string(line_no) + ".");

        // Write the output in the same format as the input.
        std::string lat_str, lon_str;
        iss.seekg(0, std::ios::beg);
        iss >> lat_str >> lon_str;
        output << lat_str << " " << lon_str << " " << ellipsoid_elevation << "\n";
    }
}

int main(int argc, char** argv)
{
    try
    {
        auto args = parse_args(argc, argv);
        auto geoid_grid = get_geoid_grid();

        Input input(args.value("input file").as_string());
        Output output(args.value("output file").as_string());
        convert_elevations(geoid_grid, input.stream(), output.stream());
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
