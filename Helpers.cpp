#include "GooseBot.h"
/*******************
 * This file contains functions for checking pre-conditions
********************/


// Checks whether a given ABILITY_ID is already in progress
bool GooseBot::actionPending(ABILITY_ID action){
	return (std::find(pendingOrders.begin(), pendingOrders.end(), action) != pendingOrders.end());
}


// Re-calculates which actions are pending 
// [Call once in beginning of OnStep()]
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
