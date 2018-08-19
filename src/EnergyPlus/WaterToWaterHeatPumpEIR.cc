// EnergyPlus, Copyright (c) 1996-2018, The Board of Trustees of the University of Illinois,
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

#include <string>
#include <vector>

#include <BranchNodeConnections.hh>
#include <DataGlobals.hh>
#include <DataIPShortCuts.hh>
#include <DataLoopNode.hh>
#include <DataPlant.hh>
#include <FluidProperties.hh>
#include <InputProcessing/InputProcessor.hh>
#include <NodeInputManager.hh>
#include <OutputProcessor.hh>
#include <PlantComponent.hh>
#include <PlantUtilities.hh>
#include <UtilityRoutines.hh>
#include <WaterToWaterHeatPumpEIR.hh>
#include "CurveManager.hh"

namespace EnergyPlus {
    namespace EIRWaterToWaterHeatPumps {

        bool getInputsWWHP(true);
        std::vector<EIRWaterToWaterHeatPump> eir_wwhp;

        void EIRWaterToWaterHeatPump::clear_state() {
            getInputsWWHP = true;
            eir_wwhp.clear();
        }

        Real64 EIRWaterToWaterHeatPump::getLoadSideOutletSetpointTemp() {
            auto &thisLoadPlantLoop = DataPlant::PlantLoop(this->loadSideLocation.loopNum);
            auto &thisLoadLoopSide = thisLoadPlantLoop.LoopSide(this->loadSideLocation.loopSideNum);
            auto &thisLoadBranch = thisLoadLoopSide.Branch(this->loadSideLocation.branchNum);
            auto &thisLoadComp = thisLoadBranch.Comp(this->loadSideLocation.compNum);
            if (thisLoadPlantLoop.LoopDemandCalcScheme == DataPlant::SingleSetPoint) {
                if (thisLoadComp.CurOpSchemeType == DataPlant::CompSetPtBasedSchemeType) {
                    // there will be a valid setpoint on outlet
                    return DataLoopNode::Node(this->loadSideNodes.outlet).TempSetPoint;
                } else { // use plant loop overall setpoint
                    return DataLoopNode::Node(thisLoadPlantLoop.TempSetPointNodeNum).TempSetPoint;
                }
            } else if (thisLoadPlantLoop.LoopDemandCalcScheme == DataPlant::DualSetPointDeadBand) {
                if (thisLoadComp.CurOpSchemeType == DataPlant::CompSetPtBasedSchemeType) {
                    // there will be a valid setpoint on outlet
                    return DataLoopNode::Node(this->loadSideNodes.outlet).TempSetPointHi;
                } else { // use plant loop overall setpoint
                    return DataLoopNode::Node(thisLoadPlantLoop.TempSetPointNodeNum).TempSetPointHi;
                }
            }
        }

        void EIRWaterToWaterHeatPump::simulate(const EnergyPlus::PlantLocation &calledFromLocation,
                                               bool const FirstHVACIteration,
                                               Real64 &CurLoad,
                                               bool const RunFlag) {
            std::string const routineName = "WaterToWaterHeatPumpEIR::simulate";

            if (calledFromLocation.loopNum == this->sourceSideLocation.loopNum) { // condenser side
                PlantUtilities::UpdateChillerComponentCondenserSide(this->sourceSideLocation.loopNum,
                                                                    this->sourceSideLocation.loopSideNum,
                                                                    this->plantTypeOfNum,
                                                                    this->sourceSideNodes.inlet,
                                                                    this->sourceSideNodes.outlet,
                                                                    this->sourceSideHeatTransfer,
                                                                    this->sourceSideInletTemp,
                                                                    this->sourceSideOutletTemp,
                                                                    this->sourceSideMassFlowRate,
                                                                    FirstHVACIteration);
                return;
            }

            this->running = RunFlag;
            if (!this->running) {
                this->loadSideMassFlowRate = 0.0;
                this->sourceSideMassFlowRate = 0.0;
                PlantUtilities::SetComponentFlowRate(this->loadSideMassFlowRate,
                                                     this->loadSideNodes.inlet,
                                                     this->loadSideNodes.outlet,
                                                     this->loadSideLocation.loopNum,
                                                     this->loadSideLocation.loopSideNum,
                                                     this->loadSideLocation.branchNum,
                                                     this->loadSideLocation.compNum);
                PlantUtilities::SetComponentFlowRate(this->sourceSideMassFlowRate,
                                                     this->sourceSideNodes.inlet,
                                                     this->sourceSideNodes.outlet,
                                                     this->sourceSideLocation.loopNum,
                                                     this->sourceSideLocation.loopSideNum,
                                                     this->sourceSideLocation.branchNum,
                                                     this->sourceSideLocation.compNum);
                PlantUtilities::PullCompInterconnectTrigger(this->loadSideLocation.loopNum,
                                                            this->loadSideLocation.loopSideNum,
                                                            this->loadSideLocation.branchNum,
                                                            this->loadSideLocation.compNum,
                                                            this->condMassFlowRateTriggerIndex,
                                                            this->sourceSideLocation.loopNum,
                                                            this->sourceSideLocation.loopSideNum,
                                                            DataPlant::CriteriaType_MassFlowRate,
                                                            this->sourceSideMassFlowRate);
                // Set flows if the heat pump is running
            } else { // the heat pump must run
                this->loadSideMassFlowRate = this->loadSideDesignMassFlowRate;
                this->sourceSideMassFlowRate = this->sourceSideDesignMassFlowRate;
                PlantUtilities::SetComponentFlowRate(this->loadSideMassFlowRate,
                                                     this->loadSideNodes.inlet,
                                                     this->loadSideNodes.outlet,
                                                     this->loadSideLocation.loopNum,
                                                     this->loadSideLocation.loopSideNum,
                                                     this->loadSideLocation.branchNum,
                                                     this->loadSideLocation.compNum);
                PlantUtilities::SetComponentFlowRate(this->sourceSideMassFlowRate,
                                                     this->sourceSideNodes.inlet,
                                                     this->sourceSideNodes.outlet,
                                                     this->sourceSideLocation.loopNum,
                                                     this->sourceSideLocation.loopSideNum,
                                                     this->sourceSideLocation.branchNum,
                                                     this->sourceSideLocation.compNum);

                // if there's no flow in one, try to turn the entire heat pump off
                if (this->loadSideMassFlowRate <= 0.0 || this->sourceSideMassFlowRate <= 0.0) {

                    this->loadSideMassFlowRate = 0.0;
                    this->sourceSideMassFlowRate = 0.0;
                    this->running = false;

                    PlantUtilities::SetComponentFlowRate(this->loadSideMassFlowRate,
                                                         this->loadSideNodes.inlet,
                                                         this->loadSideNodes.outlet,
                                                         this->loadSideLocation.loopNum,
                                                         this->loadSideLocation.loopSideNum,
                                                         this->loadSideLocation.branchNum,
                                                         this->loadSideLocation.compNum);
                    PlantUtilities::SetComponentFlowRate(this->sourceSideMassFlowRate,
                                                         this->sourceSideNodes.inlet,
                                                         this->sourceSideNodes.outlet,
                                                         this->sourceSideLocation.loopNum,
                                                         this->sourceSideLocation.loopSideNum,
                                                         this->sourceSideLocation.branchNum,
                                                         this->sourceSideLocation.compNum);
                    PlantUtilities::PullCompInterconnectTrigger(this->loadSideLocation.loopNum,
                                                                this->loadSideLocation.loopSideNum,
                                                                this->loadSideLocation.branchNum,
                                                                this->loadSideLocation.compNum,
                                                                this->condMassFlowRateTriggerIndex,
                                                                this->sourceSideLocation.loopNum,
                                                                this->sourceSideLocation.loopSideNum,
                                                                DataPlant::CriteriaType_MassFlowRate,
                                                                this->sourceSideMassFlowRate);
                }
                PlantUtilities::PullCompInterconnectTrigger(this->loadSideLocation.loopNum,
                                                            this->loadSideLocation.loopSideNum,
                                                            this->loadSideLocation.branchNum,
                                                            this->loadSideLocation.compNum,
                                                            this->condMassFlowRateTriggerIndex,
                                                            this->sourceSideLocation.loopNum,
                                                            this->sourceSideLocation.loopSideNum,
                                                            DataPlant::CriteriaType_MassFlowRate,
                                                            this->sourceSideMassFlowRate);
            }
            this->loadSideInletTemp = DataLoopNode::Node(this->loadSideNodes.inlet).Temp;
            this->sourceSideInletTemp = DataLoopNode::Node(this->sourceSideNodes.inlet).Temp;

            if (this->loadSideMassFlowRate > 0 && this->sourceSideMassFlowRate > 0) {

                Real64 loadSideOutletSetpointTemp = this->getLoadSideOutletSetpointTemp();
                Real64 const sourceInletNodeTemp = DataLoopNode::Node(this->sourceSideNodes.inlet).Temp;
                Real64 capacityModifierFuncTemp = CurveManager::CurveValue(
                        this->capFuncTempCurveIndex, loadSideOutletSetpointTemp, sourceInletNodeTemp
                );
                Real64 availableCapacity = this->referenceCapacity * capacityModifierFuncTemp;
                Real64 partLoadRatio = 0.0;
                if (availableCapacity > 0) {
                    partLoadRatio = max(0.0, min(std::abs(CurLoad) / availableCapacity, 1.0));
                }

                auto &thisLoadPlantLoop = DataPlant::PlantLoop(this->loadSideLocation.loopNum);
                Real64 Cp = FluidProperties::GetSpecificHeatGlycol(
                        thisLoadPlantLoop.FluidName,
                        DataLoopNode::Node(this->loadSideNodes.inlet).Temp,
                        thisLoadPlantLoop.FluidIndex,
                        "WWHPEIR::simulate()"
                );

                this->loadSideHeatTransfer = availableCapacity * partLoadRatio;

                if (this->plantTypeOfNum == DataPlant::TypeOf_HeatPumpEIRHeating) {

                    // calculate load side
                    Real64 const loadMCp = this->loadSideMassFlowRate * Cp;
                    this->loadSideOutletTemp = this->loadSideInletTemp + this->loadSideHeatTransfer / loadMCp;

                    // calculate power usage from EIR curves
                    Real64 eirModifierFuncTemp = CurveManager::CurveValue(this->powerRatioFuncTempCurveIndex,
                                                                          this->loadSideOutletTemp,
                                                                          this->sourceSideInletTemp);
                    Real64 eirModifierFuncPLR = CurveManager::CurveValue(this->powerRatioFuncPLRCurveIndex,
                                                                         partLoadRatio);
                    Real64 ReferenceCOP = 3.14;  // use proper value
                    this->powerUsage = (availableCapacity / ReferenceCOP) * eirModifierFuncPLR * eirModifierFuncTemp;

                    // energy balance on coil
                    this->sourceSideHeatTransfer = this->loadSideHeatTransfer - this->powerUsage;

                    // calculate source side
                    Real64 const sourceMCp = this->sourceSideMassFlowRate * Cp;
                    this->sourceSideOutletTemp = this->sourceSideInletTemp - this->sourceSideHeatTransfer / sourceMCp;

                } else if (this->plantTypeOfNum == DataPlant::TypeOf_HeatPumpEIRCooling) {

                    // calculate load side
                    Real64 const loadMCp = this->loadSideMassFlowRate * Cp;
                    this->loadSideOutletTemp = this->loadSideInletTemp - this->loadSideHeatTransfer / loadMCp;

                    // calculate power usage from EIR curves
                    Real64 eirModifierFuncTemp = CurveManager::CurveValue(this->powerRatioFuncTempCurveIndex,
                                                                          this->loadSideOutletTemp,
                                                                          this->sourceSideInletTemp);
                    Real64 eirModifierFuncPLR = CurveManager::CurveValue(this->powerRatioFuncPLRCurveIndex,
                                                                         partLoadRatio);
                    Real64 ReferenceCOP = 3.14;  // use proper value
                    this->powerUsage = (availableCapacity / ReferenceCOP) * eirModifierFuncPLR * eirModifierFuncTemp;

                    // energy balance on coil
                    this->sourceSideHeatTransfer = this->loadSideHeatTransfer + this->powerUsage;

                    // calculate source side
                    Real64 const sourceMCp = this->sourceSideMassFlowRate * Cp;
                    this->sourceSideOutletTemp = this->sourceSideInletTemp + this->sourceSideHeatTransfer / sourceMCp;

                }
            } else {
                this->loadSideHeatTransfer = 0.0;
                this->loadSideOutletTemp = this->loadSideInletTemp;
                this->powerUsage = 0.0;
                this->sourceSideHeatTransfer = 0.0;
                this->sourceSideOutletTemp = this->sourceSideInletTemp;
            }

            // update nodes
            DataLoopNode::Node(this->loadSideNodes.outlet).Temp = this->loadSideOutletTemp;
            DataLoopNode::Node(this->sourceSideNodes.outlet).Temp = this->sourceSideOutletTemp;
        }

        void EIRWaterToWaterHeatPump::onInitLoopEquip(const PlantLocation &EP_UNUSED(calledFromLocation)) {
            std::string const routineName = "initWaterToWaterHeatPumpEIR";

            if (this->oneTimeInit) {
                // setup output variables
                SetupOutputVariable(
                        "EIR WWHP Load Side Heat Transfer", OutputProcessor::Unit::W, this->loadSideHeatTransfer,
                        "System", "Average", this->name);
                SetupOutputVariable(
                        "EIR WWHP Source Side Heat Transfer", OutputProcessor::Unit::W, this->sourceSideHeatTransfer,
                        "System", "Average", this->name);
                SetupOutputVariable(
                        "EIR WWHP Load Side Inlet Temperature", OutputProcessor::Unit::C, this->loadSideInletTemp,
                        "System", "Average", this->name);
                SetupOutputVariable(
                        "EIR WWHP Load Side Outlet Temperature", OutputProcessor::Unit::C, this->loadSideOutletTemp,
                        "System", "Average", this->name);
                SetupOutputVariable(
                        "EIR WWHP Source Side Inlet Temperature", OutputProcessor::Unit::C, this->sourceSideInletTemp,
                        "System", "Average", this->name);
                SetupOutputVariable(
                        "EIR WWHP Source Side Outlet Temperature", OutputProcessor::Unit::C, this->sourceSideOutletTemp,
                        "System", "Average", this->name);
                SetupOutputVariable("EIR WWHP Power Usage", OutputProcessor::Unit::W, this->powerUsage, "System",
                                    "Average", this->name);

                // find this component on the plant
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(this->name,
                                                        this->plantTypeOfNum,
                                                        this->loadSideLocation.loopNum,
                                                        this->loadSideLocation.loopSideNum,
                                                        this->loadSideLocation.branchNum,
                                                        this->loadSideLocation.compNum,
                                                        _,
                                                        _,
                                                        _,
                                                        this->loadSideNodes.inlet,
                                                        _,
                                                        errFlag);

                if (this->loadSideLocation.loopSideNum != DataPlant::SupplySide) { // throw error
                    ShowSevereError(routineName + ": Invalid connections for " +
                                    DataPlant::ccSimPlantEquipTypes(this->plantTypeOfNum) + " name = \"" +
                                    this->name + "\"");
                    ShowContinueError("The load side connections are not on the Supply Side of a plant loop");
                    errFlag = true;
                }

                PlantUtilities::ScanPlantLoopsForObject(this->name,
                                                        this->plantTypeOfNum,
                                                        this->sourceSideLocation.loopNum,
                                                        this->sourceSideLocation.loopSideNum,
                                                        this->sourceSideLocation.branchNum,
                                                        this->sourceSideLocation.compNum,
                                                        _,
                                                        _,
                                                        _,
                                                        this->sourceSideNodes.inlet,
                                                        _,
                                                        errFlag);

                if (this->sourceSideLocation.loopSideNum != DataPlant::DemandSide) { // throw error
                    ShowSevereError(routineName + ": Invalid connections for " +
                                    DataPlant::ccSimPlantEquipTypes(this->plantTypeOfNum) + " name = \"" +
                                    this->name + "\"");
                    ShowContinueError("The source side connections are not on the Demand Side of a plant loop");
                    errFlag = true;
                }

                // make sure it is not the same loop on both sides.
                if (this->loadSideLocation.loopNum ==
                    this->sourceSideLocation.loopNum) { // user is being too tricky, don't allow
                    ShowSevereError(routineName + ": Invalid connections for " +
                                    DataPlant::ccSimPlantEquipTypes(this->plantTypeOfNum) + " name = \"" +
                                    this->name + "\"");
                    ShowContinueError("The load and source sides need to be on different loops.");
                    errFlag = true;
                } else {

                    PlantUtilities::InterConnectTwoPlantLoopSides(this->loadSideLocation.loopNum,
                                                                  this->loadSideLocation.loopSideNum,
                                                                  this->sourceSideLocation.loopNum,
                                                                  this->sourceSideLocation.loopSideNum,
                                                                  this->plantTypeOfNum,
                                                                  true);
                }

                if (errFlag) {
                    ShowFatalError(routineName + ": Program terminated due to previous condition(s).");
                }
                this->oneTimeInit = false;
            } // plant setup

            if (DataGlobals::BeginEnvrnFlag && this->envrnInit && DataPlant::PlantFirstSizesOkayToFinalize) {
                Real64 rho = FluidProperties::GetDensityGlycol(
                        DataPlant::PlantLoop(this->loadSideLocation.loopNum).FluidName,
                        DataGlobals::InitConvTemp,
                        DataPlant::PlantLoop(this->loadSideLocation.loopNum).FluidIndex,
                        routineName);
                this->loadSideDesignMassFlowRate = rho * this->loadSideDesignVolFlowRate;
                PlantUtilities::InitComponentNodes(0.0,
                                                   this->loadSideDesignMassFlowRate,
                                                   this->loadSideNodes.inlet,
                                                   this->loadSideNodes.outlet,
                                                   this->loadSideLocation.loopNum,
                                                   this->loadSideLocation.loopSideNum,
                                                   this->loadSideLocation.branchNum,
                                                   this->loadSideLocation.compNum);

                rho = FluidProperties::GetDensityGlycol(
                        DataPlant::PlantLoop(this->sourceSideLocation.loopNum).FluidName,
                        DataGlobals::InitConvTemp,
                        DataPlant::PlantLoop(this->sourceSideLocation.loopNum).FluidIndex,
                        routineName);
                this->sourceSideDesignMassFlowRate = rho * this->sourceSideDesignVolFlowRate;
                PlantUtilities::InitComponentNodes(0.0,
                                                   this->sourceSideDesignMassFlowRate,
                                                   this->sourceSideNodes.inlet,
                                                   this->sourceSideNodes.outlet,
                                                   this->sourceSideLocation.loopNum,
                                                   this->sourceSideLocation.loopSideNum,
                                                   this->sourceSideLocation.branchNum,
                                                   this->sourceSideLocation.compNum);
                this->envrnInit = false;
            }
            if (!DataGlobals::BeginEnvrnFlag) {
                this->envrnInit = true;
            }
        }

        PlantComponent *EIRWaterToWaterHeatPump::factory(int plantTypeOfNum, std::string objectName) {
            if (getInputsWWHP) {
                EIRWaterToWaterHeatPump::processInputForEIRWWHPHeating();
                EIRWaterToWaterHeatPump::processInputForEIRWWHPCooling();
                EIRWaterToWaterHeatPump::pairUpCompanionCoils();
                getInputsWWHP = false;
            }

            for (auto &wwhp : eir_wwhp) {
                if (wwhp.name == UtilityRoutines::MakeUPPERCase(objectName) && wwhp.plantTypeOfNum == plantTypeOfNum) {
                    return &wwhp;
                }
            }

            ShowFatalError("EIR_WWHP factory: Error getting inputs for wwhp named: " + objectName);
            return nullptr;
        }

        void EIRWaterToWaterHeatPump::pairUpCompanionCoils() {
            for (auto &thisHP : eir_wwhp) {
                if (!thisHP.companionCoilName.empty()) {
                    auto thisCoilName = UtilityRoutines::MakeUPPERCase(thisHP.name);
                    auto &thisCoilType = thisHP.plantTypeOfNum;
                    auto targetCompanionName = UtilityRoutines::MakeUPPERCase(thisHP.companionCoilName);
                    for (auto &potentialCompanionCoil : eir_wwhp) {
                        auto &potentialCompanionType = potentialCompanionCoil.plantTypeOfNum;
                        auto potentialCompanionName = UtilityRoutines::MakeUPPERCase(potentialCompanionCoil.name);
                        if (potentialCompanionName == thisCoilName) {
                            // skip the current coil
                            continue;
                        }
                        if (potentialCompanionName == targetCompanionName) {
                            if (thisCoilType == potentialCompanionType) {
                                ShowFatalError(
                                        "I'm sorry, I don't really feel comfortable pairing up coils of the same type.");
                            }
                            thisHP.companionHeatPumpCoil = &potentialCompanionCoil;
                            break;
                        }
                    }
                    if (!thisHP.companionHeatPumpCoil) {
                        ShowSevereError("Could not find matching companion heat pump coil.");
                        ShowContinueError("Base coil: " + thisCoilName);
                        ShowContinueError("Looking for companion coil named: " + targetCompanionName);
                        ShowFatalError("Simulation aborts due to previous severe error");
                    }
                }
            }
        }

        void EIRWaterToWaterHeatPump::processInputForEIRWWHPHeating() {
            using namespace DataIPShortCuts;

            bool errorsFound = false;

            cCurrentModuleObject = "HeatPump:WaterToWater:EIR:Heating";
            int numWWHP = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
            if (numWWHP > 0) {
                auto const instances = inputProcessor->epJSON.find(cCurrentModuleObject);
                if (instances == inputProcessor->epJSON.end()) {
                    errorsFound = true;
                }
                auto &instancesValue = instances.value();
                for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                    auto const &fields = instance.value();
                    auto const &thisObjectName = instance.key();

                    EIRWaterToWaterHeatPump thisWWHP;
                    thisWWHP.plantTypeOfNum = DataPlant::TypeOf_HeatPumpEIRHeating;
                    thisWWHP.name = UtilityRoutines::MakeUPPERCase(thisObjectName);
                    std::string loadSideInletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("load_side_inlet_node_name")
                    );
                    std::string loadSideOutletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("load_side_outlet_node_name")
                    );
                    std::string sourceSideInletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("source_side_inlet_node_name")
                    );
                    std::string sourceSideOutletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("source_side_outlet_node_name")
                    );
                    if (fields.find("companion_cooling_coil_name") != fields.end()) {  // optional field
                        thisWWHP.companionCoilName = UtilityRoutines::MakeUPPERCase(
                                fields.at("companion_cooling_coil_name")
                        );
                    }
                    thisWWHP.loadSideDesignVolFlowRate = fields.at("load_side_reference_flow_rate");
                    thisWWHP.sourceSideDesignVolFlowRate = fields.at("source_side_reference_flow_rate");
                    thisWWHP.referenceCapacity = fields.at("reference_capacity");
                    thisWWHP.referenceCOP = fields.at("reference_cop");
                    thisWWHP.referenceLeavingLoadSideTemp = fields.at("reference_leaving_load_side_water_temperature");
                    thisWWHP.referenceEnteringSourceSideTemp = fields.at(
                            "reference_entering_source_side_fluid_temperature");
                    thisWWHP.capFuncTempCurveIndex = CurveManager::GetCurveIndex(UtilityRoutines::MakeUPPERCase(
                            fields.at("heating_capacity_function_of_temperature_curve_name")));
                    thisWWHP.powerRatioFuncTempCurveIndex = CurveManager::GetCurveIndex(UtilityRoutines::MakeUPPERCase(
                            fields.at("electric_input_to_heating_output_ratio_function_of_temperature_curve_name")));
                    thisWWHP.powerRatioFuncPLRCurveIndex = CurveManager::GetCurveIndex(UtilityRoutines::MakeUPPERCase(
                            fields.at("electric_input_to_heating_output_ratio_function_of_temperature_curve_name")));

                    int const flowPath1 = 1, flowPath2 = 2;
                    thisWWHP.loadSideNodes.inlet = NodeInputManager::GetOnlySingleNode(loadSideInletNodeName,
                                                                                       errorsFound,
                                                                                       cCurrentModuleObject,
                                                                                       thisWWHP.name,
                                                                                       DataLoopNode::NodeType_Water,
                                                                                       DataLoopNode::NodeConnectionType_Inlet,
                                                                                       flowPath1,
                                                                                       DataLoopNode::ObjectIsNotParent);
                    thisWWHP.loadSideNodes.outlet = NodeInputManager::GetOnlySingleNode(loadSideOutletNodeName,
                                                                                        errorsFound,
                                                                                        cCurrentModuleObject,
                                                                                        thisWWHP.name,
                                                                                        DataLoopNode::NodeType_Water,
                                                                                        DataLoopNode::NodeConnectionType_Outlet,
                                                                                        flowPath1,
                                                                                        DataLoopNode::ObjectIsNotParent);
                    thisWWHP.sourceSideNodes.inlet = NodeInputManager::GetOnlySingleNode(sourceSideInletNodeName,
                                                                                         errorsFound,
                                                                                         cCurrentModuleObject,
                                                                                         thisWWHP.name,
                                                                                         DataLoopNode::NodeType_Water,
                                                                                         DataLoopNode::NodeConnectionType_Inlet,
                                                                                         flowPath2,
                                                                                         DataLoopNode::ObjectIsNotParent);
                    thisWWHP.sourceSideNodes.outlet = NodeInputManager::GetOnlySingleNode(sourceSideOutletNodeName,
                                                                                          errorsFound,
                                                                                          cCurrentModuleObject,
                                                                                          thisWWHP.name,
                                                                                          DataLoopNode::NodeType_Water,
                                                                                          DataLoopNode::NodeConnectionType_Outlet,
                                                                                          flowPath2,
                                                                                          DataLoopNode::ObjectIsNotParent);
                    BranchNodeConnections::TestCompSet(
                            cCurrentModuleObject, thisWWHP.name, loadSideInletNodeName, loadSideOutletNodeName,
                            "Hot Water Nodes");
                    BranchNodeConnections::TestCompSet(
                            cCurrentModuleObject, thisWWHP.name, sourceSideInletNodeName, sourceSideOutletNodeName,
                            "Condenser Water Nodes");

                    eir_wwhp.push_back(thisWWHP);
                }
            }
        }

        void EIRWaterToWaterHeatPump::processInputForEIRWWHPCooling() {
            using namespace DataIPShortCuts;

            bool errorsFound = false;

            cCurrentModuleObject = "HeatPump:WaterToWater:EIR:Cooling";
            int numWWHP = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
            if (numWWHP > 0) {
                auto const instances = inputProcessor->epJSON.find(cCurrentModuleObject);
                if (instances == inputProcessor->epJSON.end()) {
                    errorsFound = true;
                }
                auto &instancesValue = instances.value();
                for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                    auto const &fields = instance.value();
                    auto const &thisObjectName = instance.key();

                    EIRWaterToWaterHeatPump thisWWHP;
                    thisWWHP.plantTypeOfNum = DataPlant::TypeOf_HeatPumpEIRCooling;
                    thisWWHP.name = UtilityRoutines::MakeUPPERCase(thisObjectName);
                    std::string loadSideInletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("load_side_inlet_node_name")
                    );
                    std::string loadSideOutletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("load_side_outlet_node_name")
                    );
                    std::string sourceSideInletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("source_side_inlet_node_name")
                    );
                    std::string sourceSideOutletNodeName = UtilityRoutines::MakeUPPERCase(
                            fields.at("source_side_outlet_node_name")
                    );
                    if (fields.find("companion_heating_coil_name") != fields.end()) {  // optional field
                        thisWWHP.companionCoilName = UtilityRoutines::MakeUPPERCase(
                                fields.at("companion_heating_coil_name")
                        );
                    }
                    thisWWHP.loadSideDesignVolFlowRate = fields.at("load_side_reference_flow_rate");
                    thisWWHP.sourceSideDesignVolFlowRate = fields.at("source_side_reference_flow_rate");
                    thisWWHP.referenceCapacity = fields.at("reference_capacity");
                    thisWWHP.referenceCOP = fields.at("reference_cop");
                    thisWWHP.referenceLeavingLoadSideTemp = fields.at("reference_leaving_load_side_water_temperature");
                    thisWWHP.referenceEnteringSourceSideTemp = fields.at(
                            "reference_entering_source_side_fluid_temperature");
                    thisWWHP.capFuncTempCurveIndex = CurveManager::GetCurveIndex(UtilityRoutines::MakeUPPERCase(
                            fields.at("cooling_capacity_function_of_temperature_curve_name")));
                    thisWWHP.powerRatioFuncTempCurveIndex = CurveManager::GetCurveIndex(UtilityRoutines::MakeUPPERCase(
                            fields.at("electric_input_to_cooling_output_ratio_function_of_temperature_curve_name")));
                    thisWWHP.powerRatioFuncPLRCurveIndex = CurveManager::GetCurveIndex(UtilityRoutines::MakeUPPERCase(
                            fields.at("electric_input_to_cooling_output_ratio_function_of_temperature_curve_name")));

                    int const flowPath1 = 1, flowPath2 = 2;
                    thisWWHP.loadSideNodes.inlet = NodeInputManager::GetOnlySingleNode(loadSideInletNodeName,
                                                                                       errorsFound,
                                                                                       cCurrentModuleObject,
                                                                                       thisWWHP.name,
                                                                                       DataLoopNode::NodeType_Water,
                                                                                       DataLoopNode::NodeConnectionType_Inlet,
                                                                                       flowPath1,
                                                                                       DataLoopNode::ObjectIsNotParent);
                    thisWWHP.loadSideNodes.outlet = NodeInputManager::GetOnlySingleNode(loadSideOutletNodeName,
                                                                                        errorsFound,
                                                                                        cCurrentModuleObject,
                                                                                        thisWWHP.name,
                                                                                        DataLoopNode::NodeType_Water,
                                                                                        DataLoopNode::NodeConnectionType_Outlet,
                                                                                        flowPath1,
                                                                                        DataLoopNode::ObjectIsNotParent);
                    thisWWHP.sourceSideNodes.inlet = NodeInputManager::GetOnlySingleNode(sourceSideInletNodeName,
                                                                                         errorsFound,
                                                                                         cCurrentModuleObject,
                                                                                         thisWWHP.name,
                                                                                         DataLoopNode::NodeType_Water,
                                                                                         DataLoopNode::NodeConnectionType_Inlet,
                                                                                         flowPath2,
                                                                                         DataLoopNode::ObjectIsNotParent);
                    thisWWHP.sourceSideNodes.outlet = NodeInputManager::GetOnlySingleNode(sourceSideOutletNodeName,
                                                                                          errorsFound,
                                                                                          cCurrentModuleObject,
                                                                                          thisWWHP.name,
                                                                                          DataLoopNode::NodeType_Water,
                                                                                          DataLoopNode::NodeConnectionType_Outlet,
                                                                                          flowPath2,
                                                                                          DataLoopNode::ObjectIsNotParent);
                    BranchNodeConnections::TestCompSet(
                            cCurrentModuleObject, thisWWHP.name, loadSideInletNodeName, loadSideOutletNodeName,
                            "Hot Water Nodes");
                    BranchNodeConnections::TestCompSet(
                            cCurrentModuleObject, thisWWHP.name, sourceSideInletNodeName, sourceSideOutletNodeName,
                            "Condenser Water Nodes");

                    eir_wwhp.push_back(thisWWHP);
                }
            }
        }

    } // namespace EIRWaterToWaterHeatPumps
} // namespace EnergyPlus
