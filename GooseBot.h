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

// Unit shorthands
#define larva UNIT_TYPEID::ZERG_LARVA
#define drone UNIT_TYPEID::ZERG_DRONE
#define zergl UNIT_TYPEID::ZERG_ZERGLING
#define banel UNIT_TYPEID::ZERG_BANELING
#define overl UNIT_TYPEID::ZERG_OVERLORD
#define queen UNIT_TYPEID::ZERG_QUEEN
#define roach UNIT_TYPEID::ZERG_ROACH
#define mutal UNIT_TYPEID::ZERG_MUTALISK
#define ultra UNIT_TYPEID::ZERG_ULTRALISK

// Base indicators
#define hatch UNIT_TYPEID::ZERG_HATCHERY
#define lair UNIT_TYPEID::ZERG_LAIR
#define hive UNIT_TYPEID::ZERG_HIVE
#define commc UNIT_TYPEID::TERRAN_COMMANDCENTER
#define orbcomm UNIT_TYPEID::TERRAN_ORBITALCOMMAND
#define orbcommf UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING
#define nexus UNIT_TYPEID::PROTOSS_NEXUS
#define gate UNIT_TYPEID::PROTOSS_GATEWAY
#define pylon UNIT_TYPEID::PROTOSS_PYLON

// Filter for finding idle larva
struct IsIdleLarva {
    IsIdleLarva(){};
    bool operator()(const Unit& unit) { return (unit.unit_type == UNIT_TYPEID::ZERG_LARVA && unit.orders.empty()); };
};

// Bot Class
class GooseBot : public sc2::Agent {

    public:
        // Constructor
        GooseBot() = default;

        // Overrides from basic api calls

        // Called when game starts, executes pre-game calculations
        virtual void OnGameStart();
        // Called each game step, manages bot actions
        virtual void OnStep();
        // Called when a unit isn't doing anything, handles default actions for unit types
        virtual void OnUnitIdle(const Unit* unit) final;
        // Called when a unit is destroyed, handles loss based on unit type
        virtual void OnUnitDestroyed(const Unit* unit) final;
        // Called when another unit comes into our range of vision, handles reactions
        virtual void OnUnitEnterVision(const Unit* unit) final;
        // Called when the game finishes, shows game results
        virtual void OnGameEnd();
        // Called when a unit is created, handles desired concurrent actions
        virtual void OnUnitCreated(const Unit* unit);
        // Called when a building is completed, handles drone loss by attempting to morph new drone
        virtual void OnBuildingConstructionComplete(const Unit* unit);
        // Called when an upgrade completes, used to track the bot's upgraded vector
        virtual void OnUpgradeCompleted(UPGRADE_ID);

        // Functions to build structures
        // returns success/failure based on pre-conditions

        // Morph a structure on top of another unit, also used for building upgrades
        bool TryMorphStructure(ABILITY_ID ability_type_for_structure,Tag location_tag, UNIT_TYPEID worker_unit = UNIT_TYPEID::ZERG_DRONE);
        // Morph a structure at a point, used to expand based on scouting information
        bool TryMorphStructure(ABILITY_ID ability_type_for_structure, const Point2D& location_point, UNIT_TYPEID worker_unit = UNIT_TYPEID::ZERG_DRONE);
	    // Morph an extractor, uses TryMorphStructure w/tag after pre-condition checks
        bool TryMorphExtractor();
        // Morph a lair from one of the hatcheries
        bool TryMorphLair();
        // Morph a hive from the lair
        bool TryMorphHive();
        // Build a structure at a random point close to a randomly selected builder unit
        bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID worker_type = UNIT_TYPEID::ZERG_DRONE, size_t struct_cap = 1);
	    // Build an expansion at a scouted potential base location
        bool TryBuildHatchery();
        // Handles the order of structures to be built, tries to build next structure
        bool BuildPhase();
        // Updates num_bases
        void HandleBases(const ObservationInterface* observation);


        // Research an upgrade
        bool TryResearch(UNIT_TYPEID researcher_type, ABILITY_ID ability, UPGRADE_ID upgrade);
        
        // Tries to birth a queen from a base unit
        bool TryBirthQueen();

        // Checks whether any of our units have already been given the order to do the given action
        bool actionPending(ABILITY_ID action);
        
        // Returns the number of units allied to the bot of the given type
        size_t CountUnitType(UNIT_TYPEID unit_type);

        // Returns a pointer to a random unit allied to the bot of the given type
        const Unit* FindUnit(UNIT_TYPEID unit_type);

        // Return a pointer to the bot's most upgraded base
        const Unit* GetMainBase();
        // Return a pointer to one of the bot's least upgraded bases
        const Unit* GetNewerBase();

        // Returns whether the bot has enough vespene and minerals to create a unit of the given type
        bool CanAfford(UNIT_TYPEID unit);
        // Returns whether the bot has enough vespene and minerals to research the given upgrade
        bool CanAfford(UPGRADE_ID upgrade);
        // Sends a unit to scout possible enemy locations
        void scout(const Unit* unit);
        // Sends a unit to scout a given point
        void scoutPoint(const Unit* unit, Point2D point);

        // Decides max number of queens needed
        void SetQueenCap();

        // Handles Queens and sending waves of units depending on built structures
        bool ArmyPhase();

        // Handles order of upgrades, researches next upgrade if available
        bool ResearchPhase();
        // Checks whether the bot has completed an upgrade already
        bool IsUpgraded(UPGRADE_ID upgrade);
        // Checks whether the bot possesses a desired structure
        // --Only works for structures filtered by struct_filter
        bool IsBuilt(UNIT_TYPEID unit);

        // Iterates over the bot's units' actions to update pendingOrders
        void VerifyPending(const ObservationInterface* observation);
        // Clears and re-fills bot's army and melee trackers with attack units
        void VerifyArmy();
        // Sets army unit caps based on built structures
        void VerifyArmyFocus();
        // Verifies which structures the bot currently has
        void VerifyBuild();        

        // Return true if two units are within a certain distance of each other
        bool UnitsWithinProximity(float proximity, const Unit& unit1, const Unit& unit2) const;

        // Return a vector of potential base building spots based on map positionings
        const std::vector<Point2D> FindBaseBuildingGrounds();
        // Return a vector of all neutral mineral patches
        const Units FindAllMineralPatches();
        // Return a pointer to the closest neutral mineral patch to a given point
        const Unit* FindNearestMineralPatch(const Point2D& start);
        // Return a pointer to the closest available vespene geyser to a given point
        const Unit* FindNearestVespeneGeyser(const Point2D& start);
        // Return a pointer to the nearest allied unit type to a given point
        const Unit* FindNearestAllied(UNIT_TYPEID target_unit, const Point2D& start);
        // Return a pointer to the nearest allied unit that has any of the given unit types, to a given point 
        const Unit* FindNearestAllied(std::vector<UNIT_TYPEID> target_units, const Point2D& start);
        // Return a pointer to the nearest enemy unit to a given point
        const Unit* FindNearestEnemy(const Point2D& start);

        // Optimize vespene worker counts
        bool GooseBot::TryHarvestVespene();
        // Determine whether we have enough units to send a wave
        // Doesn't send a wave if one is in progress
        bool GooseBot::ArmyReady();


    private:

        // vector of pointers to structures the bot has built
        Units built_structs;
        size_t num_bases;
        size_t num_desired_bases;
        size_t num_desired_extractors;

        // Initial
        size_t drone_cap = 14;
        size_t queen_cap;
        size_t overlord_cap = 1;
        size_t zergl_cap;
        size_t roach_cap;
        size_t mutal_cap;
        size_t banel_cap;
        size_t ultra_cap;

        std::unordered_set<ABILITY_ID> pendingOrders;

        std::vector<UPGRADE_ID> upgraded;

        size_t build_phase = 1;
        bool saving_for_building; 
        std::vector<UNIT_TYPEID> built_types;

        //size_t army_phase = 1;
        bool saving_for_army;

        size_t research_phase = 1;
        bool saving_for_research;
      
        UNIT_TYPEID target_struct;
        ABILITY_ID builder_ability;
        UNIT_TYPEID builder;
        size_t phase = 0;
        
        // Unit Filter Vectors for Observation.GetUnits()
        const std::vector<UNIT_TYPEID> vespeneTypes = { UNIT_TYPEID::NATURALGAS, UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_VESPENEGEYSER, UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER };
        const std::vector<UNIT_TYPEID> mineralTypes = { UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD, UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750, UNIT_TYPEID::NEUTRAL_MINERALFIELD, UNIT_TYPEID::NEUTRAL_MINERALFIELD450, UNIT_TYPEID::NEUTRAL_MINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750 };
        const std::vector<UNIT_TYPEID> resourceTypes = { UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD, UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750, UNIT_TYPEID::NEUTRAL_MINERALFIELD, UNIT_TYPEID::NEUTRAL_MINERALFIELD450, UNIT_TYPEID::NEUTRAL_MINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750, UNIT_TYPEID::NATURALGAS, UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_VESPENEGEYSER, UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER };
        const std::vector<UNIT_TYPEID> townHallTypes = { UNIT_TYPEID::ZERG_HATCHERY, UNIT_TYPEID::PROTOSS_NEXUS, UNIT_TYPEID::TERRAN_COMMANDCENTER };
        const std::vector<UNIT_TYPEID> baseTypes = { UNIT_TYPEID::ZERG_HATCHERY, UNIT_TYPEID::ZERG_LAIR, UNIT_TYPEID::ZERG_HIVE };


        // Stores possible places that bases can be built after FindBaseBuildingGrounds() is called in OnStart()
        const std::vector<Point2D> &possibleBaseGrounds = {};
        std::vector<Point2D> enemyStartLocations;


        Units army;
        Units defense;
        Units melee;
        size_t melee_cap = 6;
        size_t army_cap = 20;
        Units enemy_base;
        Point2D last_base = Point2D(0,0);
        bool EnemyLocated = false;


        std::vector<std::pair<int, const Unit*>> generalScouts = {};
        std::vector<std::pair<int, const Unit*>> suicideScouts = {};

        using BuildPair = std::pair<UNIT_TYPEID, ABILITY_ID>;
        const std::vector<BuildPair> struct_targets = { 
            BuildPair(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::BUILD_HATCHERY),                  // Build phase 1
            BuildPair(UNIT_TYPEID::ZERG_SPAWNINGPOOL, ABILITY_ID::BUILD_SPAWNINGPOOL),          // 2
            //BuildPair(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::BUILD_HATCHERY),                  // 3
            BuildPair(UNIT_TYPEID::ZERG_ROACHWARREN, ABILITY_ID::BUILD_ROACHWARREN),            // 4
            BuildPair(UNIT_TYPEID::ZERG_LAIR, ABILITY_ID::MORPH_LAIR),                          // 5
            BuildPair(UNIT_TYPEID::ZERG_SPIRE, ABILITY_ID::BUILD_SPIRE),                        // 6
            //BuildPair(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::BUILD_HATCHERY)
            } ;                // 7
        
        const std::vector<UNIT_TYPEID> struct_filter = {
            UNIT_TYPEID::ZERG_HATCHERY,
            UNIT_TYPEID::ZERG_SPAWNINGPOOL,
            //UNIT_TYPEID::ZERG_HATCHERY, 
            UNIT_TYPEID::ZERG_ROACHWARREN, 
            UNIT_TYPEID::ZERG_LAIR, 
            UNIT_TYPEID::ZERG_SPIRE//,
            //UNIT_TYPEID::ZERG_HATCHERY
            } ;

};
#endif
