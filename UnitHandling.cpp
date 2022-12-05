#include "GooseBot.h"
/*******************
 * This file contains functions for unit orders not given to idle units or attack units
********************/

// Distribute workers over vespene geysers for harvesting
bool GooseBot::TryHarvestVespene() {
    Units workers = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));
    
    if (workers.empty()) {
        return false;
    }

    const Unit* unit = GetRandomEntry(workers);
    const Unit* vespene_target = FindNearestAllied(UNIT_TYPEID::ZERG_EXTRACTOR, unit->pos);

    if (!vespene_target){
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
    if (   (!CanAfford(UNIT_TYPEID::ZERG_QUEEN))
        || (actionPending(ABILITY_ID::TRAIN_QUEEN))
        || (CountUnitType(UNIT_TYPEID::ZERG_QUEEN) >= num_bases)  ){
        return false;
    }
    Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(baseTypes));
    
    // if valid base and not a queen already closeby
    for (auto base : bases){
        if (base != nullptr 
            && (Distance2D(FindNearestAllied(UNIT_TYPEID::ZERG_QUEEN, base->pos)->pos, base->pos) < 10.0f)){
            Actions()->UnitCommand(base, ABILITY_ID::TRAIN_QUEEN);
            return true;
        }else{
            return false;
        }
    }
    return false;
}

// // Distribute workers over minerals for harvesting
// // Would probably work if better expansion locations, but not currently working, so commented out

// bool GooseBot::TryDistributeMineralWorkers() {
//     Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(baseTypes));
//     if (bases.empty() || num_bases == 1) {
//         return false;
//     }
//     std::cout << "Trying to distribute workers ... ";
//     Units available_workers;
//     int surplus = 0;
//     int total_needed = 0;
//     for (auto base : bases){
//         if (base->build_progress != 1){
//             std::cout << "base not finished" << std::endl;
//             return false;
//         }
//         int workers_needed = base->ideal_harvesters - base->assigned_harvesters;
//         total_needed += workers_needed;
//         if (workers_needed < 0){
//             surplus += (-workers_needed);
//             for (int i = 0; i < surplus; ++i){
//                 const Unit* available = FindNearestAllied(drone, base->pos);
//                 if (available != nullptr){
//                     available_workers.push_back(available);
//                 }
//             }
//         }
//     }
//     if (total_needed <= 0){
//         std::cout << "None needed" << std::endl;
//         return false;
//     }
//     for (auto base : bases){
//         int workers_needed = base->ideal_harvesters - base->assigned_harvesters;
//         std::cout << "workers needed: " << workers_needed << "... ";
//         if (workers_needed > 0 && (!available_workers.empty())){
//             auto a = available_workers.begin();
//             const Unit* mineral_target = FindNearestMineralPatch(base->pos);
//             if (Units(a, available_workers.end()).size() >= workers_needed){
//                 Actions()->UnitCommand(Units(a, a + workers_needed), ABILITY_ID::SMART, mineral_target);
//                 a += workers_needed;
//                 std::cout << "Workers distributing [mid vector]" << std::endl;
//                 return true;
//             }else{
//                 Actions()->UnitCommand(Units(a, available_workers.end()), ABILITY_ID::SMART, mineral_target);
//                 std::cout << "Workers distributing [end vector]" << std::endl;
//                 return true;
//             }
//         }
//     }
//     std::cout << "failed" << std::endl;
//     return false;   
// }