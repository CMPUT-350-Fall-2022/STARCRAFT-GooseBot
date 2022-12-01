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
        bool TryMorphLair();
        bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID struct_type, UNIT_TYPEID worker_type = UNIT_TYPEID::ZERG_DRONE, size_t struct_cap = 1);
	    bool TryResearch(UNIT_TYPEID researcher_type, ABILITY_ID ability, UPGRADE_ID upgrade);
        
        bool actionPending(ABILITY_ID action);
        size_t countUnitType(UNIT_TYPEID unit_type);
        const Unit* FindUnit(UNIT_TYPEID unit_type);
        const Unit* GetMainBase();
        const Unit* GetNewerBase();

        bool CanAfford(UNIT_TYPEID unit);
        bool CanAfford(UPGRADE_ID upgrade);
	    void scout(const Unit* unit);
	    
       
        //bool EnemyLocated();
   

        void VerifyPhase();
        void VerifyPending();

        // Return true if two units are within a certain distance of each other
        bool UnitsWithinProximity(float proximity, const Unit& unit1, const Unit& unit2) const;

        void AppendBases(Units& units, Unit::Alliance alliance, UNIT_TYPEID id);
        const std::vector<Point2D> FindBaseBuildingGrounds();
        const Units FindAllMineralPatches();
        const Unit* FindNearestMineralPatch(const Point2D& start);
        const Unit* FindNearestAllied(UNIT_TYPEID target_unit, const Point2D& start);

        bool GooseBot::TryHarvestVespene();

        bool ArmyReady();
        Units getArmy();
        Point2D getEnemyLocation();

    private:
        enum PHASE {SPAWN, ZERGLINGS, ROACHES, END};

        using UnitList = std::array<UNIT_TYPEID, END>;

        std::unordered_set<ABILITY_ID> pendingOrders;

        std::vector<UPGRADE_ID> upgraded;

        size_t phase = 0;
        const std::array<size_t, END> overlordCap = {2, 3, 5};
        const std::array<size_t, END> queenCap = {1, 2, 2};
        const std::array<size_t, END> tumorCap = {1, 2, 3};
        
    
        // 3rd set dummies for moment
        const UnitList targetStruct = {UNIT_TYPEID::ZERG_SPAWNINGPOOL, UNIT_TYPEID::ZERG_ROACHWARREN, UNIT_TYPEID::ZERG_BROODLORD};
        const std::array<ABILITY_ID, END> abilities = {ABILITY_ID::BUILD_SPAWNINGPOOL, ABILITY_ID::BUILD_ROACHWARREN, ABILITY_ID::TEMPLEDOORDOWN};

        // Unit Filter Vectors for Observation.GetUnits()
        const std::vector<UNIT_TYPEID> vespeneTypes = { UNIT_TYPEID::NATURALGAS, UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_VESPENEGEYSER, UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER, UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER };
        const std::vector<UNIT_TYPEID> mineralTypes = { UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD, UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD, UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750, UNIT_TYPEID::NEUTRAL_MINERALFIELD, UNIT_TYPEID::NEUTRAL_MINERALFIELD450, UNIT_TYPEID::NEUTRAL_MINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD, UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750 };
        const std::vector<UNIT_TYPEID> townHallTypes = { UNIT_TYPEID::ZERG_HATCHERY, UNIT_TYPEID::PROTOSS_NEXUS, UNIT_TYPEID::TERRAN_COMMANDCENTER };

        // Stores possible places that bases can be built after FindBaseBuildingGrounds() is called in OnStart()
        std::vector<Point2D> possibleBaseGrounds;


        Units army;
        Point2D enemy_base;
        bool EnemyLocated = false;

};
#endif
