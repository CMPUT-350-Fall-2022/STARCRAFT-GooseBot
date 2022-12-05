#ifndef GOOSE_BOT_H_
#define GOOSE_BOT_H_

// MORPH_OVERLORDTRANSPORT no target

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
#define ultra UNIT_TYPEID::ZERG_ULTRALISK
#define hatch UNIT_TYPEID::ZERG_HATCHERY
#define commc UNIT_TYPEID::TERRAN_COMMANDCENTER
#define nexus UNIT_TYPEID::PROTOSS_NEXUS

class GooseBot : public sc2::Agent {

    public:
        GooseBot() = default;

        // In GooseBot.cpp
        virtual void OnGameStart();
        virtual void OnStep();
        virtual void OnUnitIdle(const Unit* unit) final;
        virtual void OnUnitDestroyed(const Unit* unit) final;
        virtual void OnUnitEnterVision(const Unit* unit) final;
        virtual void OnGameEnd();
        virtual void OnUnitCreated(const Unit* unit);
        virtual void OnBuildingConstructionComplete(const Unit* unit);
        virtual void OnUpgradeCompleted(UPGRADE_ID);

        // In Building.cpp
        bool TryMorphStructure(ABILITY_ID ability_type_for_structure,Tag location_tag, UNIT_TYPEID worker_unit = UNIT_TYPEID::ZERG_DRONE);
        bool TryMorphStructure(ABILITY_ID ability_type_for_structure, const Point2D& location_point = Point2D(0,0) , UNIT_TYPEID worker_unit = UNIT_TYPEID::ZERG_DRONE);
	    bool TryMorphExtractor();
        bool TryMorphLair();
        bool TryMorphHive();
        bool TryBuildHatchery();
        bool TryBuildNearby(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID worker_type = UNIT_TYPEID::ZERG_DRONE);
        const Unit* GetRandomBase();
        bool BuildPhase();
        void CountBases();
        void VerifyBuild();

        // In UnitHandling.cpp
        bool TryBirthQueen();
        bool GooseBot::TryHarvestVespene();

        // In Research.cpp
        bool TryResearch(UNIT_TYPEID researcher_type, ABILITY_ID ability, UPGRADE_ID upgrade);
        bool ResearchPhase();

        // In Helpers.cpp
        bool actionPending(ABILITY_ID action);
        size_t CountUnitType(UNIT_TYPEID unit_type);
        bool CanAfford(UNIT_TYPEID unit);
        bool CanAfford(UPGRADE_ID upgrade);
        void VerifyPending();
        
        // In Attacking.cpp
        void VerifyArmyCounts();
        bool ArmyPhase();
        void VerifyArmy();
        void VerifyArmyFocus();
        bool ArmyReady();

        // In Searching.cpp
        const Unit* FindUnit(UNIT_TYPEID unit_type);
        void scout(const Unit* unit);
        void scoutPoint(const Unit* unit, Point2D point);
        bool UnitsWithinProximity(float proximity, const Unit& unit1, const Unit& unit2) const;
        const std::vector<Point2D> FindBaseBuildingGrounds();
        const Units FindAllMineralPatches();
        const Unit* FindNearestMineralPatch(const Point2D& start);
        const Unit* FindNearestVespeneGeyser(const Point2D& start);
        const Unit* FindNearestAllied(UNIT_TYPEID target_unit, const Point2D& start);
        const Unit* FindNearestAllied(std::vector<UNIT_TYPEID> target_units, const Point2D& start);

    private:
        // Number of bases
        size_t num_bases;

        // Army unit caps
        size_t zergl_cap;
        size_t roach_cap;
        size_t mutal_cap;
        size_t ultra_cap;

        // Army unit counts
        size_t drone_count;
        size_t zergl_count;
        size_t roach_count;
        size_t mutal_count;

        // Actions not yet completed
        std::unordered_set<ABILITY_ID> pendingOrders;

        // Upgrades finished
        std::vector<UPGRADE_ID> upgraded;

        // Build phase tracking
        size_t build_phase = 1;
        std::vector<UNIT_TYPEID> built_types;

        // Research phase tracking
        size_t research_phase = 1;
        
        // Unit Filter Vectors for Observation.GetUnits()
        const std::vector<UNIT_TYPEID> vespeneTypes = { UNIT_TYPEID::NATURALGAS, UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_VESPENEGEYSER, UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER };
        const std::vector<UNIT_TYPEID> mineralTypes = { UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD, UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750, UNIT_TYPEID::NEUTRAL_MINERALFIELD, UNIT_TYPEID::NEUTRAL_MINERALFIELD450, UNIT_TYPEID::NEUTRAL_MINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750 };
        const std::vector<UNIT_TYPEID> townHallTypes = { UNIT_TYPEID::ZERG_HATCHERY, UNIT_TYPEID::PROTOSS_NEXUS, UNIT_TYPEID::TERRAN_COMMANDCENTER };
        const std::vector<UNIT_TYPEID> baseTypes = { UNIT_TYPEID::ZERG_HATCHERY, UNIT_TYPEID::ZERG_LAIR, UNIT_TYPEID::ZERG_HIVE };

        // Stores possible places that bases can be built after FindBaseBuildingGrounds() is called in OnStart()
        std::vector<Point2D> possibleBaseGrounds;
        std::vector<Point2D> enemyStartLocations;

        // For attacking
        // Overall army, melee units & caps
        Units army;
        Units idle_larvae;
        Units melee;
        size_t melee_cap = 6;
        size_t army_cap = 20;
        
        // Enemy base(s)
        Units enemy_base;
        bool EnemyLocated = false;

        // Scouts
        std::vector<std::pair<int, const Unit*>> generalScouts = {};
        std::vector<std::pair<int, const Unit*>> suicideScouts = {};

        // Pre-determined build order w/ abilities
        using BuildPair = std::pair<UNIT_TYPEID, ABILITY_ID>;
        const std::vector<BuildPair> struct_targets = { 
            BuildPair(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::BUILD_HATCHERY),                  // Build phase 1
            BuildPair(UNIT_TYPEID::ZERG_SPAWNINGPOOL, ABILITY_ID::BUILD_SPAWNINGPOOL),          // 2
            BuildPair(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::BUILD_HATCHERY),                  // 3
            BuildPair(UNIT_TYPEID::ZERG_ROACHWARREN, ABILITY_ID::BUILD_ROACHWARREN),            // 4
            BuildPair(UNIT_TYPEID::ZERG_LAIR, ABILITY_ID::MORPH_LAIR),                          // 5
            BuildPair(UNIT_TYPEID::ZERG_SPIRE, ABILITY_ID::BUILD_SPIRE),                        // 6
            BuildPair(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::BUILD_HATCHERY),                  // 7
            BuildPair(UNIT_TYPEID::ZERG_INFESTATIONPIT, ABILITY_ID::BUILD_INFESTATIONPIT),      // 8
            BuildPair(UNIT_TYPEID::ZERG_HIVE, ABILITY_ID::MORPH_HIVE),                          // 9
            BuildPair(UNIT_TYPEID::ZERG_ULTRALISKCAVERN, ABILITY_ID::BUILD_ULTRALISKCAVERN)} ;  // 10
        
        // Filter for getting units built
        const std::vector<UNIT_TYPEID> struct_units = {
            UNIT_TYPEID::ZERG_HATCHERY,
            UNIT_TYPEID::ZERG_SPAWNINGPOOL,
            UNIT_TYPEID::ZERG_HATCHERY, 
            UNIT_TYPEID::ZERG_ROACHWARREN, 
            UNIT_TYPEID::ZERG_LAIR, 
            UNIT_TYPEID::ZERG_SPIRE, 
            UNIT_TYPEID::ZERG_INFESTATIONPIT, 
            UNIT_TYPEID::ZERG_HIVE, 
            UNIT_TYPEID::ZERG_ULTRALISKCAVERN} ;

};
#endif
