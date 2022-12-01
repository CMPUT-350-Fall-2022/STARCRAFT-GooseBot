#include "GooseBot.h"

#define larva UNIT_TYPEID::ZERG_LARVA
#define drone UNIT_TYPEID::ZERG_DRONE
#define zergl UNIT_TYPEID::ZERG_ZERGLING
#define banel UNIT_TYPEID::ZERG_BANELING
#define overl UNIT_TYPEID::ZERG_OVERLORD
#define queen UNIT_TYPEID::ZERG_QUEEN
#define roach UNIT_TYPEID::ZERG_ROACH
#define mutal UNIT_TYPEID::ZERG_MUTALISK


using namespace sc2;

void GooseBot::OnGameStart() { 
    const ObservationInterface* observation = Observation();
    army.reserve(100);
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
    // Make sure pendingOrders are current
    VerifyPending();
    CountBases();
    Prioritize();

    if (saving_for_building){
        if (BuildPhase()){
            std::cout << "Built structure for phase " << build_phase << std::endl;
            return;
        }        
    }
    if (saving_for_army){ 
        if (ArmyPhase()){
            std::cout << "Progressing in army phase " << army_phase << std::endl;
            return;
        }
    }
    if (ResearchPhase()){
        return;
    }
    if (TryMorphExtractor()) {
        std::cout << "Morphed Extractor" << std::endl;
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
    size_t drone_count = CountUnitType(drone);
    size_t zergl_count = CountUnitType(zergl);
    size_t banel_count = CountUnitType(banel);
    size_t roach_count = CountUnitType(roach);
    size_t mutal_count = CountUnitType(mutal);

    //for our unit pointer, we do a switch-case dependent on it's type
    switch (unit->unit_type.ToType())
    {
        //case UNIT_TYPEID::ZERG_LARVA:
    case larva:
    {
        //while our supply limit is less than or equal to our supply limit cap - 1      Note: changed to if because i can't see why we need a while in a callback, also, was probably causing unexpected behavior with the breaks. change this back if it was actually needed.
        if (observation->GetFoodUsed() <= observation->GetFoodCap() - 1)
        {   //if our total number of workers is less than 30
            if ((drone_count <= drone_cap))     
            {   //build a worker
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
                break;
            }
            if (saving_for_army){
                //if our zergling count is less than or equal to 10
                if (zergl_count <= 10)
                {   //try to train a zergling (this can't be done unless there is an existing spawning pool
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
                    break;
                }

                if (roach_count <= 5)
                {
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZERGLING);
                    break;
                }

                if (mutal_count < zergl_count)
                {
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MUTALISK);
                    break;
                }
            }
            //TODO: I feel like a break was probably intended to be here, but i'm not sure, someone decide.
        }

        //spawns overlord to increase supply cap when we need supply increase
        if (CountUnitType(UNIT_TYPEID::ZERG_OVERLORD) < overlord_cap)
        {
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
        }
        break;
    }

    case drone:
    {
        const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
        if (!mineral_target)
        {
            break;
        }
        Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
        break;
    }

    case overl:
    {
        scout(unit);
        break;
    }

        case queen:
        {
            const Unit * hatchery = FindNearestAllied(UNIT_TYPEID::ZERG_HATCHERY, unit->pos);
            //iterator pointing to buff if found, end if not found
            auto hasInjection = std::find(hatchery->buffs.begin(), hatchery->buffs.end(), BUFF_ID::QUEENSPAWNLARVATIMER);
            if (hasInjection == hatchery->buffs.end()){     //if no injection
                Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, hatchery);
            }else{
                TryBuildStructure(ABILITY_ID::BUILD_CREEPTUMOR, UNIT_TYPEID::ZERG_CREEPTUMOR, unit->unit_type, 10);
            }
            break;
        }

    case zergl:
    {
        if (banel_count < zergl_count) {
            Actions()->UnitCommand(unit, ABILITY_ID::MORPH_BANELING);

        }
    }

    case banel:
    case mutal:
    case roach: 
    {
        army.push_back(unit);
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
        || (actionPending(ABILITY_ID::TRAIN_QUEEN))
        || (CountUnitType(UNIT_TYPEID::ZERG_QUEEN) >= queen_cap)  ){
        return false;
    }
    const Unit * base = GetNewerBase();
    if (base != nullptr){
        Actions()->UnitCommand(base, ABILITY_ID::TRAIN_QUEEN);
        return true;
    }else{
        return false;
    }
    return false;
}

Units GooseBot::getArmy() { return army; }

Point2D GooseBot::getEnemyLocation() { return enemy_base; }

// EFFECT_INJECTLARVA target hatchery/lair
// MORPH_LAIR no target

// MORPH_OVERLORDTRANSPORT no target
// BUFF_ID QUEENSPAWNLARVATIMER