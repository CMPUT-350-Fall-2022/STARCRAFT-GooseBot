#include "GooseBot.h"
/*******************
 * This file contains functions called automatically by the game engine
********************/

// Called when game starts
// Checks for potential bases & reserves space in vectors
void GooseBot::OnGameStart()
{ 
    possibleBaseGrounds = FindBaseBuildingGrounds();
    enemyStartLocations = Observation()->GetGameInfo().enemy_start_locations;

    //reserve space in vectors
    army.reserve(100);
    melee.reserve(50);
    enemy_base.reserve(5);
    return; 
}

// Called when game ends
// Shows game information for win/loss
void GooseBot::OnGameEnd()
{
    const ObservationInterface* observation = Observation();
    GameInfo gameInfo = observation->GetGameInfo();
    auto players = std::map<uint32_t, PlayerInfo*>();
    for (auto &playerInfo : gameInfo.player_info)
    {

        players[playerInfo.player_id] = &playerInfo;
    }
    
    auto playerTypes = std::map<PlayerType, std::string>();
    playerTypes[Participant] = "Goose";
    playerTypes[Computer] = "Prey";
    playerTypes[Observer] = "Observer";

    auto gameResults = std::map<GameResult, std::string>();
    gameResults[Win] = " Wins";
    gameResults[Loss] = " Loses";
    gameResults[Tie] = " Tied";
    gameResults[Undecided] = " Undecided";

    for (auto &playerResult : observation->GetResults())
    {
        std::cout << playerTypes[((*(players[playerResult.player_id])).player_type)] 
                  << gameResults[playerResult.result]
                  << std::endl;
    }
}

// Called each game step
// Verifies items we are tracking, returns if one of attempted actions are successful
void GooseBot::OnStep() {
    // Make sure pendingOrders & num_bases are current
    VerifyPending();
    CountBases();
    
    if (TryResearch(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::RESEARCH_PNEUMATIZEDCARAPACE, UPGRADE_ID::OVERLORDSPEED)){
        return;
    }// Maximize Vespene workers
    if (TryHarvestVespene()) {
        return;
    }// Attempt next research task
    if (ResearchPhase()){
        std::cout << "Research Phase " << std::endl;
        return;
    }// Handle build tasks
    if (BuildPhase()){
        std::cout << "Build Phase " << build_phase << std::endl;
        return;        
    }// Handle army tasks
    if (ArmyPhase()){ 
        std::cout << "Army Phase " << std::endl;
        return;
    }
    
}


// Calls ONCE when a unit becomes idle (if order is not given, will not trigger again for same unit)
// Gives units different default commands based on types
void GooseBot::OnUnitIdle(const Unit* unit) {
    const ObservationInterface* observation = Observation();
    // Different default command dependent on unit type
    switch (unit->unit_type.ToType()){
        case larva:
        {// If our supply limit is less than or equal to our supply limit cap - 2 (which is the supply needed for our large units)
            if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
            {   // If the worker cap is less than ideal @ the nearest base
                // and have spawning pool
                VerifyArmyCounts();
                const Unit* base = FindNearestAllied(baseTypes, unit->pos);
                if ((build_phase > 2 ) && (base->ideal_harvesters - 2 > base->assigned_harvesters))     
                {   //build a worker
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
                    break;
                }
                // If we want more of a given army unit
                if (mutal_count < mutal_cap)
                {
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MUTALISK);
                    break;
                }
                if (roach_count < roach_cap)
                {
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ROACH);
                    break;
                }
                if (zergl_count < zergl_cap)
                {
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
                    break;
                }
            // Spawns overlord to increase supply cap when we need supply increase
            }else {
                VerifyPending();    //tries to fix issue where too many overlords spawn at once
                // but not before spawning pool
                if (build_phase > 2 && !(actionPending(ABILITY_ID::TRAIN_OVERLORD))){
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
                }else{
                    idle_larvae.push_back(unit);
                }
            }
            break;
        }
        // Idle worker mines nearest mineral field
        case drone:
        {
            const Unit * base = FindNearestAllied(baseTypes, unit->pos);
            if (base != nullptr) {
                const Unit* mineral_target = FindNearestMineralPatch(base->pos);
                if (!mineral_target){
                    break;
                }
                Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
            }
            break;
        }

        // Idle overlord scouts
        case overl:
        {
            std::_Vector_iterator<std::_Vector_val<std::_Simple_types<std::pair<int, const sc2::Unit *>>>> scoutIt;
            if ((scoutIt = std::find_if(suicideScouts.begin(), suicideScouts.end(), [unit](std::pair<int, const Unit*> scout){ return scout.second == unit; })) != suicideScouts.end())
            {
                scoutPoint(unit, enemyStartLocations[((*scoutIt).first)++ % enemyStartLocations.size()]);
                break;
            } else if (suicideScouts.size() < 2)
            {
                auto scout = std::make_pair(GetRandomInteger(0, enemyStartLocations.size() - 1), unit);
                suicideScouts.push_back(scout);
                scoutPoint(unit, enemyStartLocations[scout.first]);
                break;
            }

            if ((scoutIt = std::find_if(generalScouts.begin(), generalScouts.end(), [unit](std::pair<int, const Unit*> scout){ return scout.second == unit; })) != generalScouts.end())
            {
                scoutPoint(unit, possibleBaseGrounds[((*scoutIt).first)++ % possibleBaseGrounds.size()]);
            } else if (generalScouts.size() < 4)
            {
                auto scout = std::make_pair(GetRandomInteger(0, possibleBaseGrounds.size() - 1), unit);
                generalScouts.push_back(scout);
                scoutPoint(unit, possibleBaseGrounds[scout.first]);
            } else
            {
                Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_PATROL, possibleBaseGrounds[0]);   // TODO: Make non-scouting overlords more useful than this, which just makes them pace back and forth over the base.
            }
            break;
        }
        // Idle queens inject larva if able
        // Otherwise tries to build creep tumour nearby 
        // (but cant, no energy; still, allows for re-triggering of larva injection)
        case queen:
        {
            const Unit* base = FindNearestAllied(baseTypes, unit->pos);
            if (base!=nullptr){
                // iterator pointing to larva buff if found, end if not found
                auto hasInjection = std::find(base->buffs.begin(), base->buffs.end(), BUFF_ID::QUEENSPAWNLARVATIMER);
                if (hasInjection == base->buffs.end())
                {     //if no injection
                    Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, base);
                }else{
                    // Try to build creep tumor at random nearby point
                    Point2D pos = Point2D(unit->pos.x + GetRandomScalar() * 15.0f, unit->pos.y + GetRandomScalar() * 15.0f);
                    Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR, pos);
                }
            }
            break;
        }
        // In other case, do nothing
        default:
        {
            break;
        }
    }
}

// Called when a unit is destroyed
// updates scout vector if scout dies
void GooseBot::OnUnitDestroyed(const Unit* unit)
{
    switch (unit->unit_type.ToType())
    {
        case overl:
        {
            std::_Vector_iterator<std::_Vector_val<std::_Simple_types<std::pair<int, const sc2::Unit *>>>> scoutIt;
            if ((scoutIt = std::find_if(generalScouts.begin(), generalScouts.end(), [unit](std::pair<int, const Unit*> scout){ return scout.second == unit; })) != generalScouts.end())
            {
                generalScouts.erase(scoutIt);
            }
        }
    }
}

// Called when a unit is seen by one of our units
// Stores when enemy town centre located, and
// sends melee units to attack spotted enemies
void GooseBot::OnUnitEnterVision(const Unit* unit) {
    const ObservationInterface* observation = Observation();
    Point2D last_seen = unit->pos;
    switch (unit->unit_type.ToType())
    {   // Store town hall, set enemy located to true
        case commc:
        case hatch:
        case nexus:
        {
            if (enemy_base.empty()) {
                enemy_base.push_back(unit);
                EnemyLocated = true;
            }
            else {
                if (std::find(enemy_base.begin(), enemy_base.end(), unit) != enemy_base.end()) {
                    break;
                }
                else {
                    enemy_base.push_back(unit);
                    break;
                }
            }
            break;
        }
        default:
        {   // Send melee units to attack spotted units
            if (melee.size() >= melee_cap) {
                Actions()->UnitCommand(melee, ABILITY_ID::ATTACK, unit);
            }
            break;
        }
    }

}

// Called when the construction of a building completes
// Tries to help with idle larva issue by training zerglings
// Tries to research -- doesn't work though
void GooseBot::OnBuildingConstructionComplete(const Unit* unit){
    switch (unit->unit_type.ToType()){
        case UNIT_TYPEID::ZERG_SPAWNINGPOOL:{
            Units larva_pool = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(larva));
            Actions()->UnitCommand(larva_pool, ABILITY_ID::TRAIN_ZERGLING);
            TryResearch(UNIT_TYPEID::ZERG_SPAWNINGPOOL, ABILITY_ID::RESEARCH_ZERGLINGMETABOLICBOOST, UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
            break;
        }
    }

}

// Calls when a new unit is created (not finished building)
// Tries to help worker shortage issue by training a drone when a drone is sacrificed to build a structure
void GooseBot::OnUnitCreated(const Unit* unit){
    switch (unit->unit_type.ToType()){
        case UNIT_TYPEID::ZERG_SPAWNINGPOOL:
        case UNIT_TYPEID::ZERG_EXTRACTOR:
        case UNIT_TYPEID::ZERG_ROACHWARREN:
        case UNIT_TYPEID::ZERG_BANELINGNEST:
        case UNIT_TYPEID::ZERG_HATCHERY:
        case UNIT_TYPEID::ZERG_INFESTATIONPIT:
        case UNIT_TYPEID::ZERG_ULTRALISKCAVERN:
        {
            Units available_larva = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(larva));
            Actions()->UnitCommand(GetRandomEntry(available_larva), ABILITY_ID::TRAIN_DRONE);
        }
    }
    return;
}

// Called when a research task is complete
// Empty for now
void GooseBot::OnUpgradeCompleted(UPGRADE_ID){
    return;
};

