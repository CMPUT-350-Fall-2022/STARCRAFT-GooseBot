#include "GooseBot.h"
/*******************
 * This file contains functions for building and morphing structures
********************/

/// <summary>
/// Tries to morph a given structure at a give 2D point. By default worker_unit is a drone
/// </summary>
/// <param name="ability_type_for_structure"></param>
/// <param name="location_point"></param>
/// <param name="worker_unit"></param>
/// <returns>BOOL true if structure can be morphed from unit, false otherwise</returns>
bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, const Point2D& location_point, UNIT_TYPEID worker_unit) {
	//get an observation of the current game state
	const ObservationInterface* observation = Observation(); 

	// If no worker is already building one, get a random worker to build one
	const Unit* unit = FindUnit(worker_unit);
	if (unit == nullptr) {
		std::cout << "fails available worker check ,";
		return false;
	}

	// Check to see if unit can build there
	if (Query()->Placement(ability_type_for_structure, location_point)) {
		Actions()->UnitCommand(unit, ability_type_for_structure, location_point);
		return true;
	}
	return false;
}

/// <summary>
/// Tries to morph a given structure at a give location Tag. By default worker_unit is a drone
/// </summary>
/// <param name="ability_type_for_structure"></param>
/// <param name="location_tag"></param>
/// <param name="worker_unit"></param>
/// <returns>BOOL, true if structure can be morphed from unit, false otherwise</returns>
bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, Tag location_tag, UNIT_TYPEID worker_unit) {
	const ObservationInterface* observation = Observation(); 
	//Get a random worker
	const Unit * worker = FindUnit(worker_unit);
	//if we have no workers, return false
	if (worker == nullptr) {
		std::cout << "fails available worker check ";
		return false;
	}
	// Get target to morph; note that target and worker can be the same, 
	// but should not be the same unit type with different tags, as this would
	// mean that a unit would try to do a self targeting action on another unit
	const Unit* target = observation->GetUnit(location_tag);
	if ((worker->tag != location_tag) && (target->unit_type == worker_unit)){
		Actions()->UnitCommand(target, ability_type_for_structure);
		std::cout << "target self targeted ";
		return true;
	}else if (worker->tag == location_tag){		//self-targeting
		Actions()->UnitCommand(worker, ability_type_for_structure);
		std::cout << "worker self targeted ";
		return true;
	}// Otherwise, builder unit different than target unit
	
	// Check to see if worker can morph at target location
	if (Query()->Placement(ability_type_for_structure, target->pos)) {
		Actions()->UnitCommand(worker, ability_type_for_structure, target);
		std::cout << "worker targeted non-worker ";
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
	const Unit* base = GetNewerBase();
	Units geysers = observation->GetUnits(Unit::Alliance::Neutral, IsUnits(vespeneTypes));
	if ((base == nullptr) 
		|| (CountUnitType(UNIT_TYPEID::ZERG_EXTRACTOR) >= 2*num_bases)
		|| (actionPending(ABILITY_ID::BUILD_EXTRACTOR))
		|| (!CanAfford(UNIT_TYPEID::ZERG_EXTRACTOR))) {
		return false;
	}
	const Unit* closestGeyser = FindNearestVespeneGeyser(base->pos);
	// In the case where there are no more available geysers nearby
	if (closestGeyser == nullptr) {
		return false;
	}
	return TryMorphStructure(ABILITY_ID::BUILD_EXTRACTOR, closestGeyser->tag);
}

/// <summary>
/// from tutorial, adapted
//  Try to build a structure with given ability, struct type, 
//  & optional worker type[drone default], max num of structs desired[1 default] 
/// </summary>
/// <param name="ability_type_for_structure"></param>
/// <param name="struct_type"></param>
/// <param name="worker_type"></param>
/// <param name="struct_cap"></param>
/// <returns>BOOL, true if structure can be built, false otherwise</returns>
bool GooseBot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID worker_type, size_t struct_cap) {
	// If a unit already is building a supply structure of this type, do nothing.
	// OR If we have the number of these we want already, do nothing
	// OR If we can't afford it, do nothing
	if (actionPending(ability_type_for_structure) || CountUnitType(struct_type) == struct_cap || !CanAfford(struct_type)){
		//std::cout << "failed pre-build checks" << std::endl;
		return false;
	}
	// Otherwise, build
    // Get a unit to build the structure.
    const Unit* unit_to_build = FindUnit(worker_type);
	// Try to build at a position in a random direction from builder
	if (unit_to_build == nullptr) {
		return false;
	}
    float rx = GetRandomScalar();
    float ry = GetRandomScalar();
	Point2D pos = Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f);
		// Check to see if worker can morph at target location
	if (Query()->Placement(ability_type_for_structure, pos)) {
		Actions()->UnitCommand(unit_to_build, ability_type_for_structure, pos);
		//std::cout << "worker found valid build position ";
		return true;
	}
    // Query failed
	//std::cout << "failed placement query" << std::endl;
    return false;
}
/// <summary>
/// Uses pre-computed potential base locations to try to build an expansion
/// </summary>
/// <returns>BOOL, true if success, false otherwise</returns>
bool GooseBot::TryBuildHatchery() {
	const Unit* unit_to_build = FindUnit(drone);
	if (actionPending(ABILITY_ID::BUILD_HATCHERY) 
		|| !CanAfford(UNIT_TYPEID::ZERG_HATCHERY)
		|| (unit_to_build == nullptr)){
			std::cout << "Failed to find unit to build Hatchery" << std::endl;
		return false;
	}

	for (auto &buildSpot : possibleBaseGrounds)
	{
		if (Query()->Placement(ABILITY_ID::BUILD_HATCHERY, buildSpot))
		{
			std::cout << "Trying to build Hatchery" << std::endl;
			Actions()->UnitCommand(unit_to_build, ABILITY_ID::BUILD_HATCHERY, buildSpot);
			return true;
		}
		else
		{
		
		}
	}
    // Query failed
    return false;
}

/// <summary>
/// try to morph hatchery into lair
/// </summary>
/// <returns>BOOL, true if TryMorphStructure call succeeds</returns>
bool GooseBot::TryMorphLair() {
	const Unit *base = GetMainBase();
	if (base == nullptr || !CanAfford(UNIT_TYPEID::ZERG_LAIR) || CountUnitType(UNIT_TYPEID::ZERG_LAIR) == 1 
		|| actionPending(ABILITY_ID::MORPH_LAIR) ) {
		return false;
	}
	else if (base->unit_type != UNIT_TYPEID::ZERG_HATCHERY) {
		return false;
	}
	std::cout << "morphing Lair" << std::endl;
	return TryMorphStructure(ABILITY_ID::MORPH_LAIR, base->tag, base->unit_type);	
}

/// <summary>
/// try to morph lair into hive
/// </summary>
/// <returns>BOOL, true if TryMorphStructure call succeeds</returns>
bool GooseBot::TryMorphHive() {
	const Unit *base = GetMainBase();
	if (base==nullptr) {
		return false;
	}
	if ((base->unit_type != UNIT_TYPEID::ZERG_LAIR) 
		&& CanAfford(UNIT_TYPEID::ZERG_HIVE)
		&& (!actionPending(ABILITY_ID::MORPH_HIVE))) {
		return false;
	}
	return TryMorphStructure(ABILITY_ID::MORPH_HIVE, base->tag, base->unit_type);	
}

/// <summary>
/// Gets the first base we own from observation call
/// </summary>
/// <returns>const Unit* base</returns>
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

/// <summary>
/// Gets the random base we own from observation call
/// </summary>
/// <returns>const Unit* base</returns>
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

//
 void GooseBot::HandleBases(){
	num_bases = 0;
	const ObservationInterface* observation = Observation();
	num_bases = observation->GetUnits(Unit::Alliance::Self, IsUnits(baseTypes)).size();
	if (build_phase == 4|| build_phase == 8){
		//TryDistributeMineralWorkers();
	}	
}
