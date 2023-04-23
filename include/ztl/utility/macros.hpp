#ifndef ZTL_UTILITY_MACROS_HPP
#define ZTL_UTILITY_MACROS_HPP

#define ZTL_STRINGIFY(x) WGE_STRINGFY_HELPER(x)
#define ZTL_STRINGFY_HELPER(x) #x

#define ZTL_CONCAT(x, y) ZTL_CONCAT_HELPER(x, y)
#define ZTL_CONCAT_HELPER(x, y) x##y

#define ZTL_LINE __LINE__
#define ZTL_FILE __FILE__
#define ZTL_FUNCTION __PRETTY_FUNCTION__

#endif  // ZTL_UTILITY_MACROS_HPP
