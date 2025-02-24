#pragma once

/// @brief Represents the weather station data for each city.
typedef struct {
    float sum = 0.0;
    int count = 0;
    float min = 100;
    float max = -100; 
} data_t;
