#include "GooseBot.h"

using namespace sc2;

void GooseBot::OnGameStart() { 
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
    VerifyPhase();
    VerifyPending();
    if (TryBuildStructure(abilities[phase], targetStruct[phase], builders[phase])) {
        std::cout << "Built structure for phase " << phase << std::endl;
        return;
    }
    if (TryMorphLair()){
        std::cout << "Morphed Lair" << std::endl;
        return;
    }
    if (TryBirthQueen()){
        std::cout << "Birthed Queen" << std::endl;
        return;
    }
    if (TryMorphExtractor()) {
        std::cout << "Morphed Extractor" << std::endl;
        return;
    }
    if (TryHarvestVespene()) {
        return;
    }
    if (TryResearch(UNIT_TYPEID::ZERG_HATCHERY, ABILITY_ID::RESEARCH_PNEUMATIZEDCARAPACE, UPGRADE_ID::OVERLORDSPEED)){
        std::cout << "Researched" << std::endl;
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
            }else{
                TryBuildStructure(ABILITY_ID::BUILD_CREEPTUMOR, UNIT_TYPEID::ZERG_CREEPTUMOR, unit->unit_type, tumorCap[phase]);
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

// Returns number of allied units of given type
size_t GooseBot::countUnitType(UNIT_TYPEID unit_type){
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

//returns random unit of given type
const Unit *GooseBot::FindUnit(UNIT_TYPEID unit_type){
    auto all_of_type = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
    if (all_of_type.size() != 0){
        return GetRandomEntry(all_of_type);
    }else{
        return nullptr;
    }
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
    if (   (!CanAfford(UNIT_TYPEID::ZERG_QUEEN))
        || (actionPending(ABILITY_ID::TRAIN_QUEEN))  ){
        return false;
    }
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
    if (countUnitType(UNIT_TYPEID::ZERG_QUEEN) < queenCap[phase]){
        Actions()->UnitCommand(bases[0], ABILITY_ID::TRAIN_QUEEN);
        return true;
    }else{
        return false;
    }
}

//Check if can afford unit
bool GooseBot::CanAfford(UNIT_TYPEID unit){
    const ObservationInterface* observation = Observation();
    int mineral_supply = observation->GetMinerals();
    int gas_supply = observation->GetVespene();
    auto const unit_data = observation->GetUnitTypeData();
    for (auto data : unit_data){
        if (data.unit_type_id == unit){ 
            if ( (mineral_supply >= data.mineral_cost) && (gas_supply >= data.vespene_cost)){
                if ( ((data.tech_requirement != 0) && (countUnitType(data.tech_requirement) > 0))
                    || (data.tech_requirement == 0) ){
                    return true;
                }
            }else{
                return false;
            }
        }
    }
    return false;
}

//Check if can afford upgrade
bool GooseBot::CanAfford(UPGRADE_ID upgrade){
    const ObservationInterface* observation = Observation();
    int mineral_supply = observation->GetMinerals();
    int gas_supply = observation->GetVespene();
    auto const upgrade_data = observation->GetUpgradeData();
    for (auto data : upgrade_data){
        if (data.upgrade_id == static_cast<uint32_t>(upgrade)){ 
            if ( (mineral_supply >= data.mineral_cost) && (gas_supply >= data.vespene_cost)){
                return true;
            }else{
                return false;
            }
        }
    }
    std::cout << "data does not contain the ability ";
    return false;
}

//Check if can do ability
bool GooseBot::Available(ABILITY_ID ability){
    const ObservationInterface* observation = Observation();
    auto const ability_data = observation->GetAbilityData();
    for (auto data : ability_data){
        if (data.ability_id == ability){ 
            if (data.available){
                return true;
            }else{
                return false;
            }
        }
    }
    std::cout << "ability not found ";
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
            }else{
                break;
            }
        }
    }
    phase = i;
}

bool GooseBot::TryResearch(UNIT_TYPEID researcher_type, ABILITY_ID ability, UPGRADE_ID upgrade){
    if (actionPending(ability) || !Available(ability)){
        return false;
    }
    const Unit* researcher = FindUnit(researcher_type);
    if (researcher != nullptr && CanAfford(upgrade)){
        Actions()->UnitCommand(researcher, ability);
        return true;
    }else{
        return false;
    }
}

// MORPH_OVERLORDTRANSPORT no target