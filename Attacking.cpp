#include "GooseBot.h"


bool GooseBot::ArmyReady() {
    size_t zergl_count = countUnitType(zergl);
    size_t banel_count = countUnitType(banel);
    size_t roach_count = countUnitType(roach);
    size_t mutal_count = countUnitType(mutal);
    size_t queen_count = countUnitType(queen);

    return checkConditions(zergl_count, banel_count, roach_count, mutal_count, queen_count);
}

bool GooseBot::Attack() { return true; }

bool GooseBot::EnemyLocated() { return true; }

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