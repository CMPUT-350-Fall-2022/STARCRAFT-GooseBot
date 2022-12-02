#include "GooseBot.h"
                 
void GooseBot::VerifyBuild(){
    Units built_structs = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(struct_units));
    built_types.clear();
    for (auto s : built_structs){
        built_types.push_back(s->unit_type);
    }
    build_phase = built_types.size();
}

// Assumes up-to-date num_bases
bool GooseBot::BuildPhase(){
    // OnUnitCreated could be keeping this updated?
    // but for now / in case of structural destruction or other mishap
    VerifyBuild();
    // Find the next item to build by iterating over desired structures,
    // setting the structure as next to build if we do not have it (or enough of it)
    auto to_build = struct_targets.end();
    for (auto it = struct_targets.begin(); it < struct_targets.end(); ++it){
        auto found = std::find(built_types.begin(), built_types.end(), (*it).first);
        if (found == built_types.end()){
            to_build = it;
            if ((*to_build).first == UNIT_TYPEID::ZERG_HATCHERY){
                //only build new hatchery on proper phase or if bases count too low for higher phases
                if (build_phase == 4 || (build_phase > 4 && num_bases < 3)){
                    break;
                }else{
                    continue;
                }
            }else{
                break;
            }
        }
    }// Build next structure based on its custom build function, if it has one
    for (size_t j = 0; j < struct_targets.size(); ++j){
        if (build_phase != 1 && CountUnitType(UNIT_TYPEID::ZERG_EXTRACTOR) < num_bases*2){
            std::cout << "Trying to build Extractor" << std::endl;
            return TryMorphExtractor();
        }
        else if ((*to_build).first == UNIT_TYPEID::ZERG_HATCHERY){
            std::cout << "Trying to build Hatchery" << std::endl;
            return TryBuildHatchery();
        }
        else if ((*to_build).first == UNIT_TYPEID::ZERG_LAIR){
            std::cout << "Trying to morph Lair" << std::endl;
            return TryMorphLair();
        }
        else if ((*to_build).first == UNIT_TYPEID::ZERG_HIVE){
            return TryMorphHive();
        }
        else {
            std::cout << "Trying to build generic building" << std::endl;
            return TryBuildStructure((*to_build).second, (*to_build).first);
        }
    }
}

//Check if can afford upgrade
bool GooseBot::CanAfford(UPGRADE_ID upgrade){
    const ObservationInterface* observation = Observation();
    int mineral_supply = observation->GetMinerals();
    int gas_supply = observation->GetVespene();
    auto const upgrade_data = observation->GetUpgradeData();
    for (auto data : upgrade_data){
        if (data.upgrade_id == static_cast<uint32_t>(upgrade)){ 
            if ( (mineral_supply >= data.mineral_cost) && (gas_supply >= data.vespene_cost)){
                return true;
            }else{
                return false;
            }
        }
    }
    std::cout << "data does not contain the ability ";
    return false;
}

//Check if can afford unit
bool GooseBot::CanAfford(UNIT_TYPEID unit){
    const ObservationInterface* observation = Observation();
    int mineral_supply = observation->GetMinerals();
    int gas_supply = observation->GetVespene();
    auto const unit_data = observation->GetUnitTypeData();
    for (auto data : unit_data){
        if (data.unit_type_id == unit){ 
            if ( (mineral_supply >= data.mineral_cost) && (gas_supply >= data.vespene_cost)){
                if ( ((data.tech_requirement != UNIT_TYPEID::INVALID) && (CountUnitType(data.tech_requirement) > 0))
                    || (data.tech_requirement == UNIT_TYPEID::INVALID) ){
                    return true;
                }
                return false;
            }
            return false;
        }
    }
    return false;
}

// Returns number of allied units of given type
size_t GooseBot::CountUnitType(UNIT_TYPEID unit_type){
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

//returns random unit of given type
const Unit *GooseBot::FindUnit(UNIT_TYPEID unit_type){
    auto all_of_type = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
    if (all_of_type.size() != 0){
        return GetRandomEntry(all_of_type);
    }else{
        return nullptr;
    }
}

// Assumes up-to-date num_bases
void GooseBot::SetDroneCap(){
    if (drone_cap < 14*num_bases){
        drone_cap += 3;
    }
}

// Assumes up-to-date num_bases
void GooseBot::SetQueenCap(){
    if (CountUnitType(UNIT_TYPEID::ZERG_SPAWNINGPOOL) > 0
        && FindUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)->build_progress == 1){
        queen_cap = 2*num_bases;
    }
    else{
        queen_cap = 0;
    }
}

// Assumes up-to-date num_bases
void GooseBot::SetOverlordCap(){
    const ObservationInterface* observation = Observation();
    if (observation->GetFoodUsed() >= observation->GetFoodCap() - 1){
        ++overlord_cap;
    }
}

//TODO, finish making this actually do its job 
void GooseBot::Prioritize(){
    if (build_phase <= army_phase){
        SetSavingsFalse();
        saving_for_building = true;
    }
    else if (build_phase > army_phase){
        SetSavingsFalse();
        saving_for_army = true;
    }
}

void GooseBot::SetSavingsFalse(){
    saving_for_army = false;
    saving_for_building = false;
    saving_for_research = false;
}


