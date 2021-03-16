// EnergyPlus, Copyright (c) 1996-2021, The Board of Trustees of the University of Illinois,
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

#ifndef DataSurfaceColors_hh_INCLUDED
#define DataSurfaceColors_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataSurfaceColors {

    int constexpr NumColors = 15;
    int constexpr ColorNo_Text = 1;
    int constexpr ColorNo_Wall = 2;
    int constexpr ColorNo_Window = 3;
    int constexpr ColorNo_GlassDoor = 4;
    int constexpr ColorNo_Door = 5;
    int constexpr ColorNo_Floor = 6;
    int constexpr ColorNo_Roof = 7;
    int constexpr ColorNo_ShdDetBldg = 8;
    int constexpr ColorNo_ShdDetFix = 9;
    int constexpr ColorNo_ShdAtt = 10;
    int constexpr ColorNo_PV = 11;
    int constexpr ColorNo_TDDDome = 12;
    int constexpr ColorNo_TDDDiffuser = 13;
    int constexpr ColorNo_DaylSensor1 = 14;
    int constexpr ColorNo_DaylSensor2 = 15;

    bool MatchAndSetColorTextString(EnergyPlusData &state, std::string const &String,          // string to be matched
                                    int SetValue,                 // value to be used for the color
                                    std::string const & ColorType // for now, must be DXF
    );

    void SetUpSchemeColors(EnergyPlusData &state, std::string const &SchemeName, Optional_string_const ColorType = _);

} // namespace DataSurfaceColors

struct SurfaceColorData : BaseGlobalStruct {
    Array1D_int const defaultcolorno = Array1D_int(DataSurfaceColors::NumColors, {3, 43, 143, 143, 45, 8, 15, 195, 9, 13, 174, 143, 143, 10, 5});
    Array1D_int DXFcolorno = Array1D_int(DataSurfaceColors::NumColors, SurfaceColorData::defaultcolorno);

    Array1D_string const colorkeys = Array1D_string(DataSurfaceColors::NumColors,
    {"Text",
                "Walls",
                "Windows",
                "GlassDoors",
                "Doors",
                "Roofs",
                "Floors",
                "DetachedBuildingShades",
                "DetachedFixedShades",
                "AttachedBuildingShades",
                "Photovoltaics",
                "TubularDaylightDomes",
                "TubularDaylightDiffusers",
                "DaylightReferencePoint1",
                "DaylightReferencePoint2"});

    Array1D_int const colorkeyptr = Array1D_int(DataSurfaceColors::NumColors,
    {DataSurfaceColors::ColorNo_Text,
                DataSurfaceColors::ColorNo_Wall,
                DataSurfaceColors::ColorNo_Window,
                DataSurfaceColors::ColorNo_GlassDoor,
                DataSurfaceColors::ColorNo_Door,
                DataSurfaceColors::ColorNo_Floor,
                DataSurfaceColors::ColorNo_Roof,
                DataSurfaceColors::ColorNo_ShdDetBldg,
                DataSurfaceColors::ColorNo_ShdDetFix,
                DataSurfaceColors::ColorNo_ShdAtt,
                DataSurfaceColors::ColorNo_PV,
                DataSurfaceColors::ColorNo_TDDDome,
                DataSurfaceColors::ColorNo_TDDDiffuser,
                DataSurfaceColors::ColorNo_DaylSensor1,
                DataSurfaceColors::ColorNo_DaylSensor2});

    void clear_state() override
    {
        this->DXFcolorno = Array1D_int(DataSurfaceColors::NumColors, SurfaceColorData::defaultcolorno);
    }
};

} // namespace EnergyPlus

#endif
