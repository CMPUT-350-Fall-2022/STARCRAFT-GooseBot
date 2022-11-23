#include "GooseBot.h"

bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, Tag location_tag, UNIT_TYPEID unit_type) {
	//get an observation of the current game state
	const ObservationInterface* observation = Observation(); 
	
	//Get a list of all workers belonging to the bot
	Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
	
	const Unit* target = observation->GetUnit(location_tag);
	if (target->unit_type == UNIT_TYPEID::ZERG_DRONE){
		return false;
	}

	//if we have no workers, we cannot build so we return false
	if (workers.empty()) {
		return false;
	}

	// Check to see if there is already a worker heading out to build it
	for (const auto& worker : workers) {
		for (const auto& order : worker->orders) {
			if (order.ability_id == ability_type_for_structure) {
				return false;
			}
		}
	}

	// If no worker is already building one, get a random worker to build one
	const Unit* unit = GetRandomEntry(workers);
	
	// Check to see if unit can build there
	if (Query()->Placement(ability_type_for_structure, target->pos)) {
		Actions()->UnitCommand(unit, ability_type_for_structure, target);
		return true;
	}
	return false;
}

bool GooseBot::TryMorphExtractor() {
	const ObservationInterface* observation = Observation();
	
	Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
	Units geysers = observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
	if (bases.empty()) {
		return false;
	}
	Point2D base_location = bases.back()->pos;
	//only search within this radius
	float minimum_distance = 30.0f;
	Tag closestGeyser = 0;
	for (const auto& geyser : geysers) {
		float current_distance = Distance2D(base_location, geyser->pos);
		if (current_distance < minimum_distance) {
			if (Query()->Placement(ABILITY_ID::BUILD_EXTRACTOR, geyser->pos)) {
				minimum_distance = current_distance;
				closestGeyser = geyser->tag;
			}
		}
	}

	// In the case where there are no more available geysers nearby
	if (closestGeyser == 0) {

		return false;
	}

	return TryMorphStructure(ABILITY_ID::BUILD_EXTRACTOR, closestGeyser);
}

bool GooseBot::TryBuildSpawningPool() {
	const ObservationInterface* observation = Observation();

	Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
	Units spawn_pools = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL));
	Units geysers = observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));

	Point2D base_location = bases.back()->pos;
	size_t bases_num = bases.size();
	size_t spawn_pools_num = spawn_pools.size();

	float minimum_distance = 30.0f;
	Point2D closest_pos = base_location;
	for (const auto& geyser : geysers) {
		Point2D new_pos = geyser->pos;
		new_pos.x -= 5;
		
		float current_distance = Distance2D(base_location, new_pos);
		if (current_distance < minimum_distance) {
			if (Query()->Placement(ABILITY_ID::BUILD_SPAWNINGPOOL, new_pos)) {
				minimum_distance = current_distance;
				closest_pos = new_pos;
			}
		}
	}

	if (closest_pos == base_location) {
		return false;
	}

	//Get a list of all workers belonging to the bot
	Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));

	//if we have no workers, we cannot build so we return false
	if (workers.empty()) {
		return false;
	}

	const Unit* unit = GetRandomEntry(workers);
	const AbilityID abil = ABILITY_ID::BUILD_SPAWNINGPOOL;
	const Point2D pos = closest_pos;
	if (Query()->Placement(abil, pos,unit)) { 
		Actions()->UnitCommand(unit, abil, pos);
		return true;
	}

	return false;
}

//from tutorial, adapted
bool GooseBot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID unit_type, size_t struct_cap) {
	const ObservationInterface* observation = Observation();
	Units structs = observation->GetUnits(Unit::Alliance::Self, IsUnit(struct_type));
	if (structs.size() >= struct_cap){
		return false;
	}
    // If a unit already is building a supply structure of this type, do nothing.
    // Also get an scv to build the structure.
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
    Actions()->UnitCommand(unit_to_build, ability_type_for_structure,\
          Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
    
    return true;
}
	

bool GooseBot::TryBuildRoachWarren() {
	const ObservationInterface* observation = Observation();

	

}