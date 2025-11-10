#include <planner/epibt/pepibt_lns.hpp>

#include <planner/epibt/epibt_lns.hpp>
#include <utils/tools.hpp>

PEPIBT_LNS::PEPIBT_LNS(Robots &robots, TimePoint end_time, const std::vector<uint32_t> &operations) : robots(robots), end_time(end_time), best_actions(this->robots.size(), ActionType::WAIT), best_desires(operations) {
}

void PEPIBT_LNS::solve(uint64_t seed) {
    Timer timer;
    EPIBT_LNS main(robots, end_time, best_desires);
    main.EPIBT::solve();

    // (score, actions, desires, time, lns_steps)
    using ItemType = std::tuple<double, std::vector<ActionType>, std::vector<uint32_t>, uint32_t, uint32_t>;

    std::vector<std::vector<ItemType>> results_pack(THREADS_NUM_VALUE);

    results_pack[0].emplace_back(main.get_score(), main.get_actions(), main.get_desires(), timer.get_ms(), main.get_lns_steps());

    launch_threads(THREADS_NUM_VALUE, [&](uint32_t thr) {
        Randomizer rnd(seed * (thr + 1) + 426136423);
        while (get_now() < end_time) {
            Timer timer;
            EPIBT_LNS solver = main;
            solver.parallel_solve(rnd.get());
            auto time = timer.get_ms();
            results_pack[thr].emplace_back(solver.get_score(), solver.get_actions(), solver.get_desires(), time, solver.get_lns_steps());
        }
    });

    std::vector<ItemType> results;
    for (uint32_t thr = 0; thr < results_pack.size(); thr++) {
        for (auto &item: results_pack[thr]) {
            results.emplace_back(std::move(item));
        }
    }

    double best_score = -1e300;
    std::sort(results.begin(), results.end(), [&](const auto &lhs, const auto &rhs) {
        return std::get<0>(lhs) > std::get<0>(rhs);
    });

    for (const auto &[score, actions, desires, time, lns_steps]: results) {
        if (best_score < score) {
            best_score = score;
            best_actions = actions;
            best_desires = desires;
        }
    }
}

std::vector<uint32_t> PEPIBT_LNS::get_desires() const {
    return best_desires;
}

std::vector<ActionType> PEPIBT_LNS::get_actions() const {
    return best_actions;
}
