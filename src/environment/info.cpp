#include <environment/info.hpp>

#include <array>
#include <map>

// ==================================

std::string map_type_to_string(MapType type) {
    const static std::array<std::string, static_cast<uint32_t>(MapType::NUM)> map = {
            "RANDOM",
            "CITY",
            "GAME",
            "SORTATION",
            "WAREHOUSE",
    };
    return map.at(static_cast<uint32_t>(type));
}

MapType string_to_map_type(const std::string &str) {
    const static std::map<std::string, MapType> map = {
            {"RANDOM", MapType::RANDOM},
            {"CITY", MapType::CITY},
            {"GAME", MapType::GAME},
            {"SORTATION", MapType::SORTATION},
            {"WAREHOUSE", MapType::WAREHOUSE},
    };
    return map.at(str);
}

std::string planner_type_to_string(PlannerType type) {
    const static std::array<std::string, static_cast<uint32_t>(PlannerType::NUM)> map = {
            "PIBT",
            "CAUSAL_PIBT",
            "PIBT_TF",
            "EPIBT",
            "EPIBT_LNS",
            "EPIBT_LNS_OLD",
            "PEPIBT_LNS",
            "WPPL",
    };
    return map.at(static_cast<uint32_t>(type));
}

PlannerType string_to_planner_type(const std::string &str) {
    const static std::map<std::string, PlannerType> map = {
            {"PIBT", PlannerType::PIBT},
            {"CAUSAL_PIBT", PlannerType::CAUSAL_PIBT},
            {"PIBT_TF", PlannerType::PIBT_TF},
            {"EPIBT", PlannerType::EPIBT},
            {"EPIBT_LNS", PlannerType::EPIBT_LNS},
            {"EPIBT_LNS_OLD", PlannerType::EPIBT_LNS_OLD},
            {"PEPIBT_LNS", PlannerType::PEPIBT_LNS},
            {"WPPL", PlannerType::WPPL},
    };
    return map.at(str);
}

std::string scheduler_type_to_string(SchedulerType type) {
    const static std::array<std::string, static_cast<uint32_t>(SchedulerType::NUM)> map = {
            "CONST",
            "GREEDY",
    };
    return map.at(static_cast<uint32_t>(type));
}

SchedulerType string_to_scheduler_type(const std::string &str) {
    const static std::map<std::string, SchedulerType> map = {
            {"CONST", SchedulerType::CONST},
            {"GREEDY", SchedulerType::GREEDY},
    };
    return map.at(str);
}

std::string graph_guidance_type_to_string(GraphGuidanceType type) {
    const static std::array<std::string, static_cast<uint32_t>(GraphGuidanceType::NUM)> map = {
            "DISABLE",
            "ENABLE",
    };
    return map.at(static_cast<uint32_t>(type));
}

GraphGuidanceType string_to_graph_guidance_type(const std::string &str) {
    const static std::map<std::string, GraphGuidanceType> map = {
            {"DISABLE", GraphGuidanceType::DISABLE},
            {"ENABLE", GraphGuidanceType::ENABLE},
    };
    return map.at(str);
}

// ==================================

MapType &get_map_type() {
    static MapType type = MapType::NUM;
    return type;
}

PlannerType &get_planner_type() {
    static PlannerType type = PlannerType::EPIBT;
    return type;
}

SchedulerType &get_scheduler_type() {
    static SchedulerType type = SchedulerType::CONST;
    return type;
}

GraphGuidanceType &get_graph_guidance_type() {
    static GraphGuidanceType type = GraphGuidanceType::DISABLE;
    return type;
}

uint32_t &get_step_time() {
    static uint32_t step_time = 0;
    return step_time;
}
