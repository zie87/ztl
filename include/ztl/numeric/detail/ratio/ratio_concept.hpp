#ifndef ZTL_NUMERIC_DETAIL_RATIO_RATIO_CONCEPT_HPP
#define ZTL_NUMERIC_DETAIL_RATIO_RATIO_CONCEPT_HPP

#include "ztl/numeric/detail/ratio/ratio_trait.hpp"

namespace ztl::detail {

template <typename T>
concept ratio_concept = is_ratio_v<T>;

}  // namespace ztl::detail

#endif  // ZTL_NUMERIC_DETAIL_RATIO_RATIO_CONCEPT_HPP
