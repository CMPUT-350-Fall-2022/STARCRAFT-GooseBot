#include "GooseBot.h"

using namespace sc2;

void GooseBot::OnGameStart() { return; }

void GooseBot::OnStep() { 
	
	return; }

// In your bot class.
void GooseBot::OnUnitIdle(const Unit* unit) {
    switch (unit->unit_type.ToType()) {
        case UNIT_TYPEID::ZERG_LARVA: {
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
            break;
        }
        default: {
            break;
        }
    }
}
