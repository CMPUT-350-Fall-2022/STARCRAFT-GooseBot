#include "GooseBot.h"

// Distribute workers over vespene geysers for harvesting
bool GooseBot::TryHarvestVespene() {
    Units workers = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));
    
    if (workers.empty()) {
        return false;
    }

    const Unit* unit = GetRandomEntry(workers);
    const Unit* vespene_target = FindNearestAllied(UNIT_TYPEID::ZERG_EXTRACTOR, unit->pos);

    if (!vespene_target)
    {
        return false;
    }
    if (vespene_target->build_progress != 1) {
        return false;
    }

    if (vespene_target->assigned_harvesters>=vespene_target->ideal_harvesters) {//change this to not be hardcoded
        return false;
    }
    
    Actions()->UnitCommand(unit, ABILITY_ID::SMART, vespene_target);
    return true;
      
}

// Distribute workers over vespene geysers for harvesting
bool GooseBot::TryDistributeMineralWorkers() {
    std::cout << "Trying to distribute workers" << std::endl;
    Units workers = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));
    Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(baseTypes));

    if (workers.empty()) {
        return false;
    }
    size_t w = workers.size();
    size_t b = bases.size();

    size_t workersPerBase = w/b;

    auto it = workers.begin();
    for (auto base : bases){
        if (it + workersPerBase <= workers.end()){
            Actions()->UnitCommand(Units(it, it + workersPerBase), ABILITY_ID::GENERAL_MOVE, base->pos);
            it += workersPerBase;
        }else{
            Actions()->UnitCommand(Units(it, workers.end()), ABILITY_ID::GENERAL_MOVE, base->pos);
        }
    }   
}

// Try to birth a queen unit from a base
bool GooseBot::TryBirthQueen(){
    if ( (!CanAfford(UNIT_TYPEID::ZERG_QUEEN))
        || (actionPending(ABILITY_ID::TRAIN_QUEEN))
        || (CountUnitType(UNIT_TYPEID::ZERG_QUEEN) >= queen_cap)  ){
        return false;
    }
    const Unit * base = GetNewerBase();
    if (base != nullptr){
        Actions()->UnitCommand(base, ABILITY_ID::TRAIN_QUEEN);
        return true;
    }else{
        return false;
    }
    return false;
}