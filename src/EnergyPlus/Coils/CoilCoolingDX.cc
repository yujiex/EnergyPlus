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


#include <ObjexxFCL/Array1D.hh> // needs to be in BranchNodeConnections.hh

#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputFiles.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/HVACFan.hh>

using namespace EnergyPlus;
using namespace DataIPShortCuts;

namespace EnergyPlus {
    std::vector<CoilCoolingDX> coilCoolingDXs;
    bool coilCoolingDXGetInputFlag = true;
    std::string const coilCoolingDXObjectName = "Coil:Cooling:DX";
    bool stillNeedToReportStandardRatings = true;
}

int CoilCoolingDX::factory(std::string const & coilName) {
    if (coilCoolingDXGetInputFlag) {
        CoilCoolingDX::getInput();
        coilCoolingDXGetInputFlag = false;
    }
    int handle = -1;
    for (auto const & thisCoil : coilCoolingDXs) {
        handle++;
        if (EnergyPlus::UtilityRoutines::MakeUPPERCase(coilName) == EnergyPlus::UtilityRoutines::MakeUPPERCase(thisCoil.name)) {
            return handle;
        }
    }
    ShowSevereError("Coil:Cooling:DX Coil not found=" + coilName);
    return -1;
}

void CoilCoolingDX::clear_state() {
    coilCoolingDXs.clear();
    coilCoolingDXGetInputFlag = true;
    stillNeedToReportStandardRatings = true;
}

void CoilCoolingDX::getInput() {
    int numCoolingCoilDXs = inputProcessor->getNumObjectsFound(coilCoolingDXObjectName);
    if (numCoolingCoilDXs <= 0) {
        ShowFatalError(R"(No "Coil:Cooling:DX" objects in input file)");
    }
    for (int coilNum = 1; coilNum <= numCoolingCoilDXs; ++coilNum) {
        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int IOStatus;
        inputProcessor->getObjectItem(coilCoolingDXObjectName, coilNum, cAlphaArgs, NumAlphas, rNumericArgs, NumNumbers, IOStatus);
        CoilCoolingDXInputSpecification input_specs;
        input_specs.name = cAlphaArgs(1);
        input_specs.evaporator_inlet_node_name = cAlphaArgs(2);
        input_specs.evaporator_outlet_node_name = cAlphaArgs(3);
        input_specs.availability_schedule_name = cAlphaArgs(4);
        input_specs.condenser_zone_name = cAlphaArgs(5);
        input_specs.condenser_inlet_node_name = cAlphaArgs(6);
        input_specs.condenser_outlet_node_name = cAlphaArgs(7);
        input_specs.performance_object_name = cAlphaArgs(8);
        input_specs.condensate_collection_water_storage_tank_name = cAlphaArgs(9);
        input_specs.evaporative_condenser_supply_water_storage_tank_name = cAlphaArgs(10);
        CoilCoolingDX thisCoil;
        thisCoil.instantiateFromInputSpec(input_specs);
        coilCoolingDXs.push_back(thisCoil);
    }
}

void CoilCoolingDX::instantiateFromInputSpec(const CoilCoolingDXInputSpecification& input_data)
{
    static const std::string routineName("CoilCoolingDX::instantiateFromInputSpec: ");
    this->original_input_specs = input_data;
    bool errorsFound = false;
    this->name = input_data.name;
    this->performance = CoilCoolingDXCurveFitPerformance(input_data.performance_object_name);

    // other construction below
    this->evapInletNodeIndex = NodeInputManager::GetOnlySingleNode(input_data.evaporator_inlet_node_name,
                                                                   errorsFound,
                                                                   coilCoolingDXObjectName,
                                                                   input_data.name,
                                                                   DataLoopNode::NodeType_Air,
                                                                   DataLoopNode::NodeConnectionType_Inlet,
                                                                   1,
                                                                   DataLoopNode::ObjectIsNotParent);
    this->evapOutletNodeIndex = NodeInputManager::GetOnlySingleNode(input_data.evaporator_outlet_node_name,
                                                                    errorsFound,
                                                                    coilCoolingDXObjectName,
                                                                    input_data.name,
                                                                    DataLoopNode::NodeType_Air,
                                                                    DataLoopNode::NodeConnectionType_Outlet,
                                                                    1,
                                                                    DataLoopNode::ObjectIsNotParent);

    this->condInletNodeIndex = NodeInputManager::GetOnlySingleNode(input_data.condenser_inlet_node_name,
                                                                   errorsFound,
                                                                   coilCoolingDXObjectName,
                                                                   input_data.name,
                                                                   DataLoopNode::NodeType_Air,
                                                                   DataLoopNode::NodeConnectionType_Inlet,
                                                                   2,
                                                                   DataLoopNode::ObjectIsNotParent);

    // Ultimately, this restriction should go away - condenser inlet node could be from anywhere
    if (!OutAirNodeManager::CheckOutAirNodeNumber(this->condInletNodeIndex)) {
        ShowWarningError(routineName + coilCoolingDXObjectName + "=\"" + this->name + "\", may be invalid");
        ShowContinueError("Condenser Inlet Node Name=\"" + input_data.condenser_inlet_node_name +
                          "\", node does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
        ShowContinueError("This node needs to be included in an air system or the coil model will not be valid, and the simulation continues");
    }

    this->condOutletNodeIndex = NodeInputManager::GetOnlySingleNode(input_data.condenser_outlet_node_name,
                                                                        errorsFound,
                                                                        coilCoolingDXObjectName,
                                                                        input_data.name,
                                                                        DataLoopNode::NodeType_Air,
                                                                        DataLoopNode::NodeConnectionType_Outlet,
                                                                        2,
                                                                        DataLoopNode::ObjectIsNotParent);

    if (!input_data.condensate_collection_water_storage_tank_name.empty()) {
        WaterManager::SetupTankSupplyComponent(this->name,
                                               coilCoolingDXObjectName,
                                 input_data.condensate_collection_water_storage_tank_name,
                                 errorsFound,
                                 this->condensateTankIndex,
                                 this->condensateTankSupplyARRID);
    }

    if (!input_data.evaporative_condenser_supply_water_storage_tank_name.empty()) {
        WaterManager::SetupTankDemandComponent(this->name,
                                               coilCoolingDXObjectName,
                                 input_data.evaporative_condenser_supply_water_storage_tank_name,
                                 errorsFound,
                                 this->evaporativeCondSupplyTankIndex,
                                 this->evaporativeCondSupplyTankARRID);
    }

    if (input_data.availability_schedule_name.empty()) {
      this->availScheduleIndex = DataGlobals::ScheduleAlwaysOn;
    } else {
        this->availScheduleIndex = ScheduleManager::GetScheduleIndex(input_data.availability_schedule_name);
    }

    if (this->availScheduleIndex == 0) {
        ShowSevereError(routineName + coilCoolingDXObjectName + "=\"" + this->name + "\", invalid");
        ShowContinueError("...Availability Schedule Name=\"" + input_data.availability_schedule_name + "\".");
        errorsFound = true;
    }

    if (!input_data.condenser_zone_name.empty()) {
        this->isSecondaryDXCoilInZone = true;
        // Setup zone data here
    }

    BranchNodeConnections::TestCompSet(
            coilCoolingDXObjectName, this->name, input_data.evaporator_inlet_node_name, input_data.evaporator_outlet_node_name, "Air Nodes");

    if (errorsFound) {
        ShowFatalError(routineName + "Errors found in getting " + coilCoolingDXObjectName + " input. Preceding condition(s) causes termination.");
    }
}

void CoilCoolingDX::oneTimeInit() {

    // setup output variables, needs to be done after object is instantiated and emplaced
    SetupOutputVariable("Cooling Coil Total Cooling Rate",
                        OutputProcessor::Unit::W,
                        this->totalCoolingEnergyRate,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Total Cooling Energy",
                        OutputProcessor::Unit::J,
                        this->totalCoolingEnergy,
                        "System",
                        "Sum",
                        this->name,
                        _,
                        "ENERGYTRANSFER",
                        "COOLINGCOILS",
                        _,
                        "System");
    SetupOutputVariable("Cooling Coil Sensible Cooling Rate",
                        OutputProcessor::Unit::W,
                        this->sensCoolingEnergyRate,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Sensible Cooling Energy",
                        OutputProcessor::Unit::J,
                        this->sensCoolingEnergy,
                        "System",
                        "Sum",
                        this->name);
    SetupOutputVariable("Cooling Coil Latent Cooling Rate",
                        OutputProcessor::Unit::W,
                        this->latCoolingEnergyRate,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Latent Cooling Energy",
                        OutputProcessor::Unit::J,
                        this->latCoolingEnergy,
                        "System",
                        "Sum",
                        this->name);
    SetupOutputVariable("Cooling Coil Electric Power",
                        OutputProcessor::Unit::W,
                        this->performance.powerUse,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Electric Energy",
                        OutputProcessor::Unit::J,
                        this->performance.electricityConsumption,
                        "System",
                        "Sum",
                        this->name,
                        _,
                        "Electric",
                        "COOLING",
                        _,
                        "System");

    if (this->performance.compressorFuelType != DataGlobalConstants::iRT_Electricity) {
        SetupOutputVariable("Cooling Coil " + DataGlobalConstants::GetResourceTypeChar(this->performance.compressorFuelType) + " Rate",
                            OutputProcessor::Unit::W,
                            this->performance.compressorFuelRate,
                            "System",
                            "Average",
                            this->name);
        SetupOutputVariable("Cooling Coil " + DataGlobalConstants::GetResourceTypeChar(this->performance.compressorFuelType) + " Energy",
                            OutputProcessor::Unit::J,
                            this->performance.compressorFuelConsumption,
                            "System",
                            "Sum",
                            this->name,
                            _,
                            DataGlobalConstants::GetResourceTypeChar(this->performance.original_input_specs.compressor_fuel_type),
                            "COOLING",
                            _,
                            "System");
    }

    SetupOutputVariable("Cooling Coil Runtime Fraction",
                        OutputProcessor::Unit::None,
                        this->coolingCoilRuntimeFraction,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Crankcase Heater Electric Power",
                        OutputProcessor::Unit::W,
                        this->performance.crankcaseHeaterPower,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Crankcase Heater Electric Energy",
                        OutputProcessor::Unit::J,
                        this->performance.crankcaseHeaterElectricityConsumption,
                        "System",
                        "Sum",
                        this->name,
                        _,
                        "Electric",
                        "DHW",
                        _,
                        "Plant");
   // Ported from variable speed coil
    SetupOutputVariable("Cooling Coil Air Mass Flow Rate",
                        OutputProcessor::Unit::kg_s,
                        this->airMassFlowRate,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Air Inlet Temperature",
                        OutputProcessor::Unit::C,
                        this->inletAirDryBulbTemp,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Air Inlet Humidity Ratio",
                        OutputProcessor::Unit::kgWater_kgDryAir,
                        this->inletAirHumRat,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Air Outlet Temperature",
                        OutputProcessor::Unit::C,
                        this->outletAirDryBulbTemp,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Air Outlet Humidity Ratio",
                        OutputProcessor::Unit::kgWater_kgDryAir,
                        this->outletAirHumRat,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Part Load Ratio",
                        OutputProcessor::Unit::None,
                        this->partLoadRatioReport,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Upper Speed Level",
                        OutputProcessor::Unit::None,
                        this->speedNumReport,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Neighboring Speed Levels Ratio",
                        OutputProcessor::Unit::None,
                        this->speedRatioReport,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Condenser Inlet Temperature",
                        OutputProcessor::Unit::C,
                        this->condenserInletTemperature,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Dehumidification Mode",
                        OutputProcessor::Unit::None,
                        this->dehumidificationMode,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Waste Heat Power",
                        OutputProcessor::Unit::W,
                        this->wasteHeatEnergyRate,
                        "System",
                        "Average",
                        this->name);
    SetupOutputVariable("Cooling Coil Waste Heat Energy",
                        OutputProcessor::Unit::J,
                        this->wasteHeatEnergy,
                        "System",
                        "Sum",
                        this->name);

    if (this->performance.evapCondBasinHeatCap > 0) {
        SetupOutputVariable("Cooling Coil Basin Heater Electric Power",
                            OutputProcessor::Unit::W,
                            this->performance.basinHeaterPower,
                            "System",
                            "Average",
                            this->name);
        SetupOutputVariable("Cooling Coil Basin Heater Electric Energy",
                            OutputProcessor::Unit::J,
                            this->performance.basinHeaterElectricityConsumption,
                            "System",
                            "Sum",
                            this->name,
                            _,
                            "Electric",
                            "COOLING",
                            _,
                            "System");
    }
    if (this->condensateTankIndex > 0) {
        SetupOutputVariable("Cooling Coil Condensate Volume Flow Rate",
                            OutputProcessor::Unit::m3_s,
                            this->condensateVolumeFlow,
                            "System",
                            "Average",
                            this->name);
        SetupOutputVariable("Cooling Coil Condensate Volume",
                            OutputProcessor::Unit::m3,
                            this->condensateVolumeConsumption,
                            "System",
                            "Sum",
                            this->name,
                            _,
                            "OnSiteWater",
                            "Condensate",
                            _,
                            "System");
    }
    if (this->evaporativeCondSupplyTankIndex > 0) {
        SetupOutputVariable("Cooling Coil Evaporative Condenser Pump Electric Power",
                            OutputProcessor::Unit::W,
                            this->evapCondPumpElecPower,
                            "System",
                            "Average",
                            this->name);
        SetupOutputVariable("Cooling Coil Evaporative Condenser Pump Electric Energy",
                            OutputProcessor::Unit::J,
                            this->evapCondPumpElecConsumption,
                            "System",
                            "Sum",
                            this->name,
                            _,
                            "Electric",
                            "COOLING",
                            _,
                            "System");
        SetupOutputVariable("Cooling Coil Evaporative Condenser Water Volume Flow Rate",
                            OutputProcessor::Unit::m3_s,
                            this->evapCondPumpElecPower,
                            "System",
                            "Average",
                            this->name);
        SetupOutputVariable("Cooling Coil Evaporative Condenser Water Volume",
                            OutputProcessor::Unit::m3,
                            this->evapCondPumpElecConsumption,
                            "System",
                            "Sum",
                            this->name,
                            _,
                            "Water",
                            "Cooling",
                            _,
                            "System");
        SetupOutputVariable("Cooling Coil Evaporative Condenser Mains Supply Water Volume",
                            OutputProcessor::Unit::m3,
                            this->evaporativeCondSupplyTankConsump,
                            "System",
                            "Sum",
                            this->name,
                            _,
                            "MainsWater",
                            "Cooling",
                            _,
                            "System");
    }

    if (this->isSecondaryDXCoilInZone) {
        SetupOutputVariable("Secondary Coil Heat Rejection Rate",
                            OutputProcessor::Unit::W,
                            this->secCoilSensHeatRejEnergyRate,
                            "System",
                            "Average",
                            this->name);

        SetupOutputVariable("Secondary Coil Heat Rejection Energy",
                            OutputProcessor::Unit::J,
                            this->secCoilSensHeatRejEnergy,
                            "System",
                            "Sum",
                            this->name);
    }

    // HPWH and/or VRF output variables

    //    SetupOutputVariable("Cooling Coil VRF Super Heating Degrees",
    //                        OutputProcessor::Unit::C,
    //                        this->secCoilSensHeatRejEnergy,
    //                        "System",
    //                        "Sum",
    //                        this->name);

    //    SetupOutputVariable("Cooling Coil VRF Evaporating Temperature",
    //                        OutputProcessor::Unit::C,
    //                        this->secCoilSensHeatRejEnergy,
    //                        "System",
    //                        "Average",
    //                        this->name);

    //    SetupOutputVariable("Cooling Coil Water Heating Electric Power",
    //                        OutputProcessor::Unit::W,
    //                        this->secCoilSensHeatRejEnergy,
    //                        "System",
    //                        "Sum",
    //                        this->name);

    //    SetupOutputVariable("Cooling Coil Water Heating Electric Energy",
    //                        OutputProcessor::Unit::J,
    //                        this->secCoilSensHeatRejEnergy,
    //                        "System",
    //                        "Average",
    //                        this->name);

    //    SetupOutputVariable("Cooling Coil Total Water Heating Rate",
    //                        OutputProcessor::Unit::W,
    //                        this->secCoilSensHeatRejEnergy,
    //                        "System",
    //                        "Sum",
    //                        this->name);

    //    SetupOutputVariable("Cooling Coil Total Water Heating Energy",
    //                        OutputProcessor::Unit::J,
    //                        this->secCoilSensHeatRejEnergy,
    //                        "System",
    //                        "Average",
    //                        this->name);

}

int CoilCoolingDX::getNumModes() {
    int numModes = 1;
    if (this->performance.hasAlternateMode) {
        numModes++;
    }
    return numModes;
}

int CoilCoolingDX::getOpModeCapFTIndex(bool const isNormalOpMode)
{
    if (isNormalOpMode) {
        return this->normModeNomSpeed().indexCapFT;
    } else {
        return this->altModeNomSpeed().indexCapFT;
    }
}

void CoilCoolingDX::setData(int fanIndex, int fanType, std::string const &fanName, int _airLoopNum) {
    this->supplyFanIndex = fanIndex;
    this->supplyFanName = fanName;
    this->supplyFanType = fanType;
    this->airLoopNum = _airLoopNum;
}

void CoilCoolingDX::getFixedData(int &_evapInletNodeIndex,
                            int &_evapOutletNodeIndex,
                            int &_condInletNodeIndex,
                            int &_normalModeNumSpeeds,
                            CoilCoolingDXCurveFitPerformance::CapControlMethod &_capacityControlMethod,
                            Real64 &_minOutdoorDryBulb)
{
    _evapInletNodeIndex = this->evapInletNodeIndex;
    _evapOutletNodeIndex = this->evapOutletNodeIndex;
    _condInletNodeIndex = this->condInletNodeIndex;
    _normalModeNumSpeeds = (int)this->performance.normalMode.speeds.size();
    _capacityControlMethod = this->performance.capControlMethod;
    _minOutdoorDryBulb = this->performance.minOutdoorDrybulb;
}

void CoilCoolingDX::getDataAfterSizing(Real64 &_normalModeRatedEvapAirFlowRate,
                                 Real64 &_normalModeRatedCapacity,
                                 std::vector<Real64> &_normalModeFlowRates,
                                 std::vector<Real64> &_normalModeRatedCapacities)
{
    _normalModeRatedEvapAirFlowRate = this->performance.normalMode.ratedEvapAirFlowRate;
    _normalModeFlowRates.clear();
    _normalModeRatedCapacities.clear();
    for (auto const &thisSpeed : this->performance.normalMode.speeds) {
        _normalModeFlowRates.push_back(thisSpeed.evap_air_flow_rate);
        _normalModeRatedCapacities.push_back(thisSpeed.rated_total_capacity);
    }
    _normalModeRatedCapacity = this->performance.normalMode.ratedGrossTotalCap;
}

CoilCoolingDXCurveFitSpeed & CoilCoolingDX::normModeNomSpeed()
{
    return this->performance.normalMode.speeds[this->performance.normalMode.nominalSpeedIndex];
}

CoilCoolingDXCurveFitSpeed & CoilCoolingDX::altModeNomSpeed()
{
    return this->performance.alternateMode.speeds[this->performance.alternateMode.nominalSpeedIndex];
}

void CoilCoolingDX::size() {
    this->performance.parentName = this->name;
    this->performance.size();
}

void CoilCoolingDX::simulate(bool useAlternateMode, Real64 PLR, int speedNum, Real64 speedRatio, int const fanOpMode)
{
    if (this->myOneTimeInitFlag) {
        this->oneTimeInit();
        this->myOneTimeInitFlag = false;
    }

    // get node references
    auto &evapInletNode = DataLoopNode::Node(this->evapInletNodeIndex);
    auto &evapOutletNode = DataLoopNode::Node(this->evapOutletNodeIndex);
    auto &condInletNode = DataLoopNode::Node(this->condInletNodeIndex);
    auto &condOutletNode = DataLoopNode::Node(this->condOutletNodeIndex);

    // set some reporting variables
    this->condenserInletTemperature = condInletNode.Temp;
    this->dehumidificationMode = useAlternateMode;

    // call the simulation, which returns useful data
    // TODO: check the avail schedule and reset data/pass through data as needed
    // TODO: check the minOATcompressor and reset data/pass through data as needed
    this->performance.simulate(evapInletNode, evapOutletNode, useAlternateMode, PLR, speedNum, speedRatio, fanOpMode, condInletNode, condOutletNode);
    EnergyPlus::CoilCoolingDX::passThroughNodeData(evapInletNode, evapOutletNode);

    // calculate energy conversion factor
    Real64 reportingConstant = DataHVACGlobals::TimeStepSys * DataGlobals::SecInHour;

    // update condensate collection tank
    if (this->condensateTankIndex > 0) {
        if (speedNum > 0) {
            // calculate and report condensation rates  (how much water extracted from the air stream)
            // water flow of water in m3/s for water system interactions
            Real64 averageTemp = (evapInletNode.Temp - evapOutletNode.Temp) / 2.0;
            Real64 waterDensity = Psychrometrics::RhoH2O(averageTemp);
            Real64 inHumidityRatio = evapInletNode.HumRat;
            Real64 outHumidityRatio = evapOutletNode.HumRat;
            this->condensateVolumeFlow = max(0.0, (evapInletNode.MassFlowRate * (inHumidityRatio - outHumidityRatio) / waterDensity));
            this->condensateVolumeConsumption = this->condensateVolumeFlow * reportingConstant;
            DataWater::WaterStorage(this->condensateTankIndex).VdotAvailSupply(this->condensateTankSupplyARRID) = this->condensateVolumeFlow;
            DataWater::WaterStorage(this->condensateTankIndex).TwaterSupply(this->condensateTankSupplyARRID) = evapOutletNode.Temp;
        } else {
            DataWater::WaterStorage(this->condensateTankIndex).VdotAvailSupply(this->condensateTankSupplyARRID) = 0.0;
            DataWater::WaterStorage(this->condensateTankIndex).TwaterSupply(this->condensateTankSupplyARRID) = evapOutletNode.Temp;
        }
    }

    // update requests for evaporative condenser tank
    if (this->evaporativeCondSupplyTankIndex > 0) {
        if (speedNum > 0) {
            Real64 condInletTemp =
                DataEnvironment::OutWetBulbTemp + (DataEnvironment::OutDryBulbTemp - DataEnvironment::OutWetBulbTemp) *
                                                      (1.0 - this->performance.normalMode.speeds[speedNum - 1].evap_condenser_effectiveness);
            Real64 condInletHumRat = Psychrometrics::PsyWFnTdbTwbPb(condInletTemp, DataEnvironment::OutWetBulbTemp, DataEnvironment::OutBaroPress);
            Real64 outdoorHumRat = DataEnvironment::OutHumRat;
            Real64 condAirMassFlow = condInletNode.MassFlowRate; // TODO: How is this getting a value?
            Real64 waterDensity = Psychrometrics::RhoH2O(DataEnvironment::OutDryBulbTemp);
            this->evaporativeCondSupplyTankVolumeFlow = (condInletHumRat - outdoorHumRat) * condAirMassFlow / waterDensity;
            this->evaporativeCondSupplyTankConsump = this->evaporativeCondSupplyTankVolumeFlow * reportingConstant;
            if (!useAlternateMode) {
                this->evapCondPumpElecPower = this->performance.normalMode.getCurrentEvapCondPumpPower(speedNum);
            }
            DataWater::WaterStorage(this->evaporativeCondSupplyTankIndex).VdotRequestDemand(this->evaporativeCondSupplyTankARRID) =
                this->evaporativeCondSupplyTankVolumeFlow;
        } else {
            DataWater::WaterStorage(this->evaporativeCondSupplyTankIndex).VdotRequestDemand(this->evaporativeCondSupplyTankARRID) = 0.0;
        }
    }

    // update report variables
    this->airMassFlowRate = evapOutletNode.MassFlowRate;
    this->inletAirDryBulbTemp = evapInletNode.Temp;
    this->inletAirHumRat = evapInletNode.HumRat;
    this->outletAirDryBulbTemp = evapOutletNode.Temp;
    this->outletAirHumRat = evapOutletNode.HumRat;

    this->totalCoolingEnergyRate = evapOutletNode.MassFlowRate * (evapInletNode.Enthalpy - evapOutletNode.Enthalpy);
    this->totalCoolingEnergy = this->totalCoolingEnergyRate * reportingConstant;
    Real64 minAirHumRat = min(evapInletNode.HumRat, evapOutletNode.HumRat);
    this->sensCoolingEnergyRate = evapOutletNode.MassFlowRate * (Psychrometrics::PsyHFnTdbW(evapInletNode.Temp, minAirHumRat) -
                                                                 Psychrometrics::PsyHFnTdbW(evapOutletNode.Temp, minAirHumRat));
    this->sensCoolingEnergy = this->sensCoolingEnergyRate * reportingConstant;
    this->latCoolingEnergyRate = this->totalCoolingEnergyRate - this->sensCoolingEnergyRate;
    this->latCoolingEnergy = this->latCoolingEnergyRate * reportingConstant;

    this->evapCondPumpElecConsumption = this->evapCondPumpElecPower * reportingConstant;

    this->coolingCoilRuntimeFraction = this->performance.RTF;
    this->elecCoolingPower = this->performance.powerUse;
    this->elecCoolingConsumption = this->performance.powerUse * reportingConstant;
    this->wasteHeatEnergyRate = this->performance.wasteHeatRate;
    this->wasteHeatEnergy = this->performance.wasteHeatRate * reportingConstant;

    this->partLoadRatioReport = PLR;
    this->speedNumReport = speedNum;
    this->speedRatioReport = speedRatio;

    if (this->isSecondaryDXCoilInZone) {
        // call CalcSecondaryDXCoils ???
        this->secCoilSensHeatRejEnergyRate = this->totalCoolingEnergyRate + this->elecCoolingPower;
        this->secCoilSensHeatRejEnergy = this->totalCoolingEnergy + this->elecCoolingConsumption;
    }

    // Fishy global things that need to be set here, try to set the AFN stuff now
    // This appears to be the only location where airLoopNum gets used
    //DataAirLoop::LoopDXCoilRTF = max(this->coolingCoilRuntimeFraction, DXCoil(DXCoilNum).HeatingCoilRuntimeFraction);
    DataAirLoop::LoopDXCoilRTF = this->coolingCoilRuntimeFraction;
    DataHVACGlobals::DXElecCoolingPower = this->elecCoolingPower;
    if (this->airLoopNum > 0) {
        DataAirLoop::AirLoopAFNInfo(this->airLoopNum).AFNLoopDXCoilRTF = this->coolingCoilRuntimeFraction;
        // The original calculation is below, but no heating yet
        //        max(DXCoil(DXCoilNum).CoolingCoilRuntimeFraction, DXCoil(DXCoilNum).HeatingCoilRuntimeFraction);
    }

    // report out to the coil sizing report if needed
    if (this->reportCoilFinalSizes) {
        if (!DataGlobals::WarmupFlag && !DataGlobals::DoingHVACSizingSimulations && !DataGlobals::DoingSizing) {

            // report out final coil sizing info
            Real64 ratedSensCap(0.0);
            ratedSensCap = this->performance.normalMode.ratedGrossTotalCap * this->normModeNomSpeed().grossRatedSHR;
            coilSelectionReportObj->setCoilFinalSizes(this->name,
                                                      coilCoolingDXObjectName,
                                                      this->performance.normalMode.ratedGrossTotalCap,
                                                      ratedSensCap,
                                                      this->performance.normalMode.ratedEvapAirFlowRate,
                                                      -999.0);

            // report out fan information
            if (this->supplyFanType == DataHVACGlobals::FanType_SystemModelObject) {
                if (this->supplyFanIndex >= 0) {
                    coilSelectionReportObj->setCoilSupplyFanInfo(this->name,
                                                                 coilCoolingDXObjectName,
                                                                 HVACFan::fanObjs[this->supplyFanIndex]->name,
                                                                 DataAirSystems::objectVectorOOFanSystemModel,
                                                                 this->supplyFanIndex);
                }
            } else {
                if (this->supplyFanIndex >= 1) {
                    coilSelectionReportObj->setCoilSupplyFanInfo(this->name,
                                                                 coilCoolingDXObjectName,
                                                                 Fans::Fan(this->supplyFanIndex).FanName,
                                                                 DataAirSystems::structArrayLegacyFanModels,
                                                                 this->supplyFanIndex);
                }
            }

            // report out coil rating conditions, just create a set of dummy nodes and run calculate on them
            DataLoopNode::NodeData dummyEvapInlet;
            DataLoopNode::NodeData dummyEvapOutlet;
            DataLoopNode::NodeData dummyCondInlet;
            DataLoopNode::NodeData dummyCondOutlet;
            Real64 dummyPLR = 1.0;
            int dummySpeedNum = 1;
            Real64 dummySpeedRatio = 1.0;
            int dummyFanOpMode = 1.0;

            Real64 const RatedInletAirTemp(26.6667);        // 26.6667C or 80F
            Real64 const RatedInletWetBulbTemp(19.44);      // 19.44 or 67F
            Real64 const RatedOutdoorAirTemp(35.0);         // 35 C or 95F
            Real64 ratedOutdoorAirWetBulb = 23.9; // from I/O ref. more precise value?

            Real64 ratedInletEvapMassFlowRate = this->performance.normalMode.ratedEvapAirMassFlowRate;
            dummyEvapInlet.MassFlowRate = ratedInletEvapMassFlowRate;
            dummyEvapInlet.Temp = RatedInletAirTemp;
            Real64 dummyInletAirHumRat = Psychrometrics::PsyWFnTdbTwbPb(RatedInletAirTemp, RatedInletWetBulbTemp, DataEnvironment::StdPressureSeaLevel, "Coil:Cooling:DX::simulate");
            dummyEvapInlet.HumRat = dummyInletAirHumRat;
            dummyEvapInlet.Enthalpy = Psychrometrics::PsyHFnTdbW(RatedInletAirTemp, dummyInletAirHumRat);

            // maybe we don't actually need to override weather below, we'll see
            dummyCondInlet.Temp = RatedOutdoorAirTemp;
            dummyCondInlet.HumRat = Psychrometrics::PsyWFnTdbTwbPb(RatedOutdoorAirTemp, ratedOutdoorAirWetBulb, DataEnvironment::StdPressureSeaLevel, "Coil:Cooling:DX::simulate");
            dummyCondInlet.OutAirWetBulb = ratedOutdoorAirWetBulb;
            dummyCondInlet.Press = condInletNode.Press; // for now; TODO: Investigate

            // overriding outdoor conditions temporarily
            Real64 holdOutDryBulbTemp = DataEnvironment::OutDryBulbTemp;
            Real64 holdOutHumRat = DataEnvironment::OutHumRat;
            Real64 holdOutWetBulb = DataEnvironment::OutWetBulbTemp;
            Real64 holdOutBaroPress = DataEnvironment::OutBaroPress;
            DataEnvironment::OutDryBulbTemp = RatedOutdoorAirTemp;
            DataEnvironment::OutWetBulbTemp = ratedOutdoorAirWetBulb;
            DataEnvironment::OutBaroPress = DataEnvironment::StdPressureSeaLevel; // assume rating is for sea level.
            DataEnvironment::OutHumRat = Psychrometrics::PsyWFnTdbTwbPb(RatedOutdoorAirTemp, ratedOutdoorAirWetBulb, DataEnvironment::StdPressureSeaLevel, "Coil:Cooling:DX::simulate");

            this->performance.simulate(dummyEvapInlet, dummyEvapOutlet, false, dummyPLR, dummySpeedNum, dummySpeedRatio, dummyFanOpMode, dummyCondInlet, dummyCondOutlet);

            // reset outdoor conditions back to previous state
            DataEnvironment::OutDryBulbTemp = holdOutDryBulbTemp;
            DataEnvironment::OutWetBulbTemp = holdOutWetBulb;
            DataEnvironment::OutBaroPress = holdOutBaroPress;
            DataEnvironment::OutHumRat = holdOutHumRat;

            Real64 const coolingRate = dummyEvapInlet.MassFlowRate * (dummyEvapInlet.Enthalpy - dummyEvapOutlet.Enthalpy);
            Real64 const thisMinAirHumRat = min(dummyEvapInlet.HumRat, dummyEvapOutlet.HumRat);
            Real64 const sensCoolingRate = dummyEvapInlet.MassFlowRate * (Psychrometrics::PsyHFnTdbW(dummyEvapInlet.Temp, thisMinAirHumRat) - Psychrometrics::PsyHFnTdbW(dummyEvapOutlet.Temp, thisMinAirHumRat));
            Real64 const ratedOutletWetBulb = Psychrometrics::PsyTwbFnTdbWPb(dummyEvapOutlet.Temp, dummyEvapOutlet.HumRat, DataEnvironment::StdPressureSeaLevel, "Coil:Cooling:DX::simulate");
            coilSelectionReportObj->setRatedCoilConditions(this->name,
                                                           coilCoolingDXObjectName,
                                                           coolingRate,
                                                           sensCoolingRate,
                                                           ratedInletEvapMassFlowRate,
                                                           RatedInletAirTemp,
                                                           dummyInletAirHumRat,
                                                           RatedInletWetBulbTemp,
                                                           dummyEvapOutlet.Temp,
                                                           dummyEvapOutlet.HumRat,
                                                           ratedOutletWetBulb,
                                                           RatedOutdoorAirTemp,
                                                           ratedOutdoorAirWetBulb,
                                                           this->normModeNomSpeed().RatedCBF, -999.0);

            this->reportCoilFinalSizes = false;
        }
    }

}

void CoilCoolingDX::setToHundredPercentDOAS() {
    for (auto & speed : this->performance.normalMode.speeds) {
        speed.minRatedVolFlowPerRatedTotCap = DataHVACGlobals::MinRatedVolFlowPerRatedTotCap2;
        speed.maxRatedVolFlowPerRatedTotCap = DataHVACGlobals::MaxRatedVolFlowPerRatedTotCap2;
    }
    if (this->performance.hasAlternateMode) {
        for (auto &speed : this->performance.alternateMode.speeds) {
            speed.minRatedVolFlowPerRatedTotCap = DataHVACGlobals::MinRatedVolFlowPerRatedTotCap2;
            speed.maxRatedVolFlowPerRatedTotCap = DataHVACGlobals::MaxRatedVolFlowPerRatedTotCap2;
        }
    }
}

void CoilCoolingDX::passThroughNodeData(EnergyPlus::DataLoopNode::NodeData &in, EnergyPlus::DataLoopNode::NodeData &out)
{
    // pass through all the other node variables that we don't update as a part of this model calculation
    out.MassFlowRate = in.MassFlowRate;
    out.Press = in.Press;
    out.Quality = in.Quality;
    out.MassFlowRateMax = in.MassFlowRateMax;
    out.MassFlowRateMin = in.MassFlowRateMin;
    out.MassFlowRateMaxAvail = in.MassFlowRateMaxAvail;
    out.MassFlowRateMinAvail = in.MassFlowRateMinAvail;
}

void CoilCoolingDX::reportAllStandardRatings(OutputFiles &outputFiles) {

    if (!coilCoolingDXs.empty()) {
        Real64 const ConvFromSIToIP(3.412141633);              // Conversion from SI to IP [3.412 Btu/hr-W]
        static constexpr auto Format_990(
                "! <DX Cooling Coil Standard Rating Information>, Component Type, Component Name, Standard Rating (Net) "
                "Cooling Capacity {W}, Standard Rated Net COP {W/W}, EER {Btu/W-h}, SEER {Btu/W-h}, IEER {Btu/W-h}\n");
        print(outputFiles.eio, "{}", Format_990);
        for (auto &coil : coilCoolingDXs) {
            coil.performance.calcStandardRatings210240();

            static constexpr auto Format_991(
                    " DX Cooling Coil Standard Rating Information, {}, {}, {:.1R}, {:.2R}, {:.2R}, {:.2R}, {:.2R}\n");
            print(outputFiles.eio, Format_991,
                  "Coil:Cooling:DX", coil.name,
                  coil.performance.standardRatingCoolingCapacity,
                  coil.performance.standardRatingEER,
                  coil.performance.standardRatingEER * ConvFromSIToIP,
                  coil.performance.standardRatingSEER * ConvFromSIToIP,
                  coil.performance.standardRatingIEER * ConvFromSIToIP);

            OutputReportPredefined::PreDefTableEntry(OutputReportPredefined::pdchDXCoolCoilType, coil.name,
                                                     "Coil:Cooling:DX");
            OutputReportPredefined::PreDefTableEntry(OutputReportPredefined::pdchDXCoolCoilNetCapSI, coil.name,
                                                     coil.performance.standardRatingCoolingCapacity, 1);
            // W/W is the same as Btuh/Btuh so that's fine too
            OutputReportPredefined::PreDefTableEntry(OutputReportPredefined::pdchDXCoolCoilCOP, coil.name,
                                                     coil.performance.standardRatingEER, 2);
            // Btu/W-h will convert to itself
            OutputReportPredefined::PreDefTableEntry(OutputReportPredefined::pdchDXCoolCoilEERIP, coil.name,
                                                     coil.performance.standardRatingEER * ConvFromSIToIP, 2);
            OutputReportPredefined::PreDefTableEntry(OutputReportPredefined::pdchDXCoolCoilSEERIP, coil.name,
                                                     coil.performance.standardRatingSEER * ConvFromSIToIP, 2);
            OutputReportPredefined::PreDefTableEntry(OutputReportPredefined::pdchDXCoolCoilIEERIP, coil.name,
                                                     coil.performance.standardRatingIEER * ConvFromSIToIP, 2);
            OutputReportPredefined::addFootNoteSubTable(OutputReportPredefined::pdstDXCoolCoil,
                                                        "ANSI/AHRI ratings account for supply air fan heat and electric power.");
        }
    }
    stillNeedToReportStandardRatings = false;
}
