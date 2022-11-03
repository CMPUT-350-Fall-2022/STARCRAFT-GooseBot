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
	TryMorphOverlord();
}


// In your bot class.
void GooseBot::OnUnitIdle(const Unit* unit) {
    const ObservationInterface* observation = Observation();

	switch (unit->unit_type.ToType())
    {
		case UNIT_TYPEID::ZERG_LARVA:
        {
			while (observation->GetFoodUsed() <= observation->GetFoodCap() - 1)
            {
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
				break;
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


bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
	const ObservationInterface* observation = Observation();

	const Unit* unit_to_build = nullptr;
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ability_type_for_structure) {
				return false;
			}
		}

		if (unit->unit_type == unit_type) {
			unit_to_build = unit;
		}
	}

	if (unit_to_build)
    {
        float rx = GetRandomScalar();
        float ry = GetRandomScalar();
        Actions()->UnitCommand(unit_to_build,
                               ability_type_for_structure,
                               Point2D(unit_to_build->pos.x + rx * 15.0f,
                                       unit_to_build->pos.y + ry * 15.0f));
        return true;
    }
    return false;
}


bool GooseBot::TryMorphOverlord() {
	const ObservationInterface* observation = Observation();

	if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2) {
		return false;
	}

	return TryMorphStructure(ABILITY_ID::TRAIN_OVERLORD);
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
