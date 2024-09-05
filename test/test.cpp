//#define S2N_CPP_VERSION 201402L
#include "str-to-num.hpp"

#include <iostream> // std::cout
#include <cstddef> // size_t

using namespace std;
using namespace s2n_str_traits;

template<typename Ta, typename Tb>
constexpr bool same_str_data(const Ta &str_a, const Tb &str_b) {
  for (size_t idx = 0; idx < str_a.count; ++idx) {
    if (str_a.data[idx] != str_b.data[idx]) {
      return false;
    }
  }
  return true;
}

int main()
{
  {
    constexpr auto smsm = s2n<s2n_cvt::xor_cvt<>>("my super secret");
    constexpr auto smsm_22 = s2n<s2n_cvt::xor_cvt<>>(L"my super secret");
    
    static_assert(smsm.can_convert_to_str, "error");
    static_assert(smsm_22.can_convert_to_str, "error");
    static_assert(same_str_data(smsm_22.str(), smsm.str()), "error");
    
    std::cout << "my secret string = [" << smsm.str().data << "], count=" << smsm.str_count << std::endl;
  }
  
  {
    constexpr auto smsmempty = s2n<s2n_cvt::crc32>("");
    constexpr auto dempty = smsmempty.data();
    static_assert(smsmempty.data() == 0, "error");
  }

  {
    constexpr auto smsm22 = s2n<s2n_cvt::crc32>("my super secret");
    constexpr auto smsm22_22 = s2n<s2n_cvt::crc32>(L"my super secret");

    static_assert(smsm22 == smsm22_22, "error");
    static_assert(!smsm22.can_convert_to_str, "error");
    static_assert(0x6e103909U == smsm22.data(), "error");
    // constexpr auto sdata22 = smsm22.str();

    std::cout << std::hex << "crc32 = 0x" << smsm22.data() << std::endl;
  }

  {
    constexpr auto smsm33 = s2n<s2n_cvt::hash_fnv_1a_64>("my super secret");
    constexpr auto smsm33_22 = s2n<s2n_cvt::hash_fnv_1a_64>(L"my super secret");
    static_assert(smsm33 == smsm33_22, "error");
    static_assert(!smsm33.can_convert_to_str, "error");
    // constexpr auto sdata33 = smsm22.str();

    std::cout << std::hex << "FNV 1a = 0x" << smsm33.data() << std::endl;
  }


  using tt1 = str_char_t<char (&) [4]>;
  using tt2 = str_char_t<const char (&) [4]>;
  using tt4 = str_char_t<char *>;
  using tt5 = str_char_t<const char *>;
  using tt6 = str_char_t<const char * const>;
  
  static_assert( str_count_v<decltype("0")> == 1, "error" );
  static_assert( str_count_v<decltype("")> == 1, "error" );
  static_assert( str_count_v<decltype(L"")> == 1, "error" );
  static_assert( str_count_v<decltype("hello")> == 5, "error" );

  static_assert( s2n_traits::same_data("aaaa", "aaaa"), "error" );
  static_assert( s2n_traits::same_data("aaaa", L"aaaa"), "error" );
  static_assert( s2n_traits::same_data(L"aaaa", "aaaa"), "error" );
  static_assert( s2n_traits::same_data((char(&)[5])"aaaa", (const char(&)[5])"aaaa"), "error" );
  static_assert( s2n_traits::same_data((const char(&)[5])"aaaa", "aaaa"), "error" );
#ifdef _MSC_VER
  static_assert( s2n_traits::same_data((char[5])"aaaa", (const char[5])"aaaa"), "error" );
  static_assert( s2n_traits::same_data((char[5])"aaaa", (char[5])"aaaa"), "error" );
  static_assert( s2n_traits::same_data((const char(&)[5])"aaaa", (char[5])"aaaa"), "error" );
  static_assert( s2n_traits::same_data("aaaa", (char[5])"aaaa"), "error" );
#endif

  return 0;
}