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

using namespace sc2;

class GooseBot : public sc2::Agent {

    public:
        GooseBot() = default;
        ~GooseBot() = default;

        virtual void OnGameStart();
        virtual void OnStep();
        virtual void OnUnitIdle(const sc2::Unit* unit) final;
        virtual void OnGameEnd();

        bool TryMorphStructure(ABILITY_ID ability_type_for_structure,Tag location_tag, UNIT_TYPEID unit_type = UNIT_TYPEID::ZERG_DRONE);
	    bool TryMorphExtractor();
        bool TryBuildSpawningPool();
	    size_t countUnitType(UNIT_TYPEID unit_type);
	    void scout(const Unit* unit);

	    const Unit* FindNearestMineralPatch(const Point2D& start);
        const Unit* FindNearestZergExtractor(const Point2D& start);

        bool GooseBot::TryHarvestVespene();
    private:

};
#endif