#include "GooseBot.h"

bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, Tag location_tag, UNIT_TYPEID unit_type) {
	//get an observation of the current game state
	const ObservationInterface* observation = Observation(); 
	
	//Get a list of all workers belonging to the bot
	Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
	
	const Unit* target = observation->GetUnit(location_tag);

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
	std::cout << "wtf" << std::endl;
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
			std::cout << "Placement failed" << std::endl;
		}
	}

	if (closest_pos == base_location) {
		std::cout << "closest_pos is base_location" << std::endl;

		return false;
	}

	//Get a list of all workers belonging to the bot
	Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));

	//if we have no workers, we cannot build so we return false
	if (workers.empty()) {
		std::cout << "no workers" << std::endl;
		return false;
	}

	const Unit* unit = GetRandomEntry(workers);
	const AbilityID abil = ABILITY_ID::BUILD_SPAWNINGPOOL;
	const Point2D pos = closest_pos;
	if (Query()->Placement(abil, pos,unit)) { 
		Actions()->UnitCommand(unit, abil, pos);
		
		return true;

	}

	std::cout << "Placement failed" << std::endl;
	return false;
}
