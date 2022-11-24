#include "GooseBot.h"

bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, Tag location_tag, UNIT_TYPEID worker_unit) {
	const ObservationInterface* observation = Observation(); 
	//Get a random worker
	const Unit * worker = FindUnit(worker_unit);
	//if we have no workers, return false
	if (worker == nullptr) {
		std::cout << "fails available worker or cost checks ,";
		return false;
	}
	// Get target to morph; note that target and worker can be the same, 
	// but should not be the same unit type with different tags, as this would
	// mean that a unit would try to do a self targeting action on another unit
	const Unit* target = observation->GetUnit(location_tag);
	if ((worker->tag != location_tag) && (target->unit_type == worker_unit)){
		Actions()->UnitCommand(target, ability_type_for_structure);
		std::cout << "target self targeted";
		return true;
	}else if (worker->tag == location_tag){		//self-targeting
		Actions()->UnitCommand(worker, ability_type_for_structure);
		std::cout << "worker self targeted";
		return true;
	}// Otherwise, builder unit different than target unit
	
	// Check to see if worker can morph at target location
	if (Query()->Placement(ability_type_for_structure, target->pos)) {
		Actions()->UnitCommand(worker, ability_type_for_structure, target);
		std::cout << "worker targeted non-worker";
		return true;
	}
	std::cout << "failing to catch success condition ,";
	return false;
}

// Try to morph a drone into an extractor on a nearby vespene geyser
bool GooseBot::TryMorphExtractor() {
	const ObservationInterface* observation = Observation();
	
	Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
	Units geysers = observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
	if (bases.empty() 
		|| (countUnitType(UNIT_TYPEID::ZERG_EXTRACTOR) >= 2*bases.size())
		|| (actionPending(ABILITY_ID::BUILD_EXTRACTOR))
		|| (!CanAfford(UNIT_TYPEID::ZERG_EXTRACTOR))) {
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

// from tutorial, adapted
// Try to build a structure with given ability, struct type, 
// & optional worker type[drone default], max num of structs desired[1 default] 
bool GooseBot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID unit_type, size_t struct_cap) {
	// If a unit already is building a supply structure of this type, do nothing.
	if (actionPending(ability_type_for_structure) || (countUnitType(struct_type) >= struct_cap) || !CanAfford(struct_type)){
		return false;
	}
	// If we have the number of these we want already, do nothing
	const ObservationInterface* observation = Observation();
	Units structs = observation->GetUnits(Unit::Alliance::Self, IsUnit(struct_type));
	if (structs.size() >= struct_cap){
		return false;
	}
	// Otherwise, build
    // Get a unit to build the structure.
    const Unit* unit_to_build = FindUnit(unit_type);
	// Try to build at a position in a random direction from builder
    float rx = GetRandomScalar();
    float ry = GetRandomScalar();
    Actions()->UnitCommand(unit_to_build, ability_type_for_structure,\
          Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
    return true;
}

//try to morph hatchery into lair
bool GooseBot::TryMorphLair() {
	const ObservationInterface* observation = Observation();
	
	Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
	if (bases.empty() || !CanAfford(UNIT_TYPEID::ZERG_LAIR) || actionPending(ABILITY_ID::MORPH_LAIR)) {
		return false;
	}
	const Unit* base = bases.back();
	Actions()->UnitCommand(base, ABILITY_ID::MORPH_LAIR);
	return true;	
}

//return true if the action is pending, false otherwise
bool GooseBot::actionPending(ABILITY_ID action){
	return (std::find(pendingOrders.begin(), pendingOrders.end(), action) != pendingOrders.end());
}

//get the pending orders for the game step
void GooseBot::VerifyPending(){
	pendingOrders.clear();
	const ObservationInterface* observation = Observation();
	Units workers = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& worker : workers) {
		for (const auto& order : worker->orders) {
			pendingOrders.insert(order.ability_id);
		}
	}
}

