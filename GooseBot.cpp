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
	TryMorphExtractor();
    TryBuildSpawningPool();
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
                if ((observation->GetFoodWorkers() <= 22 - 2)) {        //TODO: change this limit to rely on our number of hatcheries
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

            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
            
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
