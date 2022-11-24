#include "GooseBot.h"
#define larva UNIT_TYPEID::ZERG_LARVA
#define drone UNIT_TYPEID::ZERG_DRONE
#define zergl UNIT_TYPEID::ZERG_ZERGLING
#define banel UNIT_TYPEID::ZERG_BANELING
#define overl UNIT_TYPEID::ZERG_OVERLORD
#define queen UNIT_TYPEID::ZERG_QUEEN
#define roach UNIT_TYPEID::ZERG_ROACH
#define mutal UNIT_TYPEID::ZERG_MUTALISK
#define hatch UNIT_TYPEID::ZERG_HATCHERY
#define commc UNIT_TYPEID::TERRAN_COMMANDCENTER
#define nexus UNIT_TYPEID::PROTOSS_NEXUS

bool checkConditions(size_t z, size_t b, size_t r, size_t m, size_t q) {
    size_t z_ideal = 10, b_ideal = 10, r_ideal = 5, m_ideal = 10, q_ideal = 1;
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
    size_t zergl_count = countUnitType(zergl);
    size_t banel_count = countUnitType(banel);
    size_t roach_count = countUnitType(roach);
    size_t mutal_count = countUnitType(mutal);
    size_t queen_count = countUnitType(queen);

    return checkConditions(zergl_count, banel_count, roach_count, mutal_count, queen_count);
}

//experimental function to see if we can attack without seeing the enemy
void GooseBot::Attack() { 
    const ObservationInterface* observation = Observation();
    Units enemies = observation->GetUnits(Unit::Alliance::Enemy);
    Units terran_bases = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(commc));
    Units protoss_bases = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(nexus));
    Units zerg_bases = observation->GetUnits(Unit::Alliance::Enemy, IsUnit(hatch));
    if (enemies.empty() || terran_bases.empty() || protoss_bases.empty() || zerg_bases.empty()) {
        return;
    }
    Units zergls = observation->GetUnits(Unit::Alliance::Self, IsUnit(zergl));
    Units banels = observation->GetUnits(Unit::Alliance::Self, IsUnit(banel));
    Units mutals = observation->GetUnits(Unit::Alliance::Self, IsUnit(mutal));
    Units roaches = observation->GetUnits(Unit::Alliance::Self, IsUnit(roach));
    
    const Unit* terran_base = GetRandomEntry(terran_bases);
    Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK_ATTACKTOWARDS, terran_base->pos);
    Actions()->UnitCommand(banels, ABILITY_ID::ATTACK_ATTACKTOWARDS, terran_base->pos);
    Actions()->UnitCommand(mutals, ABILITY_ID::ATTACK_ATTACKTOWARDS, terran_base->pos);
    Actions()->UnitCommand(roaches, ABILITY_ID::ATTACK_ATTACKTOWARDS, terran_base->pos);
}

bool GooseBot::EnemyLocated() { return true; }

void GooseBot::OnUnitEnterVision(const Unit* unit) {
    const ObservationInterface* observation = Observation();
    Point2D last_seen = unit->pos;
    
    if (ArmyReady()) {
        //uint32_t army_count = observation->GetArmyCount();
        //Unit* army = new Unit[army_count];
        Units zergls = observation->GetUnits(Unit::Alliance::Self, IsUnit(zergl));
        Units banels = observation->GetUnits(Unit::Alliance::Self, IsUnit(banel));
        Units mutals = observation->GetUnits(Unit::Alliance::Self, IsUnit(mutal));
        Units roaches = observation->GetUnits(Unit::Alliance::Self, IsUnit(roach));
        //Units queens = observation->GetUnits(Unit::Alliance::Self, IsUnit(queens));
        switch (unit->unit_type.ToType())
        {
        case commc:
        case hatch:
        case nexus:
            Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK_ATTACKTOWARDS, last_seen);
            Actions()->UnitCommand(banels, ABILITY_ID::ATTACK_ATTACKTOWARDS, last_seen);
            Actions()->UnitCommand(mutals, ABILITY_ID::ATTACK_ATTACKTOWARDS, last_seen);
            Actions()->UnitCommand(roaches, ABILITY_ID::ATTACK_ATTACKTOWARDS, last_seen);
            //Actions()->UnitCommand(queens, ABILITY_ID::ATTACK_ATTACKTOWARDS, unit);
            break;
        default:
            Actions()->UnitCommand(zergls, ABILITY_ID::ATTACK_ATTACKTOWARDS, unit);
            break;
        }
    }
}