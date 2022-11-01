#include "BasicSc2Bot.h"
using namespace sc2;

void BasicSc2Bot::OnGameStart() { return; }

void BasicSc2Bot::OnStep() { 
	TryMorphOverlord();
}

void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
	switch (unit->unit_type.ToType()) {
		case UNIT_TYPEID::ZERG_LARVA: {
			
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
			break;
		}
		case UNIT_TYPEID::ZERG_DRONE: {
			const Unit * mineral_target = FindNearestMineralPatch(unit->pos);
			if (!mineral_target) {
				break;

			}
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
			break;
		}
		default: {
			break;
		}
	}
}


bool BasicSc2Bot::TryMorphStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
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

	float rx = GetRandomScalar();
	float ry = GetRandomScalar();

	Actions()->UnitCommand(unit_to_build, ability_type_for_structure, Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));

	return true;
}

bool BasicSc2Bot::TryMorphOverlord() {
	const ObservationInterface* observation = Observation();

	if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2) {
		return false;
	}

	return TryMorphStructure(ABILITY_ID::TRAIN_OVERLORD);
}

const Unit* BasicSc2Bot::FindNearestMineralPatch(const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const Unit * target = nullptr;
	for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
            float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
				
			}
			
		}
		
	}
	return target;
}