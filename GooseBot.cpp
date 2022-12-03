#include "GooseBot.h"

void GooseBot::OnGameStart()
{ 
    possibleBaseGrounds = FindBaseBuildingGrounds();
    enemyStartLocations = Observation()->GetGameInfo().enemy_start_locations;
    const ObservationInterface* observation = Observation();
    army.reserve(100);
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
    // Make sure pendingOrders are current
    VerifyPending();
    HandleBases();
    //Prioritize();

    if (TryHarvestVespene()) {
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
    if (ResearchPhase()){
        std::cout << "Research Phase " << std::endl;
        return;
    } 
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
        if (observation->GetFoodUsed() <= observation->GetFoodCap() - 1)
        {   //if our total number of workers is less than 30
            if ((drone_count < drone_cap))     
            {   //build a worker
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
                break;
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
            VerifyPending();
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
                TryBuildStructure(ABILITY_ID::BUILD_CREEPTUMOR, UNIT_TYPEID::ZERG_CREEPTUMOR, unit->unit_type, 10);
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

    //I dont think we need this now that we have VerifyArmy
    // case banel:
    // case mutal:
    // case roach: 
    // {
    //     army.push_back(unit);
    //     break;
    // }
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
        case hatch:
        case nexus:
        {
            //Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(banels, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(mutals, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(roaches, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(queens, ABILITY_ID::ATTACK_ATTACKTOWARDS, unit);

            enemy_base = unit->pos;
            EnemyLocated = true;
            if (ArmyReady()) {
                //Actions()->UnitCommand(army, ABILITY_ID::ATTACK, unit);
            }
            break;
        }
        default:
        {
            //Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK, unit);
            //Actions()->UnitCommand(army, ABILITY_ID::ATTACK, unit);
            //std::cout << "enemy unit check fail" << std::endl;

            break;
        }
        }
    
    //std::cout << "army check fail" << std::endl;
}