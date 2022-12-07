#include "GooseBot.h"
//DEV BRANCH

void GooseBot::OnGameStart()
{ 
    const_cast<std::vector<Point2D> &>(possibleBaseGrounds) = FindBaseBuildingGrounds();
    enemyStartLocations = Observation()->GetGameInfo().enemy_start_locations;
    const ObservationInterface* observation = Observation();

    //reserve space in vectors
    army.reserve(100);
    melee.reserve(50);
    enemy_base.reserve(5);
    return; 
}


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


void GooseBot::OnStep() {
    TryBuildHatchery();   // Uncomment for "All your base are belong to us *honk*"

    // Make sure pendingOrders are current
    const ObservationInterface * obs = Observation();
    VerifyPending(obs);
    HandleBases(obs);
    //Prioritize();

    if (TryHarvestVespene()) {
        return;
    }
    // if (TryDistributeMineralWorkers()){
    //     return;
    // }
    if (ResearchPhase()){
        std::cout << "Research Phase" << std::endl;
        return;
    }
    if (BuildPhase()){
        std::cout << "Build Phase " << build_phase << std::endl;
        return;        
    }
    if (ArmyPhase()){ 
        std::cout << "Army Phase " << std::endl;
        return;
    }
    std::cout << "OnStep returned empty-handed" << std::endl;

    
}


// In your bot class.
void GooseBot::OnUnitIdle(const Unit* unit) {
    //get the current game state observation
    const ObservationInterface* observation = Observation();
    size_t drone_count = CountUnitType(drone);
    size_t zergl_count = CountUnitType(zergl);
    size_t banel_count = CountUnitType(banel);
    size_t roach_count = CountUnitType(roach);
    size_t mutal_count = CountUnitType(mutal);

    //for our unit pointer, we do a switch-case dependent on it's type
    switch (unit->unit_type.ToType())
    {
        //case UNIT_TYPEID::ZERG_LARVA:
    case larva:
    {
        //while our supply limit is less than or equal to our supply limit cap - 1      Note: changed to if because i can't see why we need a while in a callback, also, was probably causing unexpected behavior with the breaks. change this back if it was actually needed.
        if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
        {   //if our optimal workers at the nearest base is too low
            const Unit *base = FindNearestAllied(baseTypes, unit->pos);
            if (base)
            {
                if ((base->ideal_harvesters <= base->assigned_harvesters - 2))     
                {   //build a worker
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
                    break;
                }
            }
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
                //if our zergling count is less than or equal to 10
            if (zergl_count < zergl_cap)
            {   //try to train a zergling (this can't be done unless there is an existing spawning pool
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
                break;
            }
           
        }else{
            //spawns overlord to increase supply cap when we need supply increase
            VerifyPending(observation);
            if (build_phase > 2 && !(actionPending(ABILITY_ID::TRAIN_OVERLORD)))
            {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
            }
        }
        break;
    }

    case drone:
    {
        const Unit * base = GetNewerBase();
        if (base!=nullptr) {
            const Unit* mineral_target = FindNearestMineralPatch(base->pos);
            if (!mineral_target)
            {
                break;
            }
            Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
        }
       
        break;
    }

    case overl:
    {
        std::_Vector_iterator<std::_Vector_val<std::_Simple_types<std::pair<int, const sc2::Unit *>>>> scoutIt;
        if ((scoutIt = std::find_if(suicideScouts.begin(), suicideScouts.end(), [unit](std::pair<int, const Unit*> scout){ return scout.second == unit; })) != suicideScouts.end())
        {
            scoutPoint(unit, enemyStartLocations[((*scoutIt).first)++ % enemyStartLocations.size()]);
            break;
        } else if (suicideScouts.size() < 0)
        {
            auto scout = std::make_pair(GetRandomInteger(0, enemyStartLocations.size() - 1), unit);
            suicideScouts.push_back(scout);
            scoutPoint(unit, enemyStartLocations[scout.first]);
            break;
        }

        if ((scoutIt = std::find_if(generalScouts.begin(), generalScouts.end(), [unit](std::pair<int, const Unit*> scout){ return scout.second == unit; })) != generalScouts.end())
        {
            scoutPoint(unit, possibleBaseGrounds[((*scoutIt).first)++ % possibleBaseGrounds.size()]);
        } else if (generalScouts.size() < 1)   // FIXME: Making it 10 prolly isn't the best solution to the hovering drones problem, but for now, should be ok.
        {
            auto scout = std::make_pair(0/*GetRandomInteger(0, possibleBaseGrounds.size() - 1)*/, unit);
            generalScouts.push_back(scout);
            scoutPoint(unit, possibleBaseGrounds[scout.first]);
        } else
        {
            Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_PATROL, possibleBaseGrounds[0]);   // TODO: Make non-scouting overlords more useful than this, which just makes them pace back and forth over the base.
        }
        break;
    }

    case queen:
    {
        const Unit* base = FindNearestAllied(baseTypes, unit->pos);
        //iterator pointing to buff if found, end if not found
        if (base!=nullptr)
        {
            auto hasInjection = std::find(base->buffs.begin(), base->buffs.end(), BUFF_ID::QUEENSPAWNLARVATIMER);
            if (hasInjection == base->buffs.end())
            {     //if no injection
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, base);
            }else{
                float rx = GetRandomScalar();
                float ry = GetRandomScalar();
                Point2D pos = Point2D(unit->pos.x + rx * 15.0f, unit->pos.y + ry * 15.0f);
                Actions()->UnitCommand(unit, ABILITY_ID::BUILD_CREEPTUMOR, pos);
            }
        }
        break;
    }

    case zergl:
    {
        if (banel_count < banel_cap) {
            Actions()->UnitCommand(unit, ABILITY_ID::MORPH_BANELING);
        }
    }

    
    case banel:
    case mutal:
    case roach: 
    {
        if (EnemyLocated && enemy_base.empty()) {
            //go over the battlezone!!!

            const Unit* to_murder = FindNearestEnemy(unit->pos);
            if (to_murder != nullptr) {
                Actions()->UnitCommand(melee, ABILITY_ID::ATTACK, to_murder);
            }
        }
        break;
    }
    default:
        break;
    }
}

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
        // case UNIT_TYPEID::ZERG_SPAWNINGPOOL:{
        //     auto ug = std::find(upgraded.begin(), upgraded.end(), UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
        //     if (ug != upgraded.end()){
        //         upgraded.erase(ug);
        //     }
        //     for (auto st : built_structs){
        //         if (st->)
        //     }
        //     auto st = std::find(built_structs.begin(), built_structs.end(), UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
        //     if (st != built_structs.end()){
        //         upgraded.erase(st);
        //     }
        // }
        // case UNIT_TYPEID::ZERG_ROACHWARREN:
        // case UNIT_TYPEID::ZERG_BANELINGNEST:
        // case UNIT_TYPEID::ZERG_SPIRE:
        // case UNIT_TYPEID::ZERG_ULTRALISKCAVERN:


    }
}


void GooseBot::OnUnitEnterVision(const Unit* unit) {
    const ObservationInterface* observation = Observation();
    Point2D last_seen = unit->pos;
    
    
        //uint32_t army_count = observation->GetArmyCount();
        //Unit* army = new Unit[army_count];
        //Units zergls = observation->GetUnits(Unit::Alliance::Self, IsUnit(zergl));
        //Units banels = observation->GetUnits(Unit::Alliance::Self, IsUnit(banel));
        //Units mutals = observation->GetUnits(Unit::Alliance::Self, IsUnit(mutal));
        //Units roaches = observation->GetUnits(Unit::Alliance::Self, IsUnit(roach));
        //Units queens = observation->GetUnits(Unit::Alliance::Self, IsUnit(queens));

        //Units attack_army = getArmy();
        //for (auto& unit : attack_army) {
        //    std::cout << unit->unit_type << std::endl;
        //}

        switch (unit->unit_type.ToType())
        {
        case commc:
        case orbcomm:
        case orbcommf:
        case lair:
        case hatch:
        case hive:
        case pylon:
        case gate:
        case nexus:
        {
            //Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(banels, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(mutals, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(roaches, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(queens, ABILITY_ID::ATTACK_ATTACKTOWARDS, unit);
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
        {
           
            if (melee.size() >= melee_cap) {
                Actions()->UnitCommand(melee, ABILITY_ID::ATTACK, unit);
            }
            break;
        }
        }
    
    //std::cout << "army check fail" << std::endl;
}

void GooseBot::OnBuildingConstructionComplete(const Unit* unit){
    // switch (unit->unit_type.ToType()){
    //     case default:
    //     {
            Units larva_pool = Observation()->GetUnits(Unit::Alliance::Self, IsIdleLarva());
            Actions()->UnitCommand(larva_pool, ABILITY_ID::TRAIN_DRONE);
     //       break;
    //    }
   // }

}

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
            if (available_larva.size() > 0)
            {
                Actions()->UnitCommand(GetRandomEntry(available_larva), ABILITY_ID::TRAIN_DRONE);
            }
        }
    }
    return;
}

void GooseBot::OnUpgradeCompleted(UPGRADE_ID){
    return;
};

