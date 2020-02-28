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

#include <EnergyPlus/Coils/CoilCoolingDXCurveFitPerformance.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

using namespace EnergyPlus;
using namespace DataIPShortCuts;

void CoilCoolingDXCurveFitPerformance::instantiateFromInputSpec(const CoilCoolingDXCurveFitPerformanceInputSpecification& input_data)
{
    static const std::string routineName("CoilCoolingDXCurveFitOperatingMode::instantiateFromInputSpec: ");
    bool errorsFound(false);
    this->original_input_specs = input_data;
    this->name = input_data.name;
    this->minOutdoorDrybulb = input_data.minimum_outdoor_dry_bulb_temperature_for_compressor_operation;
    this->maxOutdoorDrybulbForBasin = input_data.maximum_outdoor_dry_bulb_temperature_for_crankcase_heater_operation;
    this->crankcaseHeaterCap = input_data.crankcase_heater_capacity;
    this->normalMode = CoilCoolingDXCurveFitOperatingMode(input_data.base_operating_mode_name);
    this->normalMode.oneTimeInit(); // oneTimeInit does not need to be delayed in this use case
    if (UtilityRoutines::SameString(input_data.capacity_control, "CONTINUOUS")) {
        this->capControlMethod = CapControlMethod::CONTINUOUS;
    } else if (UtilityRoutines::SameString(input_data.capacity_control, "DISCRETE")) {
        this->capControlMethod = CapControlMethod::DISCRETE;
    } else {
        ShowSevereError(routineName + this->object_name + "=\"" + this->name + "\", invalid");
        ShowContinueError("...Capacity Control Method=\"" + input_data.capacity_control + "\":");
        ShowContinueError("...must be Discrete or Continuous.");
        errorsFound = true;
    }
    this->evapCondBasinHeatCap = input_data.basin_heater_capacity;
    this->evapCondBasinHeatSetpoint = input_data.basin_heater_setpoint_temperature;
    if (input_data.basin_heater_operating_schedule_name.empty()) {
        this->evapCondBasinHeatSchedulIndex = DataGlobals::ScheduleAlwaysOn;
    } else {
        this->evapCondBasinHeatSchedulIndex = ScheduleManager::GetScheduleIndex(input_data.basin_heater_operating_schedule_name);
    }
    if (this->evapCondBasinHeatSchedulIndex == 0) {
        ShowSevereError(routineName + this->object_name + "=\"" + this->name + "\", invalid");
        ShowContinueError("...Evaporative Condenser Basin Heater Operating Schedule Name=\"" + input_data.basin_heater_operating_schedule_name +
                          "\".");
        errorsFound = true;
    }

    if (!input_data.alternate_operating_mode_name.empty()) {
        this->hasAlternateMode = true;
        this->alternateMode = CoilCoolingDXCurveFitOperatingMode(input_data.alternate_operating_mode_name);
        this->alternateMode.oneTimeInit(); // oneTimeInit does not need to be delayed in this use case
    }

    this->compressorFuelType = input_data.compressor_fuel_type;

    if (errorsFound) {
        ShowFatalError(routineName + "Errors found in getting " + this->object_name + " input. Preceding condition(s) causes termination.");
    }
}

CoilCoolingDXCurveFitPerformance::CoilCoolingDXCurveFitPerformance(const std::string& name_to_find)
{
    int numPerformances = inputProcessor->getNumObjectsFound(CoilCoolingDXCurveFitPerformance::object_name);
    if (numPerformances <= 0) {
        // error
    }
    bool found_it = false;
    for (int perfNum = 1; perfNum <= numPerformances; ++perfNum) {
        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int IOStatus;
        inputProcessor->getObjectItem(
            CoilCoolingDXCurveFitPerformance::object_name, perfNum, cAlphaArgs, NumAlphas, rNumericArgs, NumNumbers, IOStatus, _, lAlphaFieldBlanks);
        if (!UtilityRoutines::SameString(name_to_find, cAlphaArgs(1))) {
            continue;
        }
        found_it = true;

        CoilCoolingDXCurveFitPerformanceInputSpecification input_specs;

        input_specs.name = cAlphaArgs(1);
        input_specs.crankcase_heater_capacity = rNumericArgs(1);
        input_specs.minimum_outdoor_dry_bulb_temperature_for_compressor_operation = rNumericArgs(2);
        input_specs.maximum_outdoor_dry_bulb_temperature_for_crankcase_heater_operation = rNumericArgs(3);
        if (lNumericFieldBlanks(4)) {
            input_specs.unit_internal_static_air_pressure = 0.0;
        } else {
            input_specs.unit_internal_static_air_pressure = rNumericArgs(4);
        }
        input_specs.capacity_control = cAlphaArgs(2);
        input_specs.basin_heater_capacity = rNumericArgs(5);
        input_specs.basin_heater_setpoint_temperature = rNumericArgs(6);
        input_specs.basin_heater_operating_schedule_name = cAlphaArgs(3);
        input_specs.compressor_fuel_type = DataGlobalConstants::AssignResourceTypeNum(cAlphaArgs(4));
        input_specs.base_operating_mode_name = cAlphaArgs(5);
        if (!lAlphaFieldBlanks(6)) {
            input_specs.alternate_operating_mode_name = cAlphaArgs(6);
        }

        this->instantiateFromInputSpec(input_specs);
        break;
    }

    if (!found_it) {
        ShowFatalError("Could not find Coil:Cooling:DX:Performance object with name: " + name_to_find);
    }
}

void CoilCoolingDXCurveFitPerformance::simulate(const DataLoopNode::NodeData &inletNode,
                                                DataLoopNode::NodeData &outletNode,
                                                bool useAlternateMode,
                                                Real64 &PLR,
                                                int &speedNum,
                                                Real64 &speedRatio,
                                                int const fanOpMode,
                                                DataLoopNode::NodeData &condInletNode,
                                                DataLoopNode::NodeData &condOutletNode)
{
    if (useAlternateMode) {
        this->calculate(this->alternateMode, inletNode, outletNode, PLR, speedNum, speedRatio, fanOpMode, condInletNode, condOutletNode);
    } else {
        this->calculate(this->normalMode, inletNode, outletNode, PLR, speedNum, speedRatio, fanOpMode, condInletNode, condOutletNode);
    }
}

void CoilCoolingDXCurveFitPerformance::size()
{
    if (!DataGlobals::SysSizingCalc && this->mySizeFlag) {
        this->normalMode.parentName = this->parentName;
        this->normalMode.size();
        if (this->hasAlternateMode) {
            this->alternateMode.parentName = this->parentName;
            this->alternateMode.size();
        }
        this->mySizeFlag = false;
    }
}

void CoilCoolingDXCurveFitPerformance::calculate(CoilCoolingDXCurveFitOperatingMode &currentMode,
                                                 const DataLoopNode::NodeData &inletNode,
                                                 DataLoopNode::NodeData &outletNode,
                                                 Real64 &PLR,
                                                 int &speedNum,
                                                 Real64 &speedRatio,
                                                 int const fanOpMode,
                                                 DataLoopNode::NodeData &condInletNode,
                                                 DataLoopNode::NodeData &condOutletNode)
{

    // calculate the performance at this mode/speed
    currentMode.CalcOperatingMode(inletNode, outletNode, PLR, speedNum, speedRatio, fanOpMode, condInletNode, condOutletNode);

    // scaling term to get rate into consumptions
    Real64 reportingConstant = DataHVACGlobals::TimeStepSys * DataGlobals::SecInHour;

    // calculate crankcase heater operation
    if (DataEnvironment::OutDryBulbTemp < this->maxOutdoorDrybulbForBasin) {
        this->crankcaseHeaterPower = this->crankcaseHeaterCap;
    } else {
        this->crankcaseHeaterPower = 0.0;
    }
    this->crankcaseHeaterPower = this->crankcaseHeaterPower * (1.0 - this->RTF);
    this->crankcaseHeaterElectricityConsumption = this->crankcaseHeaterPower * reportingConstant;

    // basin heater
    if (this->evapCondBasinHeatSchedulIndex > 0) {
        Real64 currentBasinHeaterAvail = ScheduleManager::GetCurrentScheduleValue(this->evapCondBasinHeatSchedulIndex);
        if (this->evapCondBasinHeatCap > 0.0 && currentBasinHeaterAvail > 0.0) {
            this->basinHeaterPower = max(0.0, this->evapCondBasinHeatCap * (this->evapCondBasinHeatSetpoint - DataEnvironment::OutDryBulbTemp));
        }
    } else {
        // If schedule does not exist, basin heater operates anytime outdoor dry-bulb temp is below setpoint
        if (this->evapCondBasinHeatCap > 0.0) {
            this->basinHeaterPower = max(0.0, this->evapCondBasinHeatCap * (this->evapCondBasinHeatSetpoint - DataEnvironment::OutDryBulbTemp));
        }
    }
    this->basinHeaterPower *= (1.0 - this->RTF);

    // update other reporting terms
    this->powerUse = currentMode.OpModePower;
    this->RTF = currentMode.OpModeRTF;
    this->electricityConsumption = this->powerUse * reportingConstant;
    this->wasteHeatRate = currentMode.OpModeWasteHeat;

    if (this->compressorFuelType != DataGlobalConstants::iRT_Electricity) {
        this->compressorFuelRate = this->powerUse;
        this->compressorFuelConsumption = this->electricityConsumption;

        // check this after adding parasitic loads
        this->powerUse = 0.0;
        this->electricityConsumption = 0.0;
    }

}

void CoilCoolingDXCurveFitPerformance::calcStandardRatings210240() {

    // for now this will provide standard ratings for the coil at the normal mode at speed N
    // future iterations will extend the inputs to give the user the flexibility to select different standards to
    // apply and such

    int const NumOfReducedCap(4); // Number of reduced capacity test conditions (100%,75%,50%,and 25%)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 TotCapFlowModFac(0.0);          // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
    Real64 EIRFlowModFac(0.0);             // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
    Real64 TotCapTempModFac(0.0);          // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
    Real64 EIRTempModFac(0.0);             // EIR modifier (function of entering wetbulb, outside drybulb) [-]
    Real64 TotCoolingCapAHRI(0.0);         // Total Cooling Coil capacity (gross) at AHRI test conditions [W]
    Real64 NetCoolingCapAHRI(0.0);         // Net Cooling Coil capacity at AHRI TestB conditions, accounting for fan heat [W]
    Real64 TotalElecPower(0.0);            // Net power consumption (Cond Fan+Compressor+Indoor Fan) at AHRI test conditions [W]
    Real64 TotalElecPowerRated(0.0);       // Net power consumption (Cond Fan+Compressor+Indoor Fan) at Rated test conditions [W]
    Real64 EIR(0.0);                       // Energy Efficiency Ratio at AHRI test conditions for SEER [-]
    Real64 PartLoadFactor(0.0);            // Part load factor, accounts for thermal lag at compressor startup [-]
    Real64 EERReduced(0.0);                // EER at reduced capacity test conditions (100%, 75%, 50%, and 25%)
    Real64 ElecPowerReducedCap(0.0);       // Net power consumption (Cond Fan+Compressor) at reduced test condition [W]
    Real64 NetCoolingCapReduced(0.0);      // Net Cooling Coil capacity at reduced conditions, accounting for supply fan heat [W]
    Real64 LoadFactor(0.0);                // Fractional "on" time for last stage at the desired reduced capacity, (dimensionless)
    Real64 DegradationCoeff(0.0);          // Degradation coeficient, (dimenssionless)
    Real64 OutdoorUnitInletAirDryBulbTempReduced; // Outdoor unit entering air dry-bulb temperature at reduced capacity [C]
    int RedCapNum;                                // Integer counter for reduced capacity

    // *** SOME CONSTANTS FROM THE STANDARD
    // The AHRI standard specifies a nominal/default fan electric power consumption per rated air
    // volume flow rate to account for indoor fan electric power consumption
    // when the standard tests are conducted on units that do not have an
    // indoor air circulating fan. Used if user doesn't enter a specific value.
    Real64 const DefaultFanPowerPerEvapAirFlowRate(773.3); // 365 W/1000 scfm or 773.3 W/(m3/s).
    // AHRI Standard 210/240-2008 Performance Test Conditions for Unitary Air-to-Air Air-Conditioning and Heat Pump Equipment
    Real64 const CoolingCoilInletAirWetBulbTempRated(19.44); // 19.44C (67F)  Tests A and B
    Real64 const OutdoorUnitInletAirDryBulbTemp(27.78);      // 27.78C (82F)  Test B (for SEER)
    Real64 const OutdoorUnitInletAirDryBulbTempRated(35.0);  // 35.00C (95F)  Test A (rated capacity)
    Real64 const AirMassFlowRatioRated(1.0);                 // AHRI test is at the design flow rate so AirMassFlowRatio is 1.0
    Real64 const PLRforSEER(0.5);                                 // Part-load ratio for SEER calculation (single speed DX cooling coils)
    Array1D<Real64> const ReducedPLR(4, {1.0, 0.75, 0.50, 0.25}); // Reduced Capacity part-load conditions
    Array1D<Real64> const IEERWeightingFactor(4, {0.020, 0.617, 0.238, 0.125}); // EER Weighting factors (IEER)
    Real64 const OADBTempLowReducedCapacityTest(18.3);                          // Outdoor air dry-bulb temp in degrees C (65F)

    // some conveniences
    auto & mode = this->normalMode;
    auto & speed = mode.speeds.back();

    Real64 FanPowerPerEvapAirFlowRate = DefaultFanPowerPerEvapAirFlowRate;
    if (speed.rated_evap_fan_power_per_volume_flow_rate > 0.0) {
        FanPowerPerEvapAirFlowRate = speed.rated_evap_fan_power_per_volume_flow_rate;
    }

    if (mode.ratedGrossTotalCap > 0.0) {
        // SEER calculations:
        TotCapFlowModFac = CurveManager::CurveValue(speed.indexCapFFF, AirMassFlowRatioRated);
        TotCapTempModFac = CurveManager::CurveValue(speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTemp);
        TotCoolingCapAHRI = mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac;
        EIRTempModFac = CurveManager::CurveValue(speed.indexEIRFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTemp);
        EIRFlowModFac = CurveManager::CurveValue(speed.indexEIRFFF, AirMassFlowRatioRated);
        if (speed.ratedCOP > 0.0) { // RatedCOP <= 0.0 is trapped in GetInput, but keep this as "safety"
            EIR = EIRTempModFac * EIRFlowModFac / speed.ratedCOP;
        } else {
            EIR = 0.0;
        }

        // Calculate net cooling capacity
        NetCoolingCapAHRI = TotCoolingCapAHRI - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        TotalElecPower = EIR * TotCoolingCapAHRI + FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        // Calculate SEER value from the Energy Efficiency Ratio (EER) at the AHRI test conditions and the part load factor.
        // First evaluate the Part Load Factor curve at PLR = 0.5 (AHRI Standard 210/240)
        PartLoadFactor = CurveManager::CurveValue(speed.indexPLRFPLF, PLRforSEER);
        if (TotalElecPower > 0.0) {
            this->standardRatingSEER = (NetCoolingCapAHRI / TotalElecPower) * PartLoadFactor;
        } else {
            this->standardRatingSEER = 0.0;
        }

        // EER calculations:
        // Calculate the net cooling capacity at the rated conditions (19.44C WB and 35.0C DB )
        TotCapTempModFac = CurveManager::CurveValue(speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        this->standardRatingCoolingCapacity = mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        // Calculate Energy Efficiency Ratio (EER) at (19.44C WB and 35.0C DB ), ANSI/AHRI Std. 340/360
        EIRTempModFac = CurveManager::CurveValue(speed.indexEIRFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        if (speed.ratedCOP > 0.0) {
            // RatedCOP <= 0.0 is trapped in GetInput, but keep this as "safety"
            EIR = EIRTempModFac * EIRFlowModFac / speed.ratedCOP;
        } else {
            EIR = 0.0;
        }
        TotalElecPowerRated = EIR * (mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac) + FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        if (TotalElecPowerRated > 0.0) {
            this->standardRatingEER = this->standardRatingCoolingCapacity / TotalElecPowerRated;
        } else {
            this->standardRatingEER = 0.0;
        }

        // IEER calculations:
        this->standardRatingIEER = 0.0;
        // Calculate the net cooling capacity at the rated conditions (19.44C WB and 35.0C DB )
        TotCapTempModFac = CurveManager::CurveValue(speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        this->standardRatingCoolingCapacity = mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        for (RedCapNum = 1; RedCapNum <= NumOfReducedCap; ++RedCapNum) {
            // get the outdoor air dry bulb temperature for the reduced capacity test conditions
            if (ReducedPLR(RedCapNum) > 0.444) {
                OutdoorUnitInletAirDryBulbTempReduced = 5.0 + 30.0 * ReducedPLR(RedCapNum);
            } else {
                OutdoorUnitInletAirDryBulbTempReduced = OADBTempLowReducedCapacityTest;
            }
            TotCapTempModFac = CurveManager::CurveValue(speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);
            NetCoolingCapReduced = mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
            EIRTempModFac = CurveManager::CurveValue(speed.indexEIRFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);
            if (speed.ratedCOP > 0.0) {
                EIR = EIRTempModFac * EIRFlowModFac / speed.ratedCOP;
            } else {
                EIR = 0.0;
            }
            if (NetCoolingCapReduced > 0.0) {
                LoadFactor = ReducedPLR(RedCapNum) * this->standardRatingCoolingCapacity / NetCoolingCapReduced;
            } else {
                LoadFactor = 1.0;
            }
            DegradationCoeff = 1.130 - 0.130 * LoadFactor;
            ElecPowerReducedCap = DegradationCoeff * EIR * (mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac);
            EERReduced =
                    (LoadFactor * NetCoolingCapReduced) / (LoadFactor * ElecPowerReducedCap + FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate);
            this->standardRatingIEER += IEERWeightingFactor(RedCapNum) * EERReduced;
        }

    } else {
        ShowSevereError("Standard Ratings: Coil:Cooling:DX " + this->name +  // TODO: Use dynamic COIL TYPE and COIL INSTANCE name later
                        " has zero rated total cooling capacity. Standard ratings cannot be calculated.");
    }
}
