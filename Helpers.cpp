#include "GooseBot.h"

// Assumes up-to-date num_bases
bool GooseBot::BuildPhase(){
    build_phase = 0;

    if (CountUnitType(UNIT_TYPEID::ZERG_SPAWNINGPOOL) == 0){
        return TryBuildStructure(ABILITY_ID::BUILD_SPAWNINGPOOL, UNIT_TYPEID::ZERG_SPAWNINGPOOL);
    }else{++build_phase;}
    if (num_bases < 2){
        return TryBuildHatchery();
    }else{++build_phase;}
    if (CountUnitType(UNIT_TYPEID::ZERG_ROACHWARREN) == 0){
        return TryBuildStructure(ABILITY_ID::BUILD_ROACHWARREN, UNIT_TYPEID::ZERG_ROACHWARREN);
    }else{++build_phase;}
    if (CountUnitType(UNIT_TYPEID::ZERG_BANELINGNEST) == 0){
        return TryBuildStructure(ABILITY_ID::BUILD_BANELINGNEST, UNIT_TYPEID::ZERG_BANELINGNEST);
    }else{++build_phase;}
    if (CountUnitType(UNIT_TYPEID::ZERG_LAIR) == 0){
        return TryMorphLair();
    }else{++build_phase;}
    if (CountUnitType(UNIT_TYPEID::ZERG_SPIRE) == 0){
        return TryBuildStructure(ABILITY_ID::BUILD_SPIRE, UNIT_TYPEID::ZERG_SPIRE);
    }else{++build_phase;}
    if (num_bases < 3){
        return TryBuildHatchery();
    }else{++build_phase;}
    if (CountUnitType(UNIT_TYPEID::ZERG_INFESTATIONPIT) == 0){
        return TryBuildStructure(ABILITY_ID::BUILD_INFESTATIONPIT, UNIT_TYPEID::ZERG_INFESTATIONPIT);
    }else{++build_phase;}
    if (CountUnitType(UNIT_TYPEID::ZERG_HIVE) == 0){
        return TryMorphHive();
    }else{++build_phase;}
    if (CountUnitType(UNIT_TYPEID::ZERG_ULTRALISKCAVERN) == 0){
        return TryBuildStructure(ABILITY_ID::BUILD_ULTRALISKCAVERN, UNIT_TYPEID::ZERG_ULTRALISKCAVERN);
    }else{return false;}
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
            }else{
                return false;
            }
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
    drone_cap = 0;
    Units extractors = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_EXTRACTOR));
    for (auto e : extractors){
        if (e->vespene_contents > 0){
            drone_cap += 3;
        }
    }
    drone_cap += (14*num_bases);
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

//TODO, make actually do its job 
void GooseBot::Prioritize(){

    saving_for_army = false;
    saving_for_building = true;
    
}


