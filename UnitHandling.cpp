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