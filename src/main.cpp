//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-04-15.
//
// This file is distributed under the MIT License.
// License text is included with the source distribution.
//****************************************************************************
#include <fstream>
#include <iostream>
#include <sstream>
#include <Argos/Argos.hpp>
#include <GridLib/ReadGrid.hpp>
#include "ReadOnlyStreamBuffer.hpp"
#include "Geoid.hpp"

[[nodiscard]] argos::ParsedArguments parse_args(int argc, char** argv)
{
    using namespace argos;
    return ArgumentParser()
        .add(Arg("input file")
            .help("An input file in XYZ format. Each line in the file"
                " must have a coordinate consisting of three floating point"
                " numbers separated by spaces: latitude and longitude in"
                " degrees and elevation in meters. If the name is '-',"
                " the input is read from stdin."))
        .add(Arg("output file")
            .optional()
            .help("A XYZ output file where the elevations have been"
                " converted from the NN2000 geoid to the WGS-84 ellipsoid"))
        .parse(argc, argv);
}

[[nodiscard]] GridLib::Grid get_geoid_grid()
{
    Utilities::ReadOnlyStreamBuffer buffer(GEOID_TIFF, GEOID_TIFF_SIZE);
    std::istream stream(&buffer);
    return read_grid(stream, GridLib::GridFileType::GEOTIFF);
}

class Input
{
public:
    explicit Input(const std::string& path)
    {
        if (path == "-")
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
        double ellipsoid_elevation = elevation - get_elevation(grid.view(), grid_pos);
        if (std::isnan(ellipsoid_elevation))
            throw std::runtime_error(
                "No geoid data for point on line " + std::to_string(line_no) + ".");

        output << lat << " " << lon << " " << ellipsoid_elevation << "\n";
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
