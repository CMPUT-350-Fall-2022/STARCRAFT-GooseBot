#include "GooseBot.h"

bool checkConditions(size_t z, size_t b, size_t r, size_t m, size_t q) {
    size_t z_ideal = 10, b_ideal = 10, r_ideal = 0, m_ideal = 0, q_ideal = 0;
    for (size_t i = 0; i < 5; i++)
    {
        if (z < z_ideal) {
            return false;
        }
        if (b < b_ideal) {
            return false;
        }
        if (r < r_ideal) {
            return false;
        }
        if (m < m_ideal) {
            return false;
        }
        if (q < q_ideal) {
            return false;
        }
    }
    return true;
}

bool GooseBot::ArmyReady() {
    size_t zergl_count = CountUnitType(zergl);
    size_t banel_count = CountUnitType(banel);
    size_t roach_count = CountUnitType(roach);
    size_t mutal_count = CountUnitType(mutal);
    size_t queen_count = CountUnitType(queen);

    //return checkConditions(zergl_count, banel_count, roach_count, mutal_count, queen_count);
    return true;
}

bool GooseBot::ArmyPhase(){
    SetDroneCap();
    SetQueenCap();

    // Handle base units
    if (TryBirthQueen()){
        std::cout << "Birthed Queen" << std::endl;
        return true; 
    }

    // Handle attack units
    VerifyArmy();
    VerifyArmyFocus();
    // send half to attack
    if (army.size() >= army_cap && EnemyLocated){
        //Actions()->UnitCommand(Units(army.begin(), army.begin() + army_cap), ABILITY_ID::ATTACK, enemy_base);
        return true;
    }
    return false;
}

void GooseBot::VerifyArmy(){
    army.clear();
    melee.clear();

    std::vector<UNIT_TYPEID> army_units = {zergl, roach, banel, ultra};

    Units temp = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(army_units));
    if (!temp.empty()) {
        size_t third = temp.size() / 3;
        melee = Units(temp.begin(), temp.begin() + third);
        army = Units(temp.begin() + third, temp.end());
    }
    
}

void GooseBot::VerifyArmyFocus() {

    if (build_phase == 1) {
        zergl_cap = 2;
        roach_cap = 0;
        mutal_cap = 0;
        banel_cap = 0;
        ultra_cap = 0;

    }
    else if (build_phase >= 2 && build_phase < 4) {
        zergl_cap = 30;
        roach_cap = 0;
        mutal_cap = 0;
        banel_cap = 0;
        ultra_cap = 0;

    }
    else if (build_phase == 4) {
        zergl_cap = 20;
        roach_cap = 30;
        mutal_cap = 0;
        banel_cap = 0;
        ultra_cap = 0;

    }
    else if (build_phase == 5 || build_phase == 6) {
        zergl_cap = 10;
        roach_cap = 10;
        mutal_cap = 0;
        banel_cap = 30;
        ultra_cap = 0;
 
    }else if (build_phase >= 7 && build_phase < 10) {
        zergl_cap = 10;
        roach_cap = 10;
        mutal_cap = 28;
        banel_cap = 10;
        ultra_cap = 0;

    } else if (build_phase == 10) {
        zergl_cap = 10;
        roach_cap = 5;
        mutal_cap = 5;
        banel_cap = 5;
        ultra_cap = 20;
        
    }


    army_cap = zergl_cap + roach_cap + mutal_cap + queen_cap;
}