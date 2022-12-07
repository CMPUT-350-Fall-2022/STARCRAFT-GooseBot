#include "GooseBot.h"
//DEV BRANCH

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

void GooseBot::VerifyBuild(){
    build_phase = 0;
    built_structs = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(struct_units));
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

// Assumes up-to-date num_bases
bool GooseBot::BuildPhase(){
    // OnUnitCreated could be keeping this updated?
    // but for now / in case of structural destruction or other mishap
    VerifyBuild();
    // Find the next item to build by iterating over desired structures,
    // setting the structure as next to build if we do not have it (or enough of it)
    auto to_build = struct_targets.end();
    for (auto it = struct_targets.begin(); it < struct_targets.end(); ++it){
        auto found = std::find(built_types.begin(), built_types.end(), (*it).first);
        if (found == built_types.end()){
            to_build = it;
            if ((*to_build).first == UNIT_TYPEID::ZERG_HATCHERY){
                //only build new hatchery on proper phase or if bases count too low for higher phases
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
            return TryBuildStructure((*to_build).second, (*to_build).first);
        }
    }
    return false;
}

//Check if can afford upgrade
bool GooseBot::CanAfford(UPGRADE_ID upgrade){
    const ObservationInterface* observation = Observation();
    int mineral_supply = observation->GetMinerals();
    int gas_supply = observation->GetVespene();
    auto const upgrade_data = observation->GetUpgradeData();
    for (auto data : upgrade_data){
        if (data.upgrade_id == static_cast<uint32_t>(upgrade)){ 
            if ( (mineral_supply >= data.mineral_cost) && (gas_supply >= data.vespene_cost)){
                return true;
            }else{
                return false;
            }
        }
    }
    std::cout << "data does not contain the ability ";
    return false;
}

//Check if can afford unit
bool GooseBot::CanAfford(UNIT_TYPEID unit){
    const ObservationInterface* observation = Observation();
    int mineral_supply = observation->GetMinerals();
    int gas_supply = observation->GetVespene();
    auto const unit_data = observation->GetUnitTypeData();
    for (auto data : unit_data){
        if (data.unit_type_id == unit){ 
            if ( (mineral_supply >= data.mineral_cost) && (gas_supply >= data.vespene_cost)){
                if ( ((data.tech_requirement != UNIT_TYPEID::INVALID) && (CountUnitType(data.tech_requirement) > 0))
                    || (data.tech_requirement == UNIT_TYPEID::INVALID) ){
                    return true;
                }
                return false;
            }
            return false;
        }
    }
    return false;
}

// Returns number of allied units of given type
size_t GooseBot::CountUnitType(UNIT_TYPEID unit_type){
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}


// Assumes up-to-date num_bases
void GooseBot::SetDroneCap(){
    if (drone_cap < 14*num_bases){
        drone_cap += 3;
    }
}

// Assumes up-to-date num_bases
void GooseBot::SetQueenCap(){
    if (CountUnitType(UNIT_TYPEID::ZERG_SPAWNINGPOOL) > 0
        && FindUnit(UNIT_TYPEID::ZERG_SPAWNINGPOOL)->build_progress == 1){
        queen_cap = 2*num_bases;
    }
    else{
        queen_cap = 0;
    }
}

//TODO, finish making this actually do its job 
// void GooseBot::Prioritize(){
//     if (build_phase <= army_phase){
//         SetSavingsFalse();
//         saving_for_building = true;
//     }
//     else if (build_phase > army_phase){
//         SetSavingsFalse();
//         saving_for_army = true;
//     }
// }

void GooseBot::SetSavingsFalse(){
    saving_for_army = false;
    saving_for_building = false;
    saving_for_research = false;
}




