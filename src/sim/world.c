#include "world.h"

#include "array_utils.h"

// setup graph struct to initial values
void wld_init(World *wld, int width, int height, float init_pressure, float ang_vel, float cell_size) {
    wld->width = width;
    wld->height = height;

    wld->angular_vel = ang_vel;
    wld->cell_size = cell_size;

    set_all(wld->pressure, init_pressure);
    set_all(wld->old_pressure, init_pressure);

    zeros(wld->wind_vel_x);
    zeros(wld->wind_vel_y);

    zeros(wld->old_wind_vel_x);
    zeros(wld->old_wind_vel_y);

    zeros(wld->air_temp);
    zeros(wld->precip);
    zeros(wld->humidity);

    zeros(wld->ground_temp);
    zeros(wld->ground_height);
    zeros(wld->ground_water);

#ifdef RND_DATA
    //fill_rnd_wldarray(wld->pressure, EXPECTED_PRESSURE_MAX, EXPECTED_PRESSURE_MIN);

    fill_pattern_wldarray(wld->pressure, EXPECTED_PRESSURE_MAX, EXPECTED_PRESSURE_MIN);
    fill_pattern_wldarray(wld->wind_vel_y, EXPECTED_WINDVEL_MAX, -EXPECTED_WINDVEL_MAX);
    fill_pattern_wldarray(wld->wind_vel_x, EXPECTED_WINDVEL_MAX, -EXPECTED_WINDVEL_MAX);
#endif
}
