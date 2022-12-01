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
#include <unordered_set>

using namespace sc2;

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

class GooseBot : public sc2::Agent {

    public:
        GooseBot() = default;

        virtual void OnGameStart();
        virtual void OnStep();
        virtual void OnUnitIdle(const Unit* unit) final;
        virtual void OnUnitEnterVision(const Unit* unit) final;
        virtual void OnGameEnd();

        bool TryMorphStructure(ABILITY_ID ability_type_for_structure,Tag location_tag, UNIT_TYPEID worker_unit = UNIT_TYPEID::ZERG_DRONE);
        bool TryMorphStructure(ABILITY_ID ability_type_for_structure, const Point2D& location_point = Point2D(0,0) , UNIT_TYPEID worker_unit = UNIT_TYPEID::ZERG_DRONE);
	    bool TryMorphExtractor();
 
        bool TryBirthQueen();
        bool TryBuildHatchery();
        bool TryMorphLair();
        bool TryMorphHive();
        bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID worker_type = UNIT_TYPEID::ZERG_DRONE, size_t struct_cap = 1);
	    bool TryResearch(UNIT_TYPEID researcher_type, ABILITY_ID ability, UPGRADE_ID upgrade);
        
        bool actionPending(ABILITY_ID action);
        size_t CountUnitType(UNIT_TYPEID unit_type);

        const Unit* FindUnit(UNIT_TYPEID unit_type);

        const Unit* GetMainBase();
        const Unit* GetNewerBase();

        bool CanAfford(UNIT_TYPEID unit);
        bool CanAfford(UPGRADE_ID upgrade);
	    void scout(const Unit* unit);

        bool BuildPhase();
        void CountBases();
        void SetDroneCap();
        void SetQueenCap();

        bool ArmyPhase();
        void SetOverlordCap();

        bool ResearchPhase();

        void VerifyPending();
        void VerifyArmy();
        void Prioritize();

        const Unit* FindNearestMineralPatch(const Point2D& start);
        const Unit* FindNearestAllied(UNIT_TYPEID target_unit, const Point2D& start);

        bool GooseBot::TryHarvestVespene();

        Point2D GooseBot::getEnemyLocation();
        Units GooseBot::getArmy();
        bool GooseBot::ArmyReady();


    private:
        size_t num_bases;
        size_t drone_cap;
        size_t queen_cap;
        size_t overlord_cap = 2;

        std::unordered_set<ABILITY_ID> pendingOrders;

        std::vector<UPGRADE_ID> upgraded;

        size_t build_phase;
        bool saving_for_building; 
        size_t army_phase;
        bool saving_for_army;
        size_t research_phase;
      
        UNIT_TYPEID target_struct;
        ABILITY_ID builder_ability;
        UNIT_TYPEID builder;

        Units army;
        size_t army_cap = 30;
        Point2D enemy_base;
        bool EnemyLocated = false;



};
#endif
