#ifndef ZTL_TYPE_STDDEF_HPP
#define ZTL_TYPE_STDDEF_HPP

#include <stddef.h>

namespace ztl {

using ::ptrdiff_t;
using ::size_t;
using nullptr_t = decltype(nullptr);

}  // namespace ztl

#endif  // ZTL_TYPE_STDDEF_HPP
