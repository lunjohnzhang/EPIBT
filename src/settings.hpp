#pragma once

#include <cstdint>

// Compile-time settings now controlled by CMake options:
// -DENABLE_ROTATE_MODEL=ON/OFF
// -DENABLE_ASSERT=ON/OFF
// -DENABLE_SCHEDULER_CHANGE_TASK=ON/OFF
// -DEPIBT_DEPTH=<value>
// -DTHREADS_NUM_DEFAULT=<value>

// Use the CMake-defined value, or fallback
constexpr uint32_t THREADS_NUM = THREADS_NUM_DEFAULT;

/* TODO
 * 1) улучшить проверки в Answer
 */

constexpr uint32_t EPIBT_REVISIT_NUM = 10;

// TODO: remove get_operation_weight

// #define ENABLE_EPIBT_SMART_OPERATION_EXECUTION

// #define ENABLE_WRITE_SCHEDULE

// выключить наследование операций EPIBT
#define ENABLE_EPIBT_INHERITANCE

// #define ENABLE_PIBT_REVISIT
