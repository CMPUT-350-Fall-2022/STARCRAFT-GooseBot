#include "GooseBot.h"
//DEV BRANCH

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
    if (army.size() >= army_cap) {
        return true;
    }
    else {
       // std::cout << "Army Not Ready"<<std::endl;
        return false;
    }
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

    // check if enemy base still exists
    if (!enemy_base.empty()) {
        if (enemy_base.back() == nullptr) {
            std::cout << "base was defeated" << std::endl;
            enemy_base.pop_back();
        }
    }

    // send army to attack
    if (EnemyLocated && !enemy_base.empty()) {
        
        if (ArmyReady()) {
            Actions()->UnitCommand(army, ABILITY_ID::ATTACK, enemy_base.back());
            last_base = enemy_base.back()->pos;
        }
    }

    if (EnemyLocated && enemy_base.empty() && last_base!=Point2D(0,0)) {
        //go over the battlezone!!!
        
        const Unit* to_murder = FindNearestEnemy(last_base);
        if (to_murder != nullptr) {
            Actions()->UnitCommand(army, ABILITY_ID::ATTACK, to_murder);
        }
    }

    return false;
}

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

void GooseBot::VerifyArmyFocus() {

    if (build_phase == 1) {
        zergl_cap = 0;
        roach_cap = 0;
        mutal_cap = 0;
        banel_cap = 0;
        ultra_cap = 0;

    }
    else if (build_phase >= 2 && build_phase < 4) {
        zergl_cap = 15;
        roach_cap = 0;
        mutal_cap = 0;
        banel_cap = 0;
        ultra_cap = 0;

    }
    else if (build_phase == 4) {
        zergl_cap = 24;
        roach_cap = 16;
        mutal_cap = 0;
        banel_cap = 0;
        ultra_cap = 0;

    }
    else if (build_phase == 5 || build_phase == 6) {
        zergl_cap = 30;
        roach_cap = 24;
        mutal_cap = 0;
        banel_cap = 20;
        ultra_cap = 0;
 
    }else if (build_phase >= 7 && build_phase < 10) {
        zergl_cap = 40;
        roach_cap = 30;
        mutal_cap = 35;
        banel_cap = 18;
        ultra_cap = 0;

    } else if (build_phase == 10) {
        zergl_cap = 45;
        roach_cap = 35;
        mutal_cap = 42;
        banel_cap = 20;
        ultra_cap = 25;
        
    }


    //army_cap = (zergl_cap + roach_cap + mutal_cap + queen_cap)*2/3;
}