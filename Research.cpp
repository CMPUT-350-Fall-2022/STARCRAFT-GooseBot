#include "GooseBot.h"
//DEV BRANCH

bool GooseBot::ResearchPhase(){
    if (Observation()->GetVespene() > 100){
        bool success = TryResearch(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::RESEARCH_PNEUMATIZEDCARAPACE, UPGRADE_ID::OVERLORDSPEED);
        if (success){
            upgraded.push_back(UPGRADE_ID::OVERLORDSPEED);
            ++research_phase;
        }
        if (IsBuilt(UNIT_TYPEID::ZERG_SPAWNINGPOOL)){
            bool success = TryResearch(UNIT_TYPEID::ZERG_SPAWNINGPOOL, ABILITY_ID::RESEARCH_ZERGLINGMETABOLICBOOST, UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
            if (success){
                std::cout << "Zergling Speed Increased" << std::endl;
                upgraded.push_back(UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
                ++research_phase;
            }
            return success;
        }
        return false;
    }else{return false;}
}

bool GooseBot::TryResearch(UNIT_TYPEID researcher_type, ABILITY_ID ability, UPGRADE_ID upgrade){
    if (IsUpgraded(upgrade) || actionPending(ability)) {
        return false;
    }
    const Unit* researcher = FindUnit(researcher_type);
    if (researcher != nullptr) {
        Actions()->UnitCommand(researcher, ability);
        return true;
    }
    else {
        return false;
    }
}

bool GooseBot::IsUpgraded(UPGRADE_ID to_test){
    return (std::find(upgraded.begin(), upgraded.end(), to_test) != upgraded.end());
}
