#include "GooseBot.h"

bool GooseBot::ResearchPhase(){
    if (CanAfford(UPGRADE_ID::OVERLORDSPEED)){
        bool success = TryResearch(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::RESEARCH_PNEUMATIZEDCARAPACE, UPGRADE_ID::OVERLORDSPEED);
        if (success){
            upgraded.push_back(UPGRADE_ID::OVERLORDSPEED);
        }
        if (IsBuilt(UNIT_TYPEID::ZERG_SPAWNINGPOOL)){
            bool success = TryResearch(UNIT_TYPEID::ZERG_SPAWNINGPOOL, ABILITY_ID::RESEARCH_ZERGLINGMETABOLICBOOST, UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
            if (success){
                std::cout << "Zergling Speed Increased" << std::endl;
                upgraded.push_back(UPGRADE_ID::ZERGLINGMOVEMENTSPEED);
            }
            return success;
        }
    }
    if (IsBuilt(UNIT_TYPEID::ZERG_SPIRE)){
        UPGRADE_ID flyer_upgrade = UPGRADE_ID::ZERGFLYERWEAPONSLEVEL1;
        if (IsUpgraded(flyer_upgrade)){
            flyer_upgrade = UPGRADE_ID::ZERGFLYERWEAPONSLEVEL2;
        }
        if (CanAfford(flyer_upgrade)){
            bool success = TryResearch(UNIT_TYPEID::ZERG_SPIRE, ABILITY_ID::RESEARCH_ZERGFLYERATTACK, flyer_upgrade);
            if (success){
                std::cout << "Flyer Attack Increased" << std::endl;
                upgraded.push_back(flyer_upgrade);
            }
            return success;
        }else {return false;}
    }
    if (CanAfford(UPGRADE_ID::CHITINOUSPLATING)){
        if (IsBuilt(UNIT_TYPEID::ZERG_ULTRALISKCAVERN)){
            bool success = TryResearch(UNIT_TYPEID::ZERG_ULTRALISKCAVERN, ABILITY_ID::RESEARCH_CHITINOUSPLATING, UPGRADE_ID::CHITINOUSPLATING);
            if (success){
                std::cout << "Ultralisk Armor Increased" << std::endl;
                upgraded.push_back(UPGRADE_ID::CHITINOUSPLATING);
            }
            return success;
        }
    }else{return false;}
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

bool GooseBot::IsUpgraded(UPGRADE_ID to_test){
    return (std::find(upgraded.begin(), upgraded.end(), to_test) != upgraded.end());
}