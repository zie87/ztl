#ifndef ZTL_TYPE_STDDEF_HPP
#define ZTL_TYPE_STDDEF_HPP

#include <stddef.h>

namespace ztl {

using ::max_align_t;
using ::ptrdiff_t;
using ::size_t;
using nullptr_t = decltype(nullptr);

}  // namespace ztl

#endif  // ZTL_TYPE_STDDEF_HPP
