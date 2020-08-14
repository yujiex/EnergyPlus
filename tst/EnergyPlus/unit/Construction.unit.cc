// EnergyPlus, Copyright (c) 1996-2020, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Google test headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/FileSystem.hh>

#include "Fixtures/EnergyPlusFixture.hh"

#include <nlohmann/json.hpp>

#include <fstream>
#include <chrono>

std::ofstream static file("out.csv", std::ofstream::out);

namespace EnergyPlus {

void readJSONfile(std::string &filePath, nlohmann::json &j) {
    if (!FileSystem::fileExists(filePath)) {
        // if the file doesn't exist, there are no data to read
        return;
    } else {
        std::ifstream ifs(filePath);

        // read json_in data
        try {
            ifs >> j;
            ifs.close();
        } catch (...) {
            if (!j.empty()) {
                // file exists, is not empty, but failed for some other reason
                ShowWarningError(filePath + " contains invalid file format");
            }
            ifs.close();
            return;
        }
    }
}

void writeJSONfile(std::string &fPath, nlohmann::json &j) {
    std::ofstream ofs(fPath);
    ofs << std::setw(2) << j;
    ofs.close();
}

void constrToJSON(Construction::ConstructionProps &c, nlohmann::json &j) {

    // Probably should give it a name
    j["name"] = c.Name;

    // Array1D to std::vector for the CTFCross, CTFFlux, CTFInside, and CTFOutside Arrays
    std::vector<Real64> cross;
    std::vector<Real64> flux;
    std::vector<Real64> inside;
    std::vector<Real64> outside;

    for (auto v : c.CTFCross) cross.push_back(v);
    for (auto v : c.CTFFlux) flux.push_back(v);
    for (auto v : c.CTFInside) inside.push_back(v);
    for (auto v : c.CTFOutside) outside.push_back(v);

    j["cross"] = cross;
    j["flux"] = flux;
    j["inside"] = inside;
    j["outside"] = outside;
}

void JSONtoConst(nlohmann::json &j, Construction::ConstructionProps &c) {
}

TEST_F(EnergyPlusFixture, ConstructionGetInput)
{
    std::string const idf_objects = delimited_string({
        "Material,",
        "    MAT-1,      != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-2,      != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-3,      != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-4,      != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-5,      != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-6,      != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-7,      != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-8,      != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    400,        != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-9,      != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-10,     != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-11,     != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-12,     != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    200,        != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-13,     != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-14,     != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    0.01,       != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-15,     != Name",
        "    Smooth,     != Roughness",
        "    0.01,       != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Material,",
        "    MAT-16,     != Name",
        "    Smooth,     != Roughness",
        "    0.1,        != Thickness {m}",
        "    100.0,      != Conductivity {W/m-K}",
        "    3000,       != Density {kg/m3}",
        "    1500,       != Specific Heat {J/kg-K}",
        "    0.5,        != Thermal Absorptance",
        "    0.5,        != Solar Absorptance",
        "    0.5;        != Visible Absorptance",
        "",
        "Construction,",
        "    CONST-1,    !- Name",
        "    MAT-1;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-2,    !- Name",
        "    MAT-1,      !- Outside Layer",
        "    MAT-1;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-3,    !- Name",
        "    MAT-1,      !- Outside Layer",
        "    MAT-1,      !- Layer 2",
        "    MAT-1,      !- Layer 3",
        "    MAT-1;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-4,    !- Name",
        "    MAT-1,      !- Outside Layer",
        "    MAT-1,      !- Layer 2",
        "    MAT-1,      !- Layer 3",
        "    MAT-1,      !- Layer 4",
        "    MAT-1,      !- Layer 5",
        "    MAT-1;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-5,    !- Name",
        "    MAT-1,      !- Outside Layer",
        "    MAT-1,      !- Layer 2",
        "    MAT-1,      !- Layer 3",
        "    MAT-1,      !- Layer 4",
        "    MAT-1,      !- Layer 5",
        "    MAT-1,      !- Layer 6",
        "    MAT-1,      !- Layer 7",
        "    MAT-1,      !- Layer 8",
        "    MAT-1;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-6,    !- Name",
        "    MAT-2;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-7,    !- Name",
        "    MAT-2,      !- Outside Layer",
        "    MAT-2;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-8,    !- Name",
        "    MAT-2,      !- Outside Layer",
        "    MAT-2,      !- Layer 2",
        "    MAT-2,      !- Layer 3",
        "    MAT-2;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-9,    !- Name",
        "    MAT-2,      !- Outside Layer",
        "    MAT-2,      !- Layer 2",
        "    MAT-2,      !- Layer 3",
        "    MAT-2,      !- Layer 4",
        "    MAT-2,      !- Layer 5",
        "    MAT-2;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-10,   !- Name",
        "    MAT-2,      !- Outside Layer",
        "    MAT-2,      !- Layer 2",
        "    MAT-2,      !- Layer 3",
        "    MAT-2,      !- Layer 4",
        "    MAT-2,      !- Layer 5",
        "    MAT-2,      !- Layer 6",
        "    MAT-2,      !- Layer 7",
        "    MAT-2,      !- Layer 8",
        "    MAT-2;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-11,   !- Name",
        "    MAT-3;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-12,   !- Name",
        "    MAT-3,      !- Outside Layer",
        "    MAT-3;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-13,   !- Name",
        "    MAT-3,      !- Outside Layer",
        "    MAT-3,      !- Layer 2",
        "    MAT-3,      !- Layer 3",
        "    MAT-3;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-14,   !- Name",
        "    MAT-3,      !- Outside Layer",
        "    MAT-3,      !- Layer 2",
        "    MAT-3,      !- Layer 3",
        "    MAT-3,      !- Layer 4",
        "    MAT-3,      !- Layer 5",
        "    MAT-3;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-15,   !- Name",
        "    MAT-3,      !- Outside Layer",
        "    MAT-3,      !- Layer 2",
        "    MAT-3,      !- Layer 3",
        "    MAT-3,      !- Layer 4",
        "    MAT-3,      !- Layer 5",
        "    MAT-3,      !- Layer 6",
        "    MAT-3,      !- Layer 7",
        "    MAT-3,      !- Layer 8",
        "    MAT-3;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-16,   !- Name",
        "    MAT-4;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-17,   !- Name",
        "    MAT-4,      !- Outside Layer",
        "    MAT-4;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-18,   !- Name",
        "    MAT-4,      !- Outside Layer",
        "    MAT-4,      !- Layer 2",
        "    MAT-4,      !- Layer 3",
        "    MAT-4;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-19,   !- Name",
        "    MAT-4,      !- Outside Layer",
        "    MAT-4,      !- Layer 2",
        "    MAT-4,      !- Layer 3",
        "    MAT-4,      !- Layer 4",
        "    MAT-4,      !- Layer 5",
        "    MAT-4;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-20,   !- Name",
        "    MAT-4,      !- Outside Layer",
        "    MAT-4,      !- Layer 2",
        "    MAT-4,      !- Layer 3",
        "    MAT-4,      !- Layer 4",
        "    MAT-4,      !- Layer 5",
        "    MAT-4,      !- Layer 6",
        "    MAT-4,      !- Layer 7",
        "    MAT-4,      !- Layer 8",
        "    MAT-4;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-21,   !- Name",
        "    MAT-5;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-22,   !- Name",
        "    MAT-5,      !- Outside Layer",
        "    MAT-5;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-23,   !- Name",
        "    MAT-5,      !- Outside Layer",
        "    MAT-5,      !- Layer 2",
        "    MAT-5,      !- Layer 3",
        "    MAT-5;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-24,   !- Name",
        "    MAT-5,      !- Outside Layer",
        "    MAT-5,      !- Layer 2",
        "    MAT-5,      !- Layer 3",
        "    MAT-5,      !- Layer 4",
        "    MAT-5,      !- Layer 5",
        "    MAT-5;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-25,   !- Name",
        "    MAT-5,      !- Outside Layer",
        "    MAT-5,      !- Layer 2",
        "    MAT-5,      !- Layer 3",
        "    MAT-5,      !- Layer 4",
        "    MAT-5,      !- Layer 5",
        "    MAT-5,      !- Layer 6",
        "    MAT-5,      !- Layer 7",
        "    MAT-5,      !- Layer 8",
        "    MAT-5;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-26,   !- Name",
        "    MAT-6;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-27,   !- Name",
        "    MAT-6,      !- Outside Layer",
        "    MAT-6;      !- Layer 2",
        "",
//        "Construction,",
//        "    CONST-28,   !- Name",
//        "    MAT-6,      !- Outside Layer",
//        "    MAT-6,      !- Layer 2",
//        "    MAT-6,      !- Layer 3",
//        "    MAT-6;      !- Layer 4",
//        "",
//        "Construction,",
//        "    CONST-29,   !- Name",
//        "    MAT-6,      !- Outside Layer",
//        "    MAT-6,      !- Layer 2",
//        "    MAT-6,      !- Layer 3",
//        "    MAT-6,      !- Layer 4",
//        "    MAT-6,      !- Layer 5",
//        "    MAT-6;      !- Layer 6",
//        "",
//        "Construction,",
//        "    CONST-30,   !- Name",
//        "    MAT-6,      !- Outside Layer",
//        "    MAT-6,      !- Layer 2",
//        "    MAT-6,      !- Layer 3",
//        "    MAT-6,      !- Layer 4",
//        "    MAT-6,      !- Layer 5",
//        "    MAT-6,      !- Layer 6",
//        "    MAT-6,      !- Layer 7",
//        "    MAT-6,      !- Layer 8",
//        "    MAT-6;      !- Layer 9",
//        "",
        "Construction,",
        "    CONST-31,   !- Name",
        "    MAT-7;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-32,   !- Name",
        "    MAT-7,      !- Outside Layer",
        "    MAT-7;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-33,   !- Name",
        "    MAT-7,      !- Outside Layer",
        "    MAT-7,      !- Layer 2",
        "    MAT-7,      !- Layer 3",
        "    MAT-7;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-34,   !- Name",
        "    MAT-7,      !- Outside Layer",
        "    MAT-7,      !- Layer 2",
        "    MAT-7,      !- Layer 3",
        "    MAT-7,      !- Layer 4",
        "    MAT-7,      !- Layer 5",
        "    MAT-7;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-35,   !- Name",
        "    MAT-7,      !- Outside Layer",
        "    MAT-7,      !- Layer 2",
        "    MAT-7,      !- Layer 3",
        "    MAT-7,      !- Layer 4",
        "    MAT-7,      !- Layer 5",
        "    MAT-7,      !- Layer 6",
        "    MAT-7,      !- Layer 7",
        "    MAT-7,      !- Layer 8",
        "    MAT-7;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-36,   !- Name",
        "    MAT-8;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-37,   !- Name",
        "    MAT-8,      !- Outside Layer",
        "    MAT-8;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-38,   !- Name",
        "    MAT-8,      !- Outside Layer",
        "    MAT-8,      !- Layer 2",
        "    MAT-8,      !- Layer 3",
        "    MAT-8;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-39,   !- Name",
        "    MAT-8,      !- Outside Layer",
        "    MAT-8,      !- Layer 2",
        "    MAT-8,      !- Layer 3",
        "    MAT-8,      !- Layer 4",
        "    MAT-8,      !- Layer 5",
        "    MAT-8;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-40,   !- Name",
        "    MAT-8,      !- Outside Layer",
        "    MAT-8,      !- Layer 2",
        "    MAT-8,      !- Layer 3",
        "    MAT-8,      !- Layer 4",
        "    MAT-8,      !- Layer 5",
        "    MAT-8,      !- Layer 6",
        "    MAT-8,      !- Layer 7",
        "    MAT-8,      !- Layer 8",
        "    MAT-8;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-41,   !- Name",
        "    MAT-9;      !- Outside Layer",
        "",
        "Construction,",
        "    CONST-42,   !- Name",
        "    MAT-9,      !- Outside Layer",
        "    MAT-9;      !- Layer 2",
        "",
        "Construction,",
        "    CONST-43,   !- Name",
        "    MAT-9,      !- Outside Layer",
        "    MAT-9,      !- Layer 2",
        "    MAT-9,      !- Layer 3",
        "    MAT-9;      !- Layer 4",
        "",
        "Construction,",
        "    CONST-44,   !- Name",
        "    MAT-9,      !- Outside Layer",
        "    MAT-9,      !- Layer 2",
        "    MAT-9,      !- Layer 3",
        "    MAT-9,      !- Layer 4",
        "    MAT-9,      !- Layer 5",
        "    MAT-9;      !- Layer 6",
        "",
        "Construction,",
        "    CONST-45,   !- Name",
        "    MAT-9,      !- Outside Layer",
        "    MAT-9,      !- Layer 2",
        "    MAT-9,      !- Layer 3",
        "    MAT-9,      !- Layer 4",
        "    MAT-9,      !- Layer 5",
        "    MAT-9,      !- Layer 6",
        "    MAT-9,      !- Layer 7",
        "    MAT-9,      !- Layer 8",
        "    MAT-9;      !- Layer 9",
        "",
        "Construction,",
        "    CONST-46,   !- Name",
        "    MAT-10;     !- Outside Layer",
        "",
        "Construction,",
        "    CONST-47,   !- Name",
        "    MAT-10,     !- Outside Layer",
        "    MAT-10;     !- Layer 2",
        "",
        "Construction,",
        "    CONST-48,   !- Name",
        "    MAT-10,     !- Outside Layer",
        "    MAT-10,     !- Layer 2",
        "    MAT-10,     !- Layer 3",
        "    MAT-10;     !- Layer 4",
        "",
        "Construction,",
        "    CONST-49,   !- Name",
        "    MAT-10,     !- Outside Layer",
        "    MAT-10,     !- Layer 2",
        "    MAT-10,     !- Layer 3",
        "    MAT-10,     !- Layer 4",
        "    MAT-10,     !- Layer 5",
        "    MAT-10;     !- Layer 6",
        "",
//        "Construction,",
//        "    CONST-50,   !- Name",
//        "    MAT-10,     !- Outside Layer",
//        "    MAT-10,     !- Layer 2",
//        "    MAT-10,     !- Layer 3",
//        "    MAT-10,     !- Layer 4",
//        "    MAT-10,     !- Layer 5",
//        "    MAT-10,     !- Layer 6",
//        "    MAT-10,     !- Layer 7",
//        "    MAT-10,     !- Layer 8",
//        "    MAT-10;     !- Layer 9",
//        "",
        "Construction,",
        "    CONST-51,   !- Name",
        "    MAT-11;     !- Outside Layer",
        "",
        "Construction,",
        "    CONST-52,   !- Name",
        "    MAT-11,     !- Outside Layer",
        "    MAT-11;     !- Layer 2",
        "",
        "Construction,",
        "    CONST-53,   !- Name",
        "    MAT-11,     !- Outside Layer",
        "    MAT-11,     !- Layer 2",
        "    MAT-11,     !- Layer 3",
        "    MAT-11;     !- Layer 4",
        "",
        "Construction,",
        "    CONST-54,   !- Name",
        "    MAT-11,     !- Outside Layer",
        "    MAT-11,     !- Layer 2",
        "    MAT-11,     !- Layer 3",
        "    MAT-11,     !- Layer 4",
        "    MAT-11,     !- Layer 5",
        "    MAT-11;     !- Layer 6",
        "",
        "Construction,",
        "    CONST-55,   !- Name",
        "    MAT-11,     !- Outside Layer",
        "    MAT-11,     !- Layer 2",
        "    MAT-11,     !- Layer 3",
        "    MAT-11,     !- Layer 4",
        "    MAT-11,     !- Layer 5",
        "    MAT-11,     !- Layer 6",
        "    MAT-11,     !- Layer 7",
        "    MAT-11,     !- Layer 8",
        "    MAT-11;     !- Layer 9",
        "",
        "Construction,",
        "    CONST-56,   !- Name",
        "    MAT-12;     !- Outside Layer",
        "",
        "Construction,",
        "    CONST-57,   !- Name",
        "    MAT-12,     !- Outside Layer",
        "    MAT-12;     !- Layer 2",
        "",
        "Construction,",
        "    CONST-58,   !- Name",
        "    MAT-12,     !- Outside Layer",
        "    MAT-12,     !- Layer 2",
        "    MAT-12,     !- Layer 3",
        "    MAT-12;     !- Layer 4",
        "",
        "Construction,",
        "    CONST-59,   !- Name",
        "    MAT-12,     !- Outside Layer",
        "    MAT-12,     !- Layer 2",
        "    MAT-12,     !- Layer 3",
        "    MAT-12,     !- Layer 4",
        "    MAT-12,     !- Layer 5",
        "    MAT-12;     !- Layer 6",
        "",
        "Construction,",
        "    CONST-60,   !- Name",
        "    MAT-12,     !- Outside Layer",
        "    MAT-12,     !- Layer 2",
        "    MAT-12,     !- Layer 3",
        "    MAT-12,     !- Layer 4",
        "    MAT-12,     !- Layer 5",
        "    MAT-12,     !- Layer 6",
        "    MAT-12,     !- Layer 7",
        "    MAT-12,     !- Layer 8",
        "    MAT-12;     !- Layer 9",
        "",
        "Construction,",
        "    CONST-61,   !- Name",
        "    MAT-13;     !- Outside Layer",
        "",
        "Construction,",
        "    CONST-62,   !- Name",
        "    MAT-13,     !- Outside Layer",
        "    MAT-13;     !- Layer 2",
        "",
        "Construction,",
        "    CONST-63,   !- Name",
        "    MAT-13,     !- Outside Layer",
        "    MAT-13,     !- Layer 2",
        "    MAT-13,     !- Layer 3",
        "    MAT-13;     !- Layer 4",
        "",
        "Construction,",
        "    CONST-64,   !- Name",
        "    MAT-13,     !- Outside Layer",
        "    MAT-13,     !- Layer 2",
        "    MAT-13,     !- Layer 3",
        "    MAT-13,     !- Layer 4",
        "    MAT-13,     !- Layer 5",
        "    MAT-13;     !- Layer 6",
        "",
        "Construction,",
        "    CONST-65,   !- Name",
        "    MAT-13,     !- Outside Layer",
        "    MAT-13,     !- Layer 2",
        "    MAT-13,     !- Layer 3",
        "    MAT-13,     !- Layer 4",
        "    MAT-13,     !- Layer 5",
        "    MAT-13,     !- Layer 6",
        "    MAT-13,     !- Layer 7",
        "    MAT-13,     !- Layer 8",
        "    MAT-13;     !- Layer 9",
        "",
        "Construction,",
        "    CONST-66,   !- Name",
        "    MAT-14;     !- Outside Layer",
        "",
//        "Construction,",
//        "    CONST-67,   !- Name",
//        "    MAT-14,     !- Outside Layer",
//        "    MAT-14;     !- Layer 2",
//        "",
//        "Construction,",
//        "    CONST-68,   !- Name",
//        "    MAT-14,     !- Outside Layer",
//        "    MAT-14,     !- Layer 2",
//        "    MAT-14,     !- Layer 3",
//        "    MAT-14;     !- Layer 4",
//        "",
//        "Construction,",
//        "    CONST-69,   !- Name",
//        "    MAT-14,     !- Outside Layer",
//        "    MAT-14,     !- Layer 2",
//        "    MAT-14,     !- Layer 3",
//        "    MAT-14,     !- Layer 4",
//        "    MAT-14,     !- Layer 5",
//        "    MAT-14;     !- Layer 6",
//        "",
//        "Construction,",
//        "    CONST-70,   !- Name",
//        "    MAT-14,     !- Outside Layer",
//        "    MAT-14,     !- Layer 2",
//        "    MAT-14,     !- Layer 3",
//        "    MAT-14,     !- Layer 4",
//        "    MAT-14,     !- Layer 5",
//        "    MAT-14,     !- Layer 6",
//        "    MAT-14,     !- Layer 7",
//        "    MAT-14,     !- Layer 8",
//        "    MAT-14;     !- Layer 9",
//        "",
        "Construction,",
        "    CONST-71,   !- Name",
        "    MAT-15;     !- Outside Layer",
        "",
        "Construction,",
        "    CONST-72,   !- Name",
        "    MAT-15,     !- Outside Layer",
        "    MAT-15;     !- Layer 2",
        "",
        "Construction,",
        "    CONST-73,   !- Name",
        "    MAT-15,     !- Outside Layer",
        "    MAT-15,     !- Layer 2",
        "    MAT-15,     !- Layer 3",
        "    MAT-15;     !- Layer 4",
        "",
        "Construction,",
        "    CONST-74,   !- Name",
        "    MAT-15,     !- Outside Layer",
        "    MAT-15,     !- Layer 2",
        "    MAT-15,     !- Layer 3",
        "    MAT-15,     !- Layer 4",
        "    MAT-15,     !- Layer 5",
        "    MAT-15;     !- Layer 6",
        "",
        "Construction,",
        "    CONST-75,   !- Name",
        "    MAT-15,     !- Outside Layer",
        "    MAT-15,     !- Layer 2",
        "    MAT-15,     !- Layer 3",
        "    MAT-15,     !- Layer 4",
        "    MAT-15,     !- Layer 5",
        "    MAT-15,     !- Layer 6",
        "    MAT-15,     !- Layer 7",
        "    MAT-15,     !- Layer 8",
        "    MAT-15;     !- Layer 9",
        "",
        "Construction,",
        "    CONST-76,   !- Name",
        "    MAT-16;     !- Outside Layer",
        "",
        "Construction,",
        "    CONST-77,   !- Name",
        "    MAT-16,     !- Outside Layer",
        "    MAT-16;     !- Layer 2",
        "",
        "Construction,",
        "    CONST-78,   !- Name",
        "    MAT-16,     !- Outside Layer",
        "    MAT-16,     !- Layer 2",
        "    MAT-16,     !- Layer 3",
        "    MAT-16;     !- Layer 4",
        "",
        "Construction,",
        "    CONST-79,   !- Name",
        "    MAT-16,     !- Outside Layer",
        "    MAT-16,     !- Layer 2",
        "    MAT-16,     !- Layer 3",
        "    MAT-16,     !- Layer 4",
        "    MAT-16,     !- Layer 5",
        "    MAT-16;     !- Layer 6",
        "",
        "Construction,",
        "    CONST-80,   !- Name",
        "    MAT-16,     !- Outside Layer",
        "    MAT-16,     !- Layer 2",
        "    MAT-16,     !- Layer 3",
        "    MAT-16,     !- Layer 4",
        "    MAT-16,     !- Layer 5",
        "    MAT-16,     !- Layer 6",
        "    MAT-16,     !- Layer 7",
        "    MAT-16,     !- Layer 8",
        "    MAT-16;     !- Layer 9",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    bool errorsFound(false);
    HeatBalanceManager::GetMaterialData(state.dataWindowEquivalentLayer, state.files, errorsFound);
    ASSERT_FALSE(errorsFound);

    HeatBalanceManager::GetConstructData(state.files, errorsFound);
    ASSERT_FALSE(errorsFound);

    DataGlobals::TimeStepZone = 1.0 / 6.0;

    bool doCTFErrorReport = false;

    std::vector<Real64> timesDirect;
    std::vector<Real64> timesCached;

    for (auto c : dataConstruction.Construct) {
        // setup CTF so it can be computed
        errorsFound = false;
        doCTFErrorReport = false;
        c.IsUsedCTF = true;

        // compute and time CTF calculation
        auto start = std::chrono::high_resolution_clock::now();
        c.calculateTransferFunction(errorsFound, doCTFErrorReport);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        timesDirect.push_back(duration.count());

        // write CTF to file
        std::string fName;
        fName += c.Name + ".json";
        nlohmann::json j;
        constrToJSON(c, j);
        writeJSONfile(fName, j);
    }

    for (auto c : dataConstruction.Construct) {
        nlohmann::json j;
        std::string filePath = c.Name + ".json";

        // load CTF from file, time results
        auto start = std::chrono::high_resolution_clock::now();
        readJSONfile(filePath, j);
        Construction::ConstructionProps cNew;

        auto sizeCross = j["cross"].size();
        auto sizeFlux = j["flux"].size();
        auto sizeInside = j["inside"].size();
        auto sizeOutside = j["outside"].size();

        cNew.CTFCross.dimension(sizeCross, 0.0);
        cNew.CTFFlux.dimension(sizeFlux, 0.0);
        cNew.CTFInside.dimension(sizeInside, 0.0);
        cNew.CTFOutside.dimension(sizeOutside, 0.0);

        int idx = 1;
        for (auto &val : j["cross"]) {
            cNew.CTFCross(idx) = val;
            ++idx;
        }

        idx = 1;
        for (auto &val : j["flux"]) {
            cNew.CTFFlux(idx) = val;
            ++idx;
        }

        idx = 1;
        for (auto &val : j["inside"]) {
            cNew.CTFInside(idx) = val;
            ++idx;
        }

        idx = 1;
        for (auto &val : j["outside"]) {
            cNew.CTFOutside(idx) = val;
            ++idx;
        }

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        timesCached.push_back(duration.count());
    }

    // report times
    for (std::size_t i = 0; i != dataConstruction.Construct.size(); ++i) {
        file << dataConstruction.Construct[i].Name << ",";
        file << timesDirect[i] << ",";
        file << timesCached[i] << "\n";
    }
 }

} // namespace EnergyPlus
