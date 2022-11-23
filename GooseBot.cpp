#include "GooseBot.h"

using namespace sc2;

void GooseBot::OnGameStart() { return; }


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

    if (TryBuildStructure(ABILITY_ID::BUILD_SPAWNINGPOOL, UNIT_TYPEID::ZERG_SPAWNINGPOOL)) {
        return;
    }
    if (TryBirthQueen()){
        return;
    }
    if (TryMorphExtractor()) {
        return;
    }
    if (TryHarvestVespene()) {
        return;
    }


    
}


// In your bot class.
void GooseBot::OnUnitIdle(const Unit* unit) {
    //get the current game state observation
    const ObservationInterface* observation = Observation();

    //for our unit pointer, we do a switch-case dependent on it's type
	switch (unit->unit_type.ToType())
    {
        //if the unit is a larva unit
		case UNIT_TYPEID::ZERG_LARVA:
        {
            //while our supply limit is less than or equal to our supply limit cap - 1
			while (observation->GetFoodUsed() <= observation->GetFoodCap() - 1)
            {
                //if our total number of workers is less than 30
                if ((observation->GetFoodWorkers() <= 16 - 2)) {        //TODO: change this limit to rely on our number of hatcheries
                    //build a worker
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
				break;
			}

            //if our army count is less than or equal to 10
                if (observation->GetFoodArmy() <= 10) {
                    //try to train a zergling (this can't be done unless there is an existing spawning pool
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
                    break;
                }
			}
            
            //spawns overlord to increase supply cap when we have only one available opening, 
            //if we have less than 4 overlords FOR NOW
			if (countUnitType(UNIT_TYPEID::ZERG_OVERLORD) < 4){
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
            }
            
		}

		case UNIT_TYPEID::ZERG_DRONE:
        {

			const Unit * mineral_target = FindNearestMineralPatch(unit->pos);
			if (!mineral_target)
            {
				break;
			}
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
			break;
		}

        case UNIT_TYPEID::ZERG_OVERLORD:
        {
            scout(unit);
            break;
        }

        case UNIT_TYPEID::ZERG_QUEEN:
        {
            Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, FindNearest(UNIT_TYPEID::ZERG_HATCHERY, unit->pos));
            break;
        }
        
		default:
			break;
	}
}


// Very simple for now.
void GooseBot::scout(const Unit* unit)
{
    const GameInfo& game_info = Observation()->GetGameInfo();
    auto enemyStartLocations = game_info.enemy_start_locations;
    Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_PATROL, GetRandomEntry(enemyStartLocations));
}

const Unit* GooseBot::FindNearest(UNIT_TYPEID target_unit, const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;

    for (const auto& u : units)
    {

        if (u->unit_type == target_unit)
        {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance)
            {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}



const Unit* GooseBot::FindNearestMineralPatch(const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const Unit * target = nullptr;
	for (const auto& u : units)
    {
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD)
        {
            float d = DistanceSquared2D(u->pos, start);
			if (d < distance)
            {
				distance = d;
				target = u;
			}			
		}		
	}
	return target;
}


size_t GooseBot::countUnitType(UNIT_TYPEID unit_type)
{
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

bool GooseBot::TryHarvestVespene() {
    Units workers = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));
    
    if (workers.empty()) {
        return false;
    }


    const Unit* unit = GetRandomEntry(workers);
    const Unit* vespene_target = FindNearest(UNIT_TYPEID::ZERG_EXTRACTOR, unit->pos);

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

bool GooseBot::TryBirthQueen(){
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
    if (countUnitType(UNIT_TYPEID::ZERG_QUEEN) == 0 || countUnitType(UNIT_TYPEID::ZERG_QUEEN) < queensCap){
        Actions()->UnitCommand(bases[0], ABILITY_ID::TRAIN_QUEEN);
        return true;
    }else{
        return false;
    }
}
// EFFECT_INJECTLARVA target hatchery/lair
// MORPH_LAIR no target

// MORPH_OVERLORDTRANSPORT no target
/*
              RALLY_HATCHERY_UNITS = 211,   // Target: Unit, Point.
  539         RALLY_HATCHERY_WORKERS = 212,   // Target: Unit, Point.
  540         RALLY_MORPHING_UNIT = 199,   // Target: Unit, Point.
  541         RALLY_NEXUS = 207,   // Target: Unit, Point.
  542         RALLY_UNITS = 3673,  // Target: Unit, Point.
  543         RALLY_WORKERS = 3690,  // Target: Unit, Point.
  
  625         RESEARCH_ZERGFLYERARMOR = 3702,  // Target: None.
  626         RESEARCH_ZERGFLYERARMORLEVEL1 = 1315,  // Target: None.
  627         RESEARCH_ZERGFLYERARMORLEVEL2 = 1316,  // Target: None.
  628         RESEARCH_ZERGFLYERARMORLEVEL3 = 1317,  // Target: None.
  629         RESEARCH_ZERGFLYERATTACK = 3703,  // Target: None.
  630         RESEARCH_ZERGFLYERATTACKLEVEL1 = 1312,  // Target: None.
  631         RESEARCH_ZERGFLYERATTACKLEVEL2 = 1313,  // Target: None.
  632         RESEARCH_ZERGFLYERATTACKLEVEL3 = 1314,  // Target: None.
  633         RESEARCH_ZERGGROUNDARMOR = 3704,  // Target: None.
  634         RESEARCH_ZERGGROUNDARMORLEVEL1 = 1189,  // Target: None.
  635         RESEARCH_ZERGGROUNDARMORLEVEL2 = 1190,  // Target: None.
  636         RESEARCH_ZERGGROUNDARMORLEVEL3 = 1191,  // Target: None.
  637         RESEARCH_ZERGLINGADRENALGLANDS = 1252,  // Target: None.
  638         RESEARCH_ZERGLINGMETABOLICBOOST = 1253,  // Target: None.
  639         RESEARCH_ZERGMELEEWEAPONS = 3705,  // Target: None.
  640         RESEARCH_ZERGMELEEWEAPONSLEVEL1 = 1186,  // Target: None.
  641         RESEARCH_ZERGMELEEWEAPONSLEVEL2 = 1187,  // Target: None.
  642         RESEARCH_ZERGMELEEWEAPONSLEVEL3 = 1188,  // Target: None.
  643         RESEARCH_ZERGMISSILEWEAPONS = 3706,  // Target: None.
  644         RESEARCH_ZERGMISSILEWEAPONSLEVEL1 = 1192,  // Target: None.
  645         RESEARCH_ZERGMISSILEWEAPONSLEVEL2 = 1193,  // Target: None.
  646         RESEARCH_ZERGMISSILEWEAPONSLEVEL3 = 1194,  // Target: None.*/