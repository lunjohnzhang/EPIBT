#pragma once

#include <cstdint>

// Compile-time settings now controlled by CMake options:
// -DENABLE_ROTATE_MODEL=ON/OFF
// -DENABLE_ASSERT=ON/OFF
// -DENABLE_SCHEDULER_CHANGE_TASK=ON/OFF
// -DEPIBT_DEPTH=<value>
// -DTHREADS_NUM_DEFAULT=<value>

// Convert macro to constexpr

constexpr uint32_t THREADS_NUM_VALUE = THREADS_NUM;

constexpr uint32_t EPIBT_DEPTH_VALUE = EPIBT_DEPTH;

constexpr uint32_t EPIBT_REVISIT_VALUE = EPIBT_REVISIT;
