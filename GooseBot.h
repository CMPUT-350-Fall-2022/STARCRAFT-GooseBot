#ifndef GOOSE_BOT_H_
#define GOOSE_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2api/sc2_unit.h"
#include "cpp-sc2/include/sc2api/sc2_interfaces.h"
#include "sc2api/sc2_unit_filters.h"
#include <array>

using namespace sc2;

class GooseBot : public sc2::Agent {

    public:
        GooseBot() = default;

        virtual void OnGameStart();
        virtual void OnStep();
        virtual void OnUnitIdle(const sc2::Unit* unit) final;
        virtual void OnGameEnd();

        bool TryMorphStructure(ABILITY_ID ability_type_for_structure,Tag location_tag = NULL, const Point2D& location_point = Point2D(0,0) , UNIT_TYPEID unit_type = UNIT_TYPEID::ZERG_DRONE);
	    bool TryMorphExtractor();
        bool TryBuildSpawningPool();
        bool TryBirthQueen();
        bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID unit_type = UNIT_TYPEID::ZERG_DRONE, size_t struct_cap = 1);
	    size_t countUnitType(UNIT_TYPEID unit_type);
        Tag FindUnitTag(UNIT_TYPEID unit_type);
        bool CanAfford(UNIT_TYPEID unit);
	    void scout(const Unit* unit);
        void VerifyPhase();

	    const Unit* FindNearestMineralPatch(const Point2D& start);
        const Unit* FindNearestAllied(UNIT_TYPEID target_unit, const Point2D& start);

        bool GooseBot::TryHarvestVespene();
    private:
        enum PHASE {SPAWN, ZERGLINGS, END};

        using UnitList = std::array<UNIT_TYPEID, END>;

        size_t phase = 0;
        const std::array<size_t, 2> overlordCap = {2, 3};
        const std::array<size_t, 2> queenCap = {1, 2};
        const UnitList targetStruct = {UNIT_TYPEID::ZERG_SPAWNINGPOOL, UNIT_TYPEID::ZERG_LAIR};
        const UnitList builders = {UNIT_TYPEID::ZERG_DRONE, UNIT_TYPEID::ZERG_HATCHERY};
        const std::array<ABILITY_ID, 2> abilities = {ABILITY_ID::BUILD_SPAWNINGPOOL, ABILITY_ID::MORPH_LAIR};



};
#endif
