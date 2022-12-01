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

        Units attack_army = getArmy();
        //for (auto& unit : attack_army) {
        //    std::cout << unit->unit_type << std::endl;
        //}

        switch (unit->unit_type.ToType())
        {
        case commc:
        case hatch:
        case nexus:
            //Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(banels, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(mutals, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(roaches, ABILITY_ID::ATTACK, last_seen);
            //Actions()->UnitCommand(queens, ABILITY_ID::ATTACK_ATTACKTOWARDS, unit);



            enemy_base = unit->pos;
            EnemyLocated = true;
            if (ArmyReady()) {
                Actions()->UnitCommand(attack_army, ABILITY_ID::ATTACK, enemy_base);
            }
            break;
        default:
            //Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK, unit);
            Actions()->UnitCommand(army, ABILITY_ID::ATTACK, unit);
            //std::cout << "enemy unit check fail" << std::endl;

            break;
        }
    
    //std::cout << "army check fail" << std::endl;
}

bool GooseBot::ArmyPhase(){
    SetDroneCap();
    SetQueenCap();
    SetOverlordCap();

    if (TryBirthQueen()){
        return true;
    }
    VerifyArmy();
    if (army.size() >= army_cap && EnemyLocated){
        Actions()->UnitCommand(army, ABILITY_ID::SMART, enemy_base);
            return true;
    }
    return false;
}

void GooseBot::VerifyArmy(){
    army.clear();
    const ObservationInterface* observation = Observation();
    Units Z = observation->GetUnits(Unit::Alliance::Self, IsUnit(zergl));
    for (auto z : Z){
        army.push_back(z);
    }
    Units R = observation->GetUnits(Unit::Alliance::Self, IsUnit(roach));
    for (auto r : R){
        army.push_back(r);
    }
    Units B = observation->GetUnits(Unit::Alliance::Self, IsUnit(banel));
    for (auto b : B){
        army.push_back(b);
    }
}