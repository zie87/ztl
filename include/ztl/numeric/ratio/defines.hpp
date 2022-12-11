#ifndef ZTL_NUMERIC_RATIO_DEFINES_HPP
#define ZTL_NUMERIC_RATIO_DEFINES_HPP

#include "ztl/numeric/ratio/ratio.hpp"

namespace ztl {

using atto  = ratio<1, 1000000000000000000LL>;
using femto = ratio<1, 1000000000000000LL>;
using pico  = ratio<1, 1000000000000LL>;
using nano  = ratio<1, 1000000000>;
using micro = ratio<1, 1000000>;
using milli = ratio<1, 1000>;
using centi = ratio<1, 100>;
using deci  = ratio<1, 10>;
using deca  = ratio<10, 1>;
using hecto = ratio<100, 1>;
using kilo  = ratio<1000, 1>;
using mega  = ratio<1000000, 1>;
using giga  = ratio<1000000000, 1>;
using tera  = ratio<1000000000000LL, 1>;
using peta  = ratio<1000000000000000LL, 1>;
using exa   = ratio<1000000000000000000LL, 1>;

}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_DEFINES_HPP
