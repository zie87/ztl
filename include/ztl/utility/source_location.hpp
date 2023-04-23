#ifndef ZTL_UTILITY_SOURCE_LOCATION_HPP
#define ZTL_UTILITY_SOURCE_LOCATION_HPP

#include "ztl/platform/cpp_std.hpp"
#include "ztl/utility/macros.hpp"

#if ZTL_CPP20_OR_GREATER

#include <source_location>

namespace ztl {
using std::source_location;
}  // namespace ztl

#define ZTL_CURRENT_SOURCE_LOCATION ztl::source_location::current()

#else

namespace ztl {

class source_location {
public:
    static inline ZTL_CONSTEVAL source_location
    current(const char* fna, unsigned lin, const char* fun) ZTL_NOEXCEPT;

    source_location(const char* fna,
                    unsigned lin,
                    const char* fun) ZTL_NOEXCEPT;

    inline ZTL_CONSTEXPR_14 unsigned line() const ZTL_NOEXCEPT;
    inline ZTL_CONSTEXPR_14 const char* file_name() const ZTL_NOEXCEPT;
    inline ZTL_CONSTEXPR_14 const char* function_name() const ZTL_NOEXCEPT;

private:
    const char* m_file_name;
    unsigned m_line_number;
    const char* m_function_name;
};

inline ZTL_CONSTEVAL source_location source_location::current(const char* fna,
                                                              unsigned lin,
                                                              const char* fun)
    ZTL_NOEXCEPT {
    return source_location(fna, lin, fun);
}

source_location::source_location(const char* fna,
                                 unsigned lin,
                                 const char* fun) ZTL_NOEXCEPT
    : m_file_name(fna),
      m_line_number(lin),
      m_function_name(fun) {}

inline ZTL_CONSTEXPR_14 unsigned source_location::line() const ZTL_NOEXCEPT {
    return m_line_number;
}
inline ZTL_CONSTEXPR_14 const char* source_location::file_name() const
    ZTL_NOEXCEPT {
    return m_file_name;
}
inline ZTL_CONSTEXPR_14 const char* source_location::function_name() const
    ZTL_NOEXCEPT {
    return m_function_name;
}

}  // namespace ztl

#define ZTL_CURRENT_SOURCE_LOCATION                                          \
    ztl::source_location::current(ZTL_FILE, static_cast<unsigned>(ZTL_LINE), \
                                  ZTL_FUNCTION)

#endif  // defined(ZTL_CPP20_OR_GREATER)

#endif  // ZTL_UTILITY_SOURCE_LOCATION_HPP
