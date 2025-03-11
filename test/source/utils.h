#pragma once

#include "twsfwphysx/twsfwphysx.h"

struct twsfwphysx_vec make_vec(float x, float y, float z);

void assert_vec_eq(struct twsfwphysx_vec v, float x, float y, float z);

void assert_vec_eq_with_tolerance(struct twsfwphysx_vec v,
                                  float x,
                                  float y,
                                  float z,
                                  float abs);
