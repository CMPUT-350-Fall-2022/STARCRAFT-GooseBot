#include "GooseBot.h"

using namespace sc2;

void GooseBot::OnGameStart() { 
    const ObservationInterface* observation = Observation();

    return; 
}


void GooseBot::OnGameEnd()
{
    const ObservationInterface* observation = Observation();
    GameInfo gameInfo = observation->GetGameInfo();
    auto players = std::map<uint32_t, PlayerInfo*>();
    for (auto &playerInfo : gameInfo.player_info)
    {

        players[playerInfo.player_id] = &playerInfo;
    }
    
    auto playerTypes = std::map<PlayerType, std::string>();
    playerTypes[Participant] = "Goose";
    playerTypes[Computer] = "Prey";
    playerTypes[Observer] = "Observer";

    auto gameResults = std::map<GameResult, std::string>();
    gameResults[Win] = " Wins";
    gameResults[Loss] = " Loses";
    gameResults[Tie] = " Tied";
    gameResults[Undecided] = " Undecided";

    for (auto &playerResult : observation->GetResults())
    {
        std::cout << playerTypes[((*(players[playerResult.player_id])).player_type)] 
                  << gameResults[playerResult.result]
                  << std::endl;
    }
}


void GooseBot::OnStep() { 

    if (TryBuildStructure(abilities[phase], targetStruct[phase], builders[phase])) {
        VerifyPhase();
        return;
    }
    if (TryMorphStructure(abilities[phase], FindUnitTag(builders[phase]), builders[phase])){
        VerifyPhase();
        return;
    }
    if (TryBirthQueen()){
        return;
    }
    if (TryMorphExtractor()) {
        return;
    }
    if (TryHarvestVespene()) {
        return;
    }


    
}


// In your bot class.
void GooseBot::OnUnitIdle(const Unit* unit) {
    //get the current game state observation
    const ObservationInterface* observation = Observation();

    //for our unit pointer, we do a switch-case dependent on it's type
	switch (unit->unit_type.ToType())
    {
        //if the unit is a larva unit
		case UNIT_TYPEID::ZERG_LARVA:
        {
            //while our supply limit is less than or equal to our supply limit cap - 1
			while (observation->GetFoodUsed() <= observation->GetFoodCap() - 1)
            {
                //if our total number of workers is less than 30
                if ((observation->GetFoodWorkers() <= 16 - 2)) {        //TODO: change this limit to rely on our number of hatcheries
                    //build a worker
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
				break;
			}

            //if our army count is less than or equal to 10
                if (observation->GetFoodArmy() <= 10) {
                    //try to train a zergling (this can't be done unless there is an existing spawning pool
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
                    break;
                }
			}
            
            //spawns overlord to increase supply cap when we have only one available opening
			if (countUnitType(UNIT_TYPEID::ZERG_OVERLORD) < overlordCap[phase]){
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
            }
            
		}

		case UNIT_TYPEID::ZERG_DRONE:
        {

			const Unit * mineral_target = FindNearestMineralPatch(unit->pos);
			if (!mineral_target)
            {
				break;
			}
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
			break;
		}

        case UNIT_TYPEID::ZERG_OVERLORD:
        {
            scout(unit);
            break;
        }

        case UNIT_TYPEID::ZERG_QUEEN:
        {
            const Unit * hatchery = FindNearestAllied(UNIT_TYPEID::ZERG_HATCHERY, unit->pos);
            //iterator pointing to buff if found, end if not found
            auto hasInjection = std::find(hatchery->buffs.begin(), hatchery->buffs.end(), BUFF_ID::QUEENSPAWNLARVATIMER);
            if (hasInjection == hatchery->buffs.end()){     //if no injection
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, hatchery);
            }
            break;
        }
        
		default:
			break;
	}
}


// Very simple for now.
void GooseBot::scout(const Unit* unit)
{
    const GameInfo& game_info = Observation()->GetGameInfo();
    auto enemyStartLocations = game_info.enemy_start_locations;
    Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_PATROL, GetRandomEntry(enemyStartLocations));
}

const Unit* GooseBot::FindNearestAllied(UNIT_TYPEID target_unit, const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;

    for (const auto& u : units)
    {

        if (u->unit_type == target_unit)
        {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance)
            {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}



const Unit* GooseBot::FindNearestMineralPatch(const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const Unit * target = nullptr;
	for (const auto& u : units)
    {
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD)
        {
            float d = DistanceSquared2D(u->pos, start);
			if (d < distance)
            {
				distance = d;
				target = u;
			}			
		}		
	}
	return target;
}


size_t GooseBot::countUnitType(UNIT_TYPEID unit_type)
{
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

//returns position tag of random unit of given type
Tag GooseBot::FindUnitTag(UNIT_TYPEID unit_type){
    auto all_of_type = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
    const Unit * unit = GetRandomEntry(all_of_type);
    return unit->tag;
}

bool GooseBot::TryHarvestVespene() {
    Units workers = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_DRONE));
    
    if (workers.empty()) {
        return false;
    }


    const Unit* unit = GetRandomEntry(workers);
    const Unit* vespene_target = FindNearestAllied(UNIT_TYPEID::ZERG_EXTRACTOR, unit->pos);

    if (!vespene_target)
    {
        return false;
    }
    if (vespene_target->build_progress != 1) {
        return false;
    }

    if (vespene_target->assigned_harvesters>=vespene_target->ideal_harvesters) {//change this to not be hardcoded
        return false;
    }
    
    Actions()->UnitCommand(unit, ABILITY_ID::SMART, vespene_target);
    return true;
    
    
}

bool GooseBot::TryBirthQueen(){
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
    if (countUnitType(UNIT_TYPEID::ZERG_QUEEN) < queenCap[phase]){
        for (const auto& order : bases[0]->orders) {
            if (order.ability_id == ABILITY_ID::TRAIN_QUEEN) {
                return false;
            }
        }
        Actions()->UnitCommand(bases[0], ABILITY_ID::TRAIN_QUEEN);
        return true;
    }else{
        return false;
    }
}

bool GooseBot::CanAfford(UNIT_TYPEID unit){
    const ObservationInterface* observation = Observation();
    int mineral_supply = observation->GetMinerals();
    int gas_supply = observation->GetVespene();
    auto const unit_data = observation->GetUnitTypeData();
    for (auto data : unit_data){
        if (data.unit_type_id == unit){ 
            if ( (mineral_supply >= data.mineral_cost) && (gas_supply >= data.vespene_cost)){
                return true;
            }else{
                return false;
            }
        }
    }
    return false;

}

void GooseBot::VerifyPhase(){
    const ObservationInterface* observation = Observation();
    auto units = observation->GetUnits(Unit::Alliance::Self);
    size_t i = 0;
    for (auto unit : units){
        while (1){
            if (unit->unit_type == targetStruct[i]){
                ++i;
            }
            else{
                break;
            }
        }
    }
    phase = i;
}
// EFFECT_INJECTLARVA target hatchery/lair
// MORPH_LAIR no target

// MORPH_OVERLORDTRANSPORT no target
// BUFF_ID QUEENSPAWNLARVATIMER