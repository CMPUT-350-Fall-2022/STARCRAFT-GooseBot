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


void GooseBot::OnUnitEnterVision(const Unit* unit) {
    const ObservationInterface* observation = Observation();
    Point2D last_seen = unit->pos;
    
    
        //uint32_t army_count = observation->GetArmyCount();
        //Unit* army = new Unit[army_count];
        //Units zergls = observation->GetUnits(Unit::Alliance::Self, IsUnit(zergl));
        //Units banels = observation->GetUnits(Unit::Alliance::Self, IsUnit(banel));
        //Units mutals = observation->GetUnits(Unit::Alliance::Self, IsUnit(mutal));
        //Units roaches = observation->GetUnits(Unit::Alliance::Self, IsUnit(roach));
        //Units queens = observation->GetUnits(Unit::Alliance::Self, IsUnit(queens));

        //Units attack_army = getArmy();
        //for (auto& unit : attack_army) {
        //    std::cout << unit->unit_type << std::endl;
        //}

        switch (unit->unit_type.ToType())
        {
        case commc:
        case hatch:
        case nexus:
        {
            //Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(banels, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(mutals, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(roaches, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(queens, ABILITY_ID::ATTACK_ATTACKTOWARDS, unit);

            enemy_base = unit->pos;
            EnemyLocated = true;
            if (ArmyReady()) {
                //Actions()->UnitCommand(army, ABILITY_ID::ATTACK, unit);
            }
            break;
        }
        default:
        {
            //Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK, unit);
            //Actions()->UnitCommand(army, ABILITY_ID::ATTACK, unit);
            //std::cout << "enemy unit check fail" << std::endl;

            break;
        }
        }
    
    //std::cout << "army check fail" << std::endl;
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
    std::vector<UNIT_TYPEID> army_units = {zergl, roach, banel};
    army = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(army_units));
}

void GooseBot::VerifyArmyFocus(){
    if (build_phase < 4){
        zergl_cap = 30;
        roach_cap = 0;
        mutal_cap = 0;
    }else if (build_phase >= 4 && build_phase <= 6){
        zergl_cap = 30;
        roach_cap = 30;
        mutal_cap = 0;
    }else if (build_phase >= 7 && build_phase < 10){
        zergl_cap = 30;
        roach_cap = 0;
        mutal_cap = 28;
    }
    army_cap = zergl_cap + roach_cap + mutal_cap + queen_cap;
}