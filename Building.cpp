#include "GooseBot.h"
/// <summary>
/// Checks if there structur can be morphed, if true morphs/builds structure
/// Requires preexisting unit to morph from
/// </summary>
/// <param name="ability_type_for_structure"></param>
/// <param name="location_tag"></param>
/// <param name="unit_type"></param>
/// <returns>BOOL, true if structure can be morphed from unit, false otherwise</returns>
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

/// <summary>
/// Checks if there is a Vespene Geyser nearby the hatchery, if true passes parameters to build extractor to TryMorphStructure()
/// </summary>
/// <returns>BOOL, true if extractor can be built, false otherwise</returns>
bool GooseBot::TryMorphExtractor() {
	const ObservationInterface* observation = Observation();
	
	size_t base_count = countUnitType(UNIT_TYPEID::ZERG_HATCHERY)
		+ countUnitType(UNIT_TYPEID::ZERG_LAIR) + countUnitType(UNIT_TYPEID::ZERG_HIVE);
	const Unit * base = GetNewerBase();
	Units geysers = observation->GetUnits(Unit::Alliance::Neutral, IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
	if ((base == nullptr) 
		|| (countUnitType(UNIT_TYPEID::ZERG_EXTRACTOR) >= 2*base_count)
		|| (actionPending(ABILITY_ID::BUILD_EXTRACTOR))
		|| (!CanAfford(UNIT_TYPEID::ZERG_EXTRACTOR))) {
		return false;
	}
	//only search within this radius
	float minimum_distance = 15.0f;
	Tag closestGeyser = 0;
	for (const auto& geyser : geysers) {
		float current_distance = Distance2D(base->pos, geyser->pos);
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
bool GooseBot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID worker_type, size_t struct_cap) {
	// If a unit already is building a supply structure of this type, do nothing.
	// OR If we have the number of these we want already, do nothing
	// OR If we can't afford it, do nothing
	if (actionPending(ability_type_for_structure) || (countUnitType(struct_type) >= struct_cap) || !CanAfford(struct_type)){
		return false;
	}
	// Otherwise, build
    // Get a unit to build the structure.
    const Unit* unit_to_build = FindUnit(worker_type);
	// Try to build at a position in a random direction from builder
    float rx = GetRandomScalar();
    float ry = GetRandomScalar();
	Point2D pos = Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f);
		// Check to see if worker can morph at target location
	if (Query()->Placement(ability_type_for_structure, pos)) {
		Actions()->UnitCommand(unit_to_build, ability_type_for_structure, pos);
		std::cout << "worker found valid build position ";
		return true;
	}
    // Query failed
    return false;
}

//try to morph hatchery into lair
bool GooseBot::TryMorphLair() {
	const Unit *base = GetMainBase();
	if (base == nullptr || !CanAfford(UNIT_TYPEID::ZERG_LAIR) 
		|| actionPending(ABILITY_ID::MORPH_LAIR) || (base->unit_type != UNIT_TYPEID::ZERG_HATCHERY)) {
		return false;
	}
	return TryMorphStructure(ABILITY_ID::MORPH_LAIR, base->tag, base->unit_type);	
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

const Unit* GooseBot::GetMainBase(){
	const ObservationInterface* observation = Observation();
	Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
	if (bases.empty()){
		bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LAIR));
		if (bases.empty()){
			bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HIVE));
			if(bases.empty()){
				return nullptr;
			}
		}
	}
	return GetRandomEntry(bases);
}

const Unit* GooseBot::GetNewerBase(){
	const ObservationInterface* observation = Observation();
	Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
	if (bases.empty()){
		bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_LAIR));
		if (bases.empty()){
			bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HIVE));
			if (!bases.empty()){
				return GetRandomEntry(bases);
			}else{
				return nullptr;
			}
		}else{
			return GetRandomEntry(bases);
		}
	}else{
		return GetRandomEntry(bases);
	}
}
