#include "GooseBot.h"
//DEV BRANCH

bool GooseBot::ResearchPhase(){
    if (research_phase == 1){
        return TryResearch(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::RESEARCH_PNEUMATIZEDCARAPACE, UPGRADE_ID::OVERLORDSPEED);
    }else{++research_phase;}
    if (research_phase == 2){
        TryResearch(UNIT_TYPEID::ZERG_SPAWNINGPOOL, ABILITY_ID::RESEARCH_ZERGLINGMETABOLICBOOST, UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
        std::cout << "Zergling Speed Increased" << std::endl;
        return true;
    }
    return false;
}

bool GooseBot::TryResearch(UNIT_TYPEID researcher_type, ABILITY_ID ability, UPGRADE_ID upgrade){
    if (actionPending(ability)
        || (std::find(upgraded.begin(), upgraded.end(), upgrade) == upgraded.end())) {
        return false;
    }
    const Unit* researcher = FindUnit(researcher_type);
    if (researcher != nullptr && CanAfford(upgrade)) {
        Actions()->UnitCommand(researcher, ability);
        return true;
    }
    else {
        return false;
    }
}
