#include "GooseBot.h"
/*******************
 * This file contains functions for handling attack units and attacks
********************/

// Returns whether army is large enough to attack
bool GooseBot::ArmyReady() {
    if (army.size() >= army_cap) {
        return true;
    }
    else {
        return false;
    }
}

// Tries to execute army functions, returns true if one executes successfully
// [Called once in OnStep()]
bool GooseBot::ArmyPhase(){
    // Handle base units
    if (TryBirthQueen()){
        std::cout << "Birthed Queen" << std::endl;
        return true; 
    }

    // Handle attack units
    VerifyArmy();
    VerifyArmyFocus();

    // Check if enemy base still exists
    if (!enemy_base.empty()) {
        if (enemy_base.back() == nullptr) {
            std::cout << "base was defeated" << std::endl;
            enemy_base.pop_back();
        }
    }

    // Send army to attack
    if (EnemyLocated && enemy_base.empty() && last_base!=Point2D(0,0)) {
        //go over the battlezone!!!
        
        const Unit* to_murder = FindNearestEnemy(last_base);
        if (to_murder != nullptr) {
            Actions()->UnitCommand(army, ABILITY_ID::ATTACK, to_murder);
        }
    }

    return false;
}

// Resets army vectors
// Called once in ArmyPhase()
void GooseBot::VerifyArmy(){
    army.clear();
    melee.clear();

    std::vector<UNIT_TYPEID> army_units = {zergl, roach, mutal, banel, ultra};

    Units temp = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(army_units));
    if (!temp.empty()) {
        size_t third = temp.size() / 3;
        melee = Units(temp.begin(), temp.begin() + third);
        army = Units(temp.begin() + third, temp.end());
    }
    
}

// Changes different types of unit caps based on build phase
void GooseBot::VerifyArmyFocus() {

    if (build_phase == 1) {
        zergl_cap = 0;
        roach_cap = 0;
        mutal_cap = 0;
        ultra_cap = 0;

    } else if (build_phase >= 2 && build_phase < 4) {
        zergl_cap = 20;
        roach_cap = 0;
        mutal_cap = 0;
        ultra_cap = 0;

    } else if (build_phase >= 4 && build_phase < 7) {
        zergl_cap = 20;
        roach_cap = 40;
        mutal_cap = 0;
        ultra_cap = 0;

    } else if (build_phase >= 7 && build_phase < 10) {
        zergl_cap = 10;
        roach_cap = 10;
        mutal_cap = 28;
        ultra_cap = 0;

    } else if (build_phase == 10) {
        zergl_cap = 5;
        roach_cap = 10;
        mutal_cap = 10;
        ultra_cap = 20;
    }
}

// Counts all army unit types to update variables for OnIdle larva
// [Called once under case:larva in OnIdle()]
void GooseBot::VerifyArmyCounts(){
    const ObservationInterface* obs = Observation();
    size_t drone_count = obs->GetUnits(Unit::Alliance::Self, IsUnit(drone)).size();
    size_t zergl_count = obs->GetUnits(Unit::Alliance::Self, IsUnit(zergl)).size();
    size_t roach_count = obs->GetUnits(Unit::Alliance::Self, IsUnit(roach)).size();
    size_t mutal_count = obs->GetUnits(Unit::Alliance::Self, IsUnit(mutal)).size();
}