#include "GooseBot.h"
/*******************
 * This file contains functions for building and morphing structures
********************/


// Verifies which buildings we want to build are already built, and which build_phase is current
// [Call once in beginning of BuildPhase()]
void GooseBot::VerifyBuild(){
    build_phase = 0;
    Units built_structs = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(struct_units));
    built_types.clear();
    for (auto s : built_structs){
        built_types.push_back(s->unit_type);
        if (s->unit_type == UNIT_TYPEID::ZERG_LAIR){
            ++build_phase;
        }else if (s->unit_type == UNIT_TYPEID::ZERG_HIVE){
            build_phase += 2;
        }
    }
    build_phase += built_types.size();
}

// Use pre-determined build order to choose what to build next
// Returns whether the attempted build action was successfully given
// Assumes up-to-date num_bases
bool GooseBot::BuildPhase(){
    VerifyBuild();
    // Find the next item to build by iterating over desired structures,
    // setting the structure as next to build if we do not have it (or enough of it)
    auto to_build = struct_targets.end();
    for (auto it = struct_targets.begin(); it < struct_targets.end(); ++it){
        // if desired structure is not found in built structures, set it as the next item to build
        auto found = std::find(built_types.begin(), built_types.end(), (*it).first);
        if (found == built_types.end()){
            to_build = it;
            if ((*to_build).first == UNIT_TYPEID::ZERG_HATCHERY){
                //only build up to max 4 bases
                if (num_bases < 4){
                    break;
                }else{
                    continue;
                }
            }else{
                break;
            }
        }
    }// Build next structure based on its custom build function, if it has one
    //  Otherwise use the generic build structure function
    for (size_t j = 0; j < struct_targets.size(); ++j){
        if (build_phase != 1 && CountUnitType(UNIT_TYPEID::ZERG_EXTRACTOR) < num_bases*2){
            return TryMorphExtractor();
        }
        else if ((*to_build).first == UNIT_TYPEID::ZERG_HATCHERY){
            return TryBuildHatchery();
        }
        else if ((*to_build).first == UNIT_TYPEID::ZERG_LAIR){
            return TryMorphLair();
        }
        else if ((*to_build).first == UNIT_TYPEID::ZERG_HIVE){
            return TryMorphHive();
        }
        else {
            return TryBuildNearby((*to_build).second, (*to_build).first);
        }
    }
    return false;
}

// Try to morph a drone into a structure at a given 2D point
// returns true if can morph
bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, const Point2D& location_point, UNIT_TYPEID worker_unit) {
	// If no worker is already building one, get a random worker to build one
	const Unit* unit = FindUnit(worker_unit);
	if (unit == nullptr) {
		return false;
	}

	// Check to see if unit can build at target location
	if (Query()->Placement(ability_type_for_structure, location_point)) {
		Actions()->UnitCommand(unit, ability_type_for_structure, location_point);
		return true;
	}
	return false;
}

// Checks if there structur can be morphed, if true morphs/builds structure
// Requires tag of preexisting unit to morph from
bool GooseBot::TryMorphStructure(ABILITY_ID ability_type_for_structure, Tag location_tag, UNIT_TYPEID worker_unit) {
	const ObservationInterface* observation = Observation(); 
	//Get a random worker
	const Unit * worker = FindUnit(worker_unit);
	//if we have no workers, return false
	if (worker == nullptr) {
		return false;
	}
	// Get target to morph; note that target and worker can be the same, 
	// but should not be the same unit type with different tags, as this would
	// mean that a unit would try to do a self targeting action on another unit
	const Unit* target = observation->GetUnit(location_tag);
	if ((worker->tag != location_tag) && (target->unit_type == worker_unit)){
		Actions()->UnitCommand(target, ability_type_for_structure);
		return true;
	}else if (worker->tag == location_tag){		//self-targeting
		Actions()->UnitCommand(worker, ability_type_for_structure);
		return true;
	}// Otherwise, builder unit different than target unit
	// Check to see if worker can morph at target location
	if (Query()->Placement(ability_type_for_structure, target->pos)) {
		Actions()->UnitCommand(worker, ability_type_for_structure, target);
		return true;
	}
	return false;
}

/// Checks if there is a Vespene Geyser nearby the hatchery, if true passes parameters to build extractor to TryMorphStructure()
/// returns BOOL, true if extractor can be built, false otherwise
bool GooseBot::TryMorphExtractor() {
	const ObservationInterface* observation = Observation();
	const Unit* base = GetRandomBase();
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

// from tutorial, adapted
// Try to build a structure with given ability, struct type, 
// & optional worker type[drone default]
bool GooseBot::TryBuildNearby(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID worker_type) {
	// If a unit already is building a supply structure of this type, do nothing.
	// OR If we can't afford it, do nothing
	if (actionPending(ability_type_for_structure) || !CanAfford(struct_type)){
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
	// Check to see if worker can place it at target location
	if (Query()->Placement(ability_type_for_structure, pos)) {
		Actions()->UnitCommand(unit_to_build, ability_type_for_structure, pos);
		return true;
	}
    // Query failed
    return false;
}

// Uses pre-computed potential base locations to try to build an expansion
// returns true if success, false otherwise
bool GooseBot::TryBuildHatchery() {
	const Unit* unit_to_build = FindUnit(drone);
	if (actionPending(ABILITY_ID::BUILD_HATCHERY) 
		|| !CanAfford(UNIT_TYPEID::ZERG_HATCHERY)
		|| (unit_to_build == nullptr)){
		return false;
	}

	int spotIndex = 0;
    Point2D buildSpot = possibleBaseGrounds[spotIndex];
    int breakCounter = 0;
    bool morphedHatchery;
    while (!(morphedHatchery = TryMorphStructure(ABILITY_ID::BUILD_HATCHERY, buildSpot))){
        if (!(spotIndex < possibleBaseGrounds.size() - 1)){ 
            break;
        }
        if (breakCounter >= 20){
            spotIndex++;
            buildSpot = possibleBaseGrounds[spotIndex];
            breakCounter = 0;
        }
        else{
            buildSpot += Point2D(GetRandomScalar() * 3, GetRandomScalar() * 3);
            breakCounter++;
        }
    }
	return morphedHatchery;
}

// Try to morph hatchery into lair
// returns true if success, false otherwise
bool GooseBot::TryMorphLair() {
	const Unit *base = FindUnit(UNIT_TYPEID::ZERG_HATCHERY);
	if (base == nullptr || !CanAfford(UNIT_TYPEID::ZERG_LAIR) || CountUnitType(UNIT_TYPEID::ZERG_LAIR) == 1 
		|| actionPending(ABILITY_ID::MORPH_LAIR) ) {
		return false;
	}
	return TryMorphStructure(ABILITY_ID::MORPH_LAIR, base->tag, base->unit_type);	
}

// Try to morph lair into hive
// returns true if success, false otherwise
bool GooseBot::TryMorphHive() {
	const Unit *base = FindUnit(UNIT_TYPEID::ZERG_LAIR);
	if (base==nullptr || !CanAfford(UNIT_TYPEID::ZERG_HIVE)
		|| actionPending(ABILITY_ID::MORPH_HIVE)) {
		return false;
	}
	return TryMorphStructure(ABILITY_ID::MORPH_HIVE, base->tag, base->unit_type);	
}

// Returns a random selection of available bases
const Unit* GooseBot::GetRandomBase(){
	const ObservationInterface* observation = Observation();
	Units bases = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(baseTypes));
	return GetRandomEntry(bases);
}

// Updates num_bases
// [Called once in OnStep]
 void GooseBot::CountBases(){
	num_bases = 0;
	const ObservationInterface* observation = Observation();
	num_bases = observation->GetUnits(Unit::Alliance::Self, IsUnits(baseTypes)).size();	
}

