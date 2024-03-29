#include "GooseBot.h"
//DEV BRANCH 
// Very simple for now.
void GooseBot::scout(const Unit* unit){
    Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_PATROL, /*possibleBaseGrounds[0]*/GetRandomEntry(enemyStartLocations));
}

// Go to a point 
void GooseBot::scoutPoint(const Unit* unit, Point2D point){
    Actions()->UnitCommand(unit, ABILITY_ID::GENERAL_MOVE, point);
}

// returns random unit of given type
const Unit *GooseBot::FindUnit(UNIT_TYPEID unit_type){
    auto all_of_type = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type));
    if (all_of_type.size() != 0){
        return GetRandomEntry(all_of_type);
    }else{
        return nullptr;
    }
}

// returns whether a given unit is within a given proximity to another unit
bool GooseBot::UnitsWithinProximity(float proximity, const Unit& unit1, const Unit& unit2) const
{
    Point2D unit1Pos = unit1.pos;
    Point2D unit2Pos = unit2.pos;
    return Distance2D(unit1Pos, unit2Pos) < proximity;
}

// return all mineral patches
const Units GooseBot::FindAllMineralPatches(){
    return Observation()->GetUnits(Unit::Alliance::Neutral, IsUnits(mineralTypes));
}

// returns pointer to nearest mineral patch
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

// FOR MULTI UNIT TYPES- returns a pointer to the closest of a selection of allied unit types
const Unit* GooseBot::FindNearestAllied(std::vector<UNIT_TYPEID> target_units, const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Self, IsUnits(target_units));
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units)
    {
        if (std::find(target_units.begin(), target_units.end(), u->unit_type) != target_units.end())
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

// FOR SINGLE UNIT TYPE - returns a pointer to the nearest allied unit type
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

// Only run in OnStart! 
const std::vector<Point2D> GooseBot::FindBaseBuildingGrounds()
{
    // Initialize each centroid to be the position of a geyser in a pair of 2 closests geysers, discarding the other (there are 2 geysers per base location)
    const ObservationInterface* observation = Observation();
    auto geysers = observation->GetUnits(Unit::Alliance::Neutral, IsUnits(vespeneTypes));
    std::vector<Point2D> centroids = {};
    while (geysers.size() > 1)
    {
        auto geyser = geysers.back();
        geysers.pop_back();
        centroids.push_back(geyser->pos);
        auto minDist = Distance2D(geyser->pos, geysers[0]->pos);
        auto closest_i = 0;
        for (auto i = 0;  i < geysers.size();  ++i)
        {
            auto dist = Distance2D(geyser->pos, geysers[i]->pos);
            if (dist < minDist)
            {
                minDist = dist;
                closest_i = i;
            }
        }
        geysers.erase(geysers.begin() + closest_i);
    }

    // Group minerals and vespene into the cluster with the closest centroid
    auto resources = observation->GetUnits(Unit::Alliance::Neutral, IsUnits(resourceTypes));
    for (auto i = 0;  i < 3;  ++i)
    {   
        // Initialize clusters
        std::vector<std::pair<Point2D, std::vector<const Unit*>>> clusters = {};
        for (auto &centroid : centroids)
        {
            std::vector<const Unit*> bucket = {};
            clusters.push_back(std::make_pair(centroid, bucket));
        }

        // Cluster minerals and vespene
        for (auto &resource : resources)
        {
            auto cluster_i = 0;
            auto minDist = Distance2D(clusters[0].first, resource->pos);
            for (auto j = 1;  j < clusters.size();  ++j)
            {
                auto dist = Distance2D(clusters[j].first, resource->pos);
                if (dist < minDist)
                {
                    minDist = dist;
                    cluster_i = j;
                }
            }
            clusters[cluster_i].second.push_back(resource);
        }
        
        // On the 3rd pass, sort the centroids based on how suitable of a position they are for building a base
        if (i == 2)
        {
            // For each cluster, find the two minerals/vespene points that are furthest from the current centroid and each other
            for (auto &cluster : clusters)
            {
                auto resource_i = 0;
                auto maxDist_i = Distance2D(cluster.second[0]->pos, cluster.first);
                for (auto j = 1;  j < cluster.second.size();  ++j)
                {
                    auto &resource = cluster.second[j];
                    auto dist = Distance2D(resource->pos, cluster.first);
                    if (dist > maxDist_i)
                    {
                        resource_i = j;
                        maxDist_i = dist;
                    }
                }

                auto resource_j = 0;
                auto maxDist_j = Distance2D(cluster.second[0]->pos, cluster.second[resource_i]->pos);
                for (auto j = 1;  j < cluster.second.size();  ++j)
                {
                    auto &resource = cluster.second[j];
                    auto dist = Distance2D(resource->pos, cluster.second[resource_i]->pos);
                    if (dist > maxDist_j)
                    {
                        resource_j = j;
                        maxDist_j = dist;
                    }
                }
                auto r1r2 = cluster.second[resource_j]->pos - cluster.second[resource_i]->pos;
                auto half_r1r2_dist = Distance3D(cluster.second[resource_j]->pos, cluster.second[resource_i]->pos) / 2;
                Normalize3D(r1r2);
                auto almostBetterCenter = (r1r2 * half_r1r2_dist) + cluster.second[resource_i]->pos;
                cluster.first = almostBetterCenter - cluster.first;
                Normalize2D(cluster.first);
                cluster.first = (cluster.first * 4) + almostBetterCenter;
            }

            std::sort(clusters.begin(), clusters.end(), [&observation](std::pair<Point2D, std::vector<const Unit*>> cluster1, std::pair<Point2D, std::vector<const Unit*>> cluster2)
            {
                auto c1Score = 0;
                auto c2Score = 0;
                for (auto &resource : cluster1.second)
                {
                    switch (resource->unit_type.ToType())
                    {
                        case UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER:    // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD:     // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750:      // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD:     // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750:
                            c1Score += 20;
                            break;
                        default:
                            c1Score += 10;
                            break;
                    }
                }
                for (auto &resource : cluster2.second)
                {
                    switch (resource->unit_type.ToType())
                    {
                        case UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER:    // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD:     // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750:  // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD:     // Intentionally falls through
                        case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750:
                            c2Score += 20;
                            break;
                        default:
                            c2Score += 10;
                            break;
                    }
                }
                auto basePos = observation->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::ZERG_HATCHERY))[0]->pos;     //TODO: Figure out if it'd be better to use GetNewerBase() from Building.cpp here
                c1Score -= Distance2D(cluster1.first, basePos);
                c2Score -= Distance2D(cluster2.first, basePos);
                
                return c1Score > c2Score;
            });

            for (auto j = 0;  j < centroids.size();  ++j)
            {
                centroids[j] = clusters[j].first;
            }
            break;
        }
        
        // before the 3rd pass, recalculate the centroids so that they are in the center 
        // (ish, doesn't really seem to work well in practice at the moment) of the cluster, 
        // rather than where the vespene is
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
    return centroids;
}

const Unit* GooseBot::FindNearestVespeneGeyser(const Point2D& start){
    float max_distance = 15.0f;
    Units geysers = Observation()->GetUnits(Unit::Alliance::Neutral, IsUnits(vespeneTypes));
    const Unit* closestGeyser = nullptr;
	for (const auto& geyser : geysers) {
		float current_distance = Distance2D(start, geyser->pos);
		if (current_distance < max_distance) {
			if (Query()->Placement(ABILITY_ID::BUILD_EXTRACTOR, geyser->pos)) {
				max_distance = current_distance;
				closestGeyser = geyser;
			}
		}
	}
    return closestGeyser;
}

const Unit* GooseBot::FindNearestEnemy(const Point2D& start) {
    float max_distance = 45.0f;
    Units enemies = Observation()->GetUnits(Unit::Alliance::Enemy);
    const Unit* closestEnemy = nullptr;
    if (!enemies.empty()) {
        for (const auto& enemy : enemies) {
           
            float current_distance = Distance2D(start, enemy->pos);
            if (current_distance < max_distance && !enemy->is_flying) {
                closestEnemy = enemy;
            }
        }
    }
    return closestEnemy;
}