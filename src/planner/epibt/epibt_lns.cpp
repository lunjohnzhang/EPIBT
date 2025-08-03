#include <planner/epibt/epibt_lns.hpp>

#include <environment/info.hpp>
#include <utils/assert.hpp>

bool EPIBT_LNS::consider() {
    return old_score - 1e-6 <= cur_score;
}

EPIBT_LNS::RetType EPIBT_LNS::try_build(uint32_t r, uint32_t &counter) {
    if (counter % 4 == 0 && get_now() >= end_time) {
        return RetType::REJECTED;
    }

    if (visited[r] != visited_counter) {
        visited_num[r] = 0;
    }
    visited_num[r]++;
    visited[r] = visited_counter;
    curr_visited[r] = visited_counter;

    uint32_t old_desired = desires[r];
    for (uint32_t desired: robot_desires[r]) {
        desires[r] = desired;
        uint32_t to_r = get_used(r);
        if (to_r == -1) {
            add_path(r);
            if (consider()) {
                return RetType::ACCEPTED;
            } else {
                remove_path(r);
                desires[r] = old_desired;
                return RetType::REJECTED;
            }
        } else if (to_r != -2) {
            ASSERT(0 <= to_r && to_r < robots.size(), "invalid to_r");

            if (curr_visited[to_r] == visited_counter ||                                    // если мы уже построили его сейчас
                (visited[to_r] == visited_counter && visited_num[to_r] >= EPIBT_REVISIT_VALUE)// ограничение на количество посещений <= EPIBT_REVISIT_VALUE
            ) {
                continue;
            }

            remove_path(to_r);
            add_path(r);

            RetType res = try_build(to_r, ++counter);
            if (res == RetType::ACCEPTED) {
                return res;
            } else if (res == RetType::REJECTED) {
                remove_path(r);
                add_path(to_r);
                desires[r] = old_desired;
                return res;
            }

            remove_path(r);
            add_path(to_r);
        }
    }

    desires[r] = old_desired;
    curr_visited[r] = 0;
    return RetType::FAILED;
}

void EPIBT_LNS::try_build(uint32_t r) {
    old_score = cur_score;
    remove_path(r);
    uint32_t counter = 0;
    if (try_build(r, counter) != RetType::ACCEPTED) {
        add_path(r);
    }
}

EPIBT_LNS::EPIBT_LNS(Robots &robots, TimePoint end_time, const std::vector<uint32_t> &operations)
    : EPIBT(robots, end_time, operations) {
}

void EPIBT_LNS::solve(uint64_t seed) {
    rnd = Randomizer(seed);
    EPIBT::solve();

    auto call_epibt = [&]() {
        visited_counter++;
        for (uint32_t r: order) {
            if (get_now() >= end_time) {
                break;
            }
            if (visited[r] != visited_counter) {
                try_build(r);
            }
        }
    };

    while (get_now() < end_time) {
        std::shuffle(order.begin(), order.end(), rnd.generator);
        call_epibt();
    }
}

void EPIBT_LNS::parallel_solve(uint64_t seed) {
    rnd = Randomizer(seed);
    while (get_now() < end_time && lns_step < robots.size() * 10) {
        uint32_t r = rnd.get(0, robots.size() - 1);
        try_build(r);
        lns_step++;
    }
}

uint32_t EPIBT_LNS::get_lns_steps() const {
    return lns_step;
}
