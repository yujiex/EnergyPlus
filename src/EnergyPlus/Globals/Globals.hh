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

#ifndef Globals_hh_INCLUDED
#define Globals_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

#include <string>

struct BaseGlobalStruct
{
    virtual void clear_state() = 0;
};

struct DataGlobal : BaseGlobalStruct
{
    // Data
    bool AnnualSimulation = false;

    // MODULE VARIABLE DECLARATIONS:
    std::string DayOfSimChr = "0";       // Counter for days (during the simulation) (character -- for reporting)

    // MODULE PARAMETER DEFINITIONS
    static constexpr int EndZoneSizingCalc = 4;

    void clear_state() override {
        AnnualSimulation = false;
        DayOfSimChr = "0";
    }
};

struct ExteriorEnergyUseGlobals : BaseGlobalStruct
{
    int NumExteriorLights = 0; // Number of Exterior Light Inputs
    int NumExteriorEqs = 0;    // Number of Exterior Equipment Inputs

    void clear_state() override {
        NumExteriorLights = 0;
        NumExteriorEqs = 0;
    }
};

struct FansGlobals : BaseGlobalStruct
{
    // constants
    static constexpr int ExhaustFanCoupledToAvailManagers = 150;
    static constexpr int ExhaustFanDecoupledFromAvailManagers = 151;

    // members
    int NumFans;
    int NumNightVentPerf;      // number of FAN:NIGHT VENT PERFORMANCE objects found in the input
    bool GetFanInputFlag;      // Flag set to make sure you get input once
    bool LocalTurnFansOn;      // If True, overrides fan schedule and cycles ZoneHVAC component fans on
    bool LocalTurnFansOff;     // If True, overrides fan schedule and LocalTurnFansOn and cycles ZoneHVAC component fans off

    FansGlobals() : NumFans(0), NumNightVentPerf(0), GetFanInputFlag(true), LocalTurnFansOn(false), LocalTurnFansOff(false)
    {
    }

    void clear_state() override
    {
        NumFans = 0;
        NumNightVentPerf = 0;
        GetFanInputFlag = true;
        LocalTurnFansOn = false;
        LocalTurnFansOff = false;
    }
};

struct PipesGlobals : BaseGlobalStruct
{
    // MODULE VARIABLE DECLARATIONS
    int NumLocalPipes = 0;
    bool GetPipeInputFlag = true;

    void clear_state() override {
        NumLocalPipes = 0;
        GetPipeInputFlag = true;
    }
};

struct AllGlobals : BaseGlobalStruct
{
    // module globals
    DataGlobal dataGlobals;
    ExteriorEnergyUseGlobals exteriorEnergyUse;
    FansGlobals fans;
    PipesGlobals pipes;

    // all clear states
    void clear_state() override
    {
        dataGlobals.clear_state();
        exteriorEnergyUse.clear_state();
        fans.clear_state();
        pipes.clear_state();
    };
};

extern AllGlobals ep_globals;

#endif
