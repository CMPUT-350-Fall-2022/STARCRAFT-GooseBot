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

void GooseBot::OnGameStart()
{ 
    possibleBaseGrounds = FindBaseBuildingGrounds();
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
    if (TryBuildStructure(abilities[phase], targetStruct[phase])) {
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
    if (TryBuildStructure(ABILITY_ID::BUILD_BANELINGNEST, UNIT_TYPEID::ZERG_BANELINGNEST)) {
        return;
    }
    // TODO: Change to get scouted point from ideal location for new base ///
    const GameInfo& game_info = Observation()->GetGameInfo();
    auto enemyStartLocations = game_info.enemy_start_locations;
    // Point2D demo_point = Point2D(0, 0);
    // if (enemyStartLocations.empty() == false) { 
      
    //     demo_point = enemyStartLocations.back();
        
    // }

    if (TryMorphStructure(ABILITY_ID::BUILD_HATCHERY, possibleBaseGrounds[0])) {
        return;
    }
    /////////////////////////////////////////////////////////////////////////


    if (ArmyReady() && EnemyLocated()) {
        Attack();       //TODO: Does this thing need a return after it like everything else? also, with all these returns, will the stuff towards the bottom actually be reachable?
    }   
}


// In your bot class.
void GooseBot::OnUnitIdle(const Unit* unit) {
    //get the current game state observation
    const ObservationInterface* observation = Observation();
    size_t drone_count = countUnitType(drone);
    size_t zergl_count = countUnitType(zergl);
    size_t banel_count = countUnitType(banel);
    size_t roach_count = countUnitType(roach);
    size_t mutal_count = countUnitType(mutal);

    //for our unit pointer, we do a switch-case dependent on it's type
    switch (unit->unit_type.ToType())
    {
        //case UNIT_TYPEID::ZERG_LARVA:
        case larva:
        {
            //while our supply limit is less than or equal to our supply limit cap - 1      Note: changed to if because i can't see why we need a while in a callback, also, was probably causing unexpected behavior with the breaks. change this back if it was actually needed.
            if (observation->GetFoodUsed() <= observation->GetFoodCap() - 1)
            {   //if our total number of workers is less than 30
                if ((drone_count <= 16 - 2))      //TODO: change this limit to rely on our number of hatcheries
                {   //build a worker
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_DRONE);
                    break;
                }

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
                //TODO: I feel like a break was probably intended to be here, but i'm not sure, someone decide.
            }
            
            //spawns overlord to increase supply cap when we have only one available opening
            if (countUnitType(UNIT_TYPEID::ZERG_OVERLORD) < overlordCap[phase])
            {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_OVERLORD);
            }
            break;
        }

		case drone:
        {
            const Unit * mineral_target = FindNearestMineralPatch(unit->pos);
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
            const Unit* hatchery = FindNearestAllied(UNIT_TYPEID::ZERG_HATCHERY, unit->pos);
            //iterator pointing to buff if found, end if not found
            if (hatchery)
            {
                auto hasInjection = std::find(hatchery->buffs.begin(), hatchery->buffs.end(), BUFF_ID::QUEENSPAWNLARVATIMER);
                if (hasInjection == hatchery->buffs.end())
                {     //if no injection
                    Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_INJECTLARVA, hatchery);
                    break;
                }
            }
            TryBuildStructure(ABILITY_ID::BUILD_CREEPTUMOR, UNIT_TYPEID::ZERG_CREEPTUMOR, unit->unit_type, tumorCap[phase]);
            break;
        }

        case zergl:
        {
            if (banel_count < zergl_count) {
                    Actions()->UnitCommand(unit, ABILITY_ID::MORPH_BANELING);
                    break;
                }
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
    // auto enemyStartLocations = FindBaseBuildingGrounds();
    Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_PATROL, GetRandomEntry(enemyStartLocations));
}


void GooseBot::AppendBases(Units& units, Unit::Alliance alliance, UNIT_TYPEID id)
{
    const ObservationInterface* observation = Observation();
    Units bases = observation->GetUnits(alliance, IsUnit(id));
    units.insert(units.end(), bases.begin(), bases.end());
}


const std::vector<Point2D> GooseBot::FindBaseBuildingGrounds()
{
    // Finding possible places to build base:
    /* 1. find all mineral patches
    2. exclude patches within a certain distance of a town hall (e.g hatchery/command center), may require sending scouts to places on the map to see things
    3. query that we can build a town hall at each location, try a few times with slightly differnt coordinate offsets from the each mineral patch  //TODO:
    4. if all good, add to list of possible locations, make sure sorted by nearest. //TODO:
    */
    // Units units = FindAllMineralPatches();
    // // const ObservationInterface* observation = Observation();
    // // Units bases = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY));
    // // AppendBases(bases, Unit::Alliance::Ally, UNIT_TYPEID::ZERG_HATCHERY);
    // // AppendBases(bases, Unit::Alliance::Ally, UNIT_TYPEID::TERRAN_COMMANDCENTER);
    // // AppendBases(bases, Unit::Alliance::Ally, UNIT_TYPEID::PROTOSS_NEXUS);
    // // AppendBases(bases, Unit::Alliance::Enemy, UNIT_TYPEID::ZERG_HATCHERY);
    // // AppendBases(bases, Unit::Alliance::Enemy, UNIT_TYPEID::TERRAN_COMMANDCENTER);
    // // AppendBases(bases, Unit::Alliance::Enemy, UNIT_TYPEID::PROTOSS_NEXUS);
    // std::vector<Point2D> grounds = {};
    // for (auto &patch : units)
    // {   
    //     // for (const auto &base : bases)
    //     // {
    //     //     if (!UnitsWithinProximity(15.f, (*patch), (*base)))
    //     //     {
    //     //         Point2D groundPos = patch->pos;
    //     //         grounds.push_back(groundPos);
    //     //         // Point2D groundPos(patch->pos.x + GetRandomScalar() * 5,
    //     //         //                   patch->pos.y + GetRandomScalar() * 5);    //TODO: Might give a bad spot.

    //     //     }
    //     // }
    // }
    // return grounds;


    // Initialize each centroid to be the position of a geyser in a pair of 2 closests geysers, discarding the other (there are 2 geysers per base location)
    // geysers = GetUnits(neutral, IsUnits(geysersTypes))
    const ObservationInterface* observation = Observation();
    auto geysers = observation->GetUnits(Unit::Alliance::Neutral, IsUnits(vespeneTypes));
    // centroids = {};
    std::vector<Point2D> centroids = {};
    // while geysers.size > 1
    while (geysers.size() > 1)
    {//     geyser = geysers.pop()
        auto geyser = geysers.back();
        geysers.pop_back();     // FIXME: Might make geyser above point to something invalid
    //     centroids.push(geyser.pos)
        centroids.push_back(geyser->pos);
    //     minDist = Distance2D(geyser.pos, geysers[0].pos)
        auto minDist = Distance2D(geyser->pos, geysers[0]->pos);
    //     closest_i = 0
        auto closest_i = 0;
    //     for i = 1; i < geysers.size; i++
        for (auto i = 0;  i < geysers.size();  ++i)
        {//    dist = Distance2D(geyser.pos, geysers[i].pos)
            auto dist = Distance2D(geyser->pos, geysers[i]->pos);
    //         if dist < minDist
            if (dist < minDist)
            {//    minDist = dist
                minDist = dist;
    //             closest_i = i
                closest_i = i;
            }
        }
    //     geysers.erase(geysers.begin() + closest_i)
        geysers.erase(geysers.begin() + closest_i);
    }

    // Group minerals and vespene into the cluster with the closest centroid
    // resources = GetUnits(neutral, IsUnits(geysreTypes and mineralTypes))
    auto resources = observation->GetUnits(Unit::Alliance::Neutral, IsUnits(mineralTypes));
    // for i=0; i<3; i++
    for (auto i = 0;  i < 3;  ++i)
    {   // Initialize clusters
        // vector<pair<Point2D, vector<Unit>>> clusters = {};
        std::vector<std::pair<Point2D, std::vector<const Unit*>>> clusters = {};
        // for each centroid in centroids
        for (auto &centroid : centroids)
        {   // vector<Unit> bucket = {};
            std::vector<const Unit*> bucket = {};
            // clusters.push(make_pair(centroid, bucket))
            clusters.push_back(std::make_pair(centroid, bucket));
        }

        // Cluster minerals and vespene
        // for resource in resources
        for (auto &resource : resources)
        {   // cluster_i = 0
            auto cluster_i = 0;
            // minDist = Distance2D(clusters[0].first, resource.pos)
            auto minDist = Distance2D(clusters[0].first, resource->pos);
            // for j = 1; j < clusters.size; j++
            for (auto j = 1;  j < clusters.size();  ++j)
            {   // dist = Distance2D(clusters[j].first, resource.pos)
                auto dist = Distance2D(clusters[j].first, resource->pos);
                // if dist < minDist
                if (dist < minDist)
                {   // minDist = dist
                    minDist = dist;
                    // cluster_i = j
                    cluster_i = j;
                }
            }
            // clusters[cluster_i].second.push(resource)
            clusters[cluster_i].second.push_back(resource);
        }
        
        // On the 3rd pass, sort the centroids based on how suitable of a position they are for building a base
        // if i == 2
        if (i == 2)
        {   // sort(clusters.begin(), clusters.end(), [](pair<Point2D, vector<Unit>> cluster1, pair<Point2D, vector<Unit>> cluster2){
            std::sort(clusters.begin(), clusters.end(), [&observation](std::pair<Point2D, std::vector<const Unit*>> cluster1, std::pair<Point2D, std::vector<const Unit*>> cluster2)
            {   // c1Score = 0
                auto c1Score = 0;
                // c2Score = 0
                auto c2Score = 0;
                // for resource in cluster1.second
                for (auto &resource : cluster1.second)
                {   // switch resource.unit_type
                    switch (resource->unit_type.ToType())
                    {   // case: richVespene
                        case UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER:    // Intentionally falls through
                        // case: richMinerals
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD:     // Intentionally falls through
                        // case: otherRichMinerals
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750:      // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD:     // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750:
                            // c1Score += 2
                            c1Score += 2;
                            // break;
                            break;
                        // default:
                        default:
                            // c1Score++
                            c1Score++;
                            // break;
                            break;
                    }
                }
                // for resource in cluster2.second
                //     switch resource.unit_type
                //         case: richVespene
                //         case: richMinerals
                //         case: otherRichMinerals
                //             c2Score += 2
                //             break;
                //         default:
                //             c2Score++
                //             break;
                for (auto &resource : cluster2.second)
                {   // switch resource.unit_type
                    switch (resource->unit_type.ToType())
                    {
                        case UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER:    // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD:     // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750:  // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD:     // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750:
                            c2Score += 2;
                            break;
                        default:
                            c2Score++;
                            break;
                    }
                }
                // basePos = GetUnits(allianc::self, Hatchery)[0].pos
                auto basePos = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))[0]->pos;
                // c1Score /= Distance2D(cluster1.first, basePos)
                c1Score /= Distance2D(cluster1.first, basePos);
                // c2Score /= Distance2D(cluster2.first, basePos)
                c2Score /= Distance2D(cluster2.first, basePos);

                // return c1Score > c2Score
                return c1Score > c2Score;
            // });
            });
            // for j = 0; j < centroids.size; j++
            //     {centroids[j] = clusters[j].first}
            // break;
            for (auto j = 0;  j < centroids.size();  ++j)
            {
                centroids[j] = clusters[j].first;
            }
            break;
        }
        
        // before the 3rd pass, recalculate the centroids so that they are in the center of the cluster, rather than where the vespene is
        // for j = 0; j < centroids.size; j++
        //     clusterSize = clusters[j].second.size
        //     Point2D newCentroid
        //     for resource in clusters[j].second
        //         newCentroid += resource.pos
        //     newCentroid /= clusterSize
        //     centroids[j] = newCentroid
        for (auto j = 0;  j < centroids.size();  ++j)
        {
            auto clusterSize = clusters[j].second.size();
            Point2D newCentroid;
            for (auto &resource : clusters[j].second)
            {
                newCentroid += resource->pos;
            }
            newCentroid /= clusterSize;
            centroids[j] = newCentroid;
        }
    }

    // return centroids
    return centroids;
}


bool GooseBot::UnitsWithinProximity(float proximity, const Unit& unit1, const Unit& unit2) const
{
    Point2D unit1Pos = unit1.pos;
    Point2D unit2Pos = unit2.pos;
    return Distance2D(unit1Pos, unit2Pos) < proximity;
}


const Units GooseBot::FindAllMineralPatches()
{
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
    Units patches = {};
    for (const auto& u : units)
    {
        if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD)
        {
            patches.push_back(u);
        }        
    }
    return patches;
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
    Units units = FindAllMineralPatches();
    float distance = std::numeric_limits<float>::max();
    const Unit * target = nullptr;
    for (const auto& u : units)
    {
        float d = DistanceSquared2D(u->pos, start);
        if (d < distance)
        {
            distance = d;
            target = u;
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
    const Unit * base = GetMainBase();
    if ((countUnitType(UNIT_TYPEID::ZERG_QUEEN) < queenCap[phase]) && (base != nullptr)){
        Actions()->UnitCommand(base, ABILITY_ID::TRAIN_QUEEN);
        return true;
    }else{
        return false;
    }
    return false;
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
                if ( ((data.tech_requirement != UNIT_TYPEID::INVALID) && (countUnitType(data.tech_requirement) > 0))
                    || (data.tech_requirement == UNIT_TYPEID::INVALID) ){
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
    if (actionPending(ability)
        || (std::find(upgraded.begin(), upgraded.end(), upgrade) == upgraded.end())){
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