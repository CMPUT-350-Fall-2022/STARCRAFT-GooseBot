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

    if (vespene_target->assigned_harvesters >= vespene_target->ideal_harvesters) {
        return false;
    }
    
    Actions()->UnitCommand(unit, ABILITY_ID::SMART, vespene_target);
    return true;
      
}

// Distribute workers over vespene geysers for harvesting
bool GooseBot::TryDistributeMineralWorkers() {
    Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(baseTypes));

    if (bases.empty() || num_bases == 1) {
        return false;
    }
    std::cout << "Trying to distribute workers" << std::endl;
    Units available_workers;
    size_t surplus = 0;
    for (auto base : bases){
        size_t workers_needed = base->ideal_harvesters - base->assigned_harvesters;
        if (workers_needed < 0){
            surplus += (-workers_needed);
            for (size_t i = 0; i < surplus; ++i){
                const Unit* available = FindNearestAllied(drone, base->pos);
                if (available != nullptr){
                    available_workers.push_back(available);
                }
            }
        }else if (workers_needed > 0 && (!available_workers.empty())){
            Actions()->UnitCommand(available_workers, ABILITY_ID::SMART, base);
            return true;
        }
    }
    return false;   
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