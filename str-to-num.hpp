/*
MIT License

Copyright (c) 2024 forgotthepen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef STR_2_NUM_H
#define STR_2_NUM_H


#ifndef S2N_CPP_VERSION
  #define S2N_CPP_VERSION __cplusplus 
#endif

#if S2N_CPP_VERSION < 201402L
  #error "C++14 or greater is required"
#endif

// noexcept cannot be part of a typedef before C++17
// starting from C++17 onwards it is legal (and required to use) as part of typedef
// https://en.cppreference.com/w/cpp/language/except_spec
#if S2N_CPP_VERSION < 201703L
  #define CPP17_TYPEDEF_NOEXCEPT
#else
  #define CPP17_TYPEDEF_NOEXCEPT noexcept
#endif


using s2n_sz_t = unsigned long long;


namespace s2n_traits {
  template<bool val>
  struct bool_holder {
    constexpr static const bool value = val;
  };

  template<typename, typename>
  struct same_type : bool_holder<false> {};
  template<typename Ta>
  struct same_type<Ta, Ta> : bool_holder<true> {};
  template<typename Ta, typename Tb>
  constexpr static const bool same_type_v = same_type<Ta, Tb>::value;

  template<bool val, typename>
  struct allowed_if {};
  template<typename Ta>
  struct allowed_if<true, Ta> {
    using type = Ta;
  };
  template<bool val, typename Ta>
  using allowed_if_t = typename allowed_if<val, Ta>::type;

  template<typename Tref>
  struct no_ref {
    using type = Tref;
  };
  template<typename Tref>
  struct no_ref<Tref&> {
    using type = Tref;
  };
  template<typename Tref>
  struct no_ref<Tref&&> {
    using type = Tref;
  };
  template<typename Tref>
  using no_ref_t = typename no_ref<Tref>::type;

  template<typename TConst>
  struct no_const {
    using type = TConst;
  };
  template<typename TConst>
  struct no_const<const TConst> {
    using type = TConst;
  };
  template<typename TConst>
  using no_const_t = typename no_const<TConst>::type;

  template<typename TArray>
  struct no_array {
    using type = TArray;
  };
  template<typename TArray, s2n_sz_t N>
  struct no_array<TArray[N]> {
    using type = TArray;
  };
  template<typename TArray>
  using no_array_t = typename no_array<TArray>::type;

  template<typename TPtr>
  struct no_ptr {
    using type = TPtr;
  };
  template<typename TPtr>
  struct no_ptr<TPtr*> {
    using type = TPtr;
  };
  template<typename TPtr>
  using no_ptr_t = typename no_ptr<TPtr>::type;

  template<typename Ta, typename Tb>
  constexpr static bool same_data(const Ta &pa, const Tb &pb) { return pa == pb; }
  template<typename Ta, typename Tb, s2n_sz_t N>
  constexpr static bool same_data(const Ta(&pa)[N], const Tb(&pb)[N]) {
    for (s2n_sz_t idx = 0; idx < N; ++idx) {
      if (!same_data(pa[idx], pb[idx])) {
        return false;
      }
    }
    return true;
  }

}

namespace s2n_str_traits {
  template<typename TStr>
  struct str_traits;

  // ------------------------------------ pointer/array str traits
  // (const char *)"str"
  template<typename TChar>
  struct str_traits<TChar*> {
    using type = s2n_traits::no_const_t<s2n_traits::no_ptr_t<TChar>>;
  };
  // (const char * const)"str"
  template<typename TChar>
  struct str_traits<TChar* const> {
    using type = s2n_traits::no_const_t<s2n_traits::no_ptr_t<s2n_traits::no_const_t<TChar>>>;
  };
  // (const char (& const) [4])"str" // MSVC
  // (const char (&) [4])"str" // GCC
  template<typename TChar, s2n_sz_t N>
  struct str_traits<TChar (&) [N]> {
    using type = s2n_traits::no_const_t<s2n_traits::no_array_t<s2n_traits::no_ref_t<s2n_traits::no_const_t<TChar>>>>;
    constexpr static s2n_sz_t count = N > 1 ? N - 1 : 1;
  };
  // (char [4])"str"
  template<typename TChar, s2n_sz_t N>
  struct str_traits<TChar [N]> {
    using type = s2n_traits::no_const_t<s2n_traits::no_array_t<s2n_traits::no_ref_t<s2n_traits::no_const_t<TChar>>>>;
    constexpr static s2n_sz_t count = N > 1 ? N - 1 : 1;
  };

  template<typename TStr>
  using str_char_t = typename str_traits<TStr>::type;
  template<typename TStr>
  constexpr static s2n_sz_t str_count_v = str_traits<TStr>::count;

}


template<typename TConverter, typename TStr>
class s2n_basic {
public:
  constexpr static s2n_sz_t str_count = s2n_str_traits::str_count_v<TStr>;

private:
  // make all specializations friends for == comparison
  template<typename TConverterOther, typename TStrOther>
  friend class s2n_basic;

  // trait to get param #1 from TConverter::to_num()
  template<typename>
  struct cvt_to_num_p1;
  template<typename TRet, typename TP1, typename... TParams>
  struct cvt_to_num_p1<TRet(*)(TP1, TParams...) CPP17_TYPEDEF_NOEXCEPT> {
    using type = TP1;
  };
  template<typename TPFn>
  using cvt_to_num_p1_t = typename cvt_to_num_p1<TPFn>::type;

  using TChar = s2n_str_traits::str_char_t<TStr>;
  using TStringCvt = TConverter;
  using TOut = typename s2n_traits::no_ref_t<
    cvt_to_num_p1_t<decltype(
      &TStringCvt::template to_num<TStr, str_count, TChar>
    )>
  >;

  using to_str_signature = void(*)(TChar(&)[str_count + 1], const TChar(&)[str_count]) CPP17_TYPEDEF_NOEXCEPT;

  // SFINAE to check if TConverter::to_str() exists
  template <typename TC>
  static s2n_traits::bool_holder<
    s2n_traits::same_type_v<to_str_signature, decltype(&TC::template to_str<TStr, str_count, TChar>)>
  > to_str_defined(decltype(&TC::template to_str<TStr, str_count, TChar>));
  template <typename>
  static s2n_traits:: bool_holder<false> to_str_defined(...);

  TOut m_data{};

public:
  // use a container since TConverter::to_str() will return an array
  struct str_container {
    constexpr static s2n_sz_t count = str_count;
    TChar data[str_count + 1]{};
  };

  constexpr static const bool can_convert_to_str = decltype(to_str_defined<TStringCvt>(nullptr))::value;
  
  constexpr s2n_basic(const TStr &str) noexcept {
    TStringCvt::template to_num<TStr, str_count, TChar>(m_data, str);
  }

  template<typename TConverterOther, typename TStrOther>
  constexpr bool operator==(const s2n_basic<TConverterOther, TStrOther> &other) const noexcept {
    if (other.str_count != this->str_count) {
      return false;
    }

    return s2n_traits::same_data(other.m_data, this->m_data);
  }
  
  template<typename TConverterOther, typename TStrOther>
  constexpr bool operator!=(const s2n_basic<TConverterOther, TStrOther> &other) const noexcept {
    return !( this == other );
  }

  template<typename TCvt = TStringCvt,
		// we have to use the function template param, otherwise it is a hard error
		s2n_traits::allowed_if_t<decltype(to_str_defined<TCvt>(nullptr))::value, bool> = true
	> 
  constexpr str_container str() const noexcept {
    str_container dec{};
    TStringCvt::template to_str<TStr, str_count, TChar>(dec.data, static_cast<const TChar(&)[str_count]>(m_data));
    dec.data[str_count] = static_cast<TChar>(0);
    return dec;
  }

  constexpr const TOut& data() const noexcept {
    return m_data;
  }

};


namespace s2n_cvt {

  template<unsigned int VKey = 0xA5>
  struct xor_cvt {
    template<typename TStr, s2n_sz_t StrCount, typename TChar>
    constexpr static void to_num(TChar (&dst) [StrCount], const TStr& src) noexcept {
      for (s2n_sz_t src_idx = 0; src_idx < StrCount; ++src_idx) {
        dst[src_idx] = src[src_idx] ^ static_cast<TChar>(VKey ^ src_idx);
      }
    }

    template<typename TStr, s2n_sz_t StrCount, typename TChar>
    constexpr static void to_str(TChar (&dst) [StrCount + 1], const TChar (&src) [StrCount]) noexcept {
      for (s2n_sz_t src_idx = 0; src_idx < StrCount; ++src_idx) {
        dst[src_idx] = src[src_idx] ^ static_cast<TChar>(VKey ^ src_idx);
      }
    }
  };

  struct crc32 {
    template<typename TStr, s2n_sz_t StrCount, typename TChar>
    constexpr static void to_num(unsigned int &crc, const TStr& src) noexcept {
      crc = 0;
      // this is illegal syntax: char my_array[0] >>> size must be > 0
      // instead we get a string of length 1 whose 1st char is null
      if (1 == StrCount && !src[0]) {
        return;
      }

      crc = 0xFFFFFFFFUL;
      for (s2n_sz_t src_idx = 0; src_idx < StrCount; src_idx++) {
        auto current_byte = src[src_idx];
        for (s2n_sz_t bit_idx = 0; bit_idx < 8; bit_idx++) { // foreach bit in this byte
          unsigned char bit_val = static_cast<unsigned char>((current_byte ^ crc) & 1);
          // discard this bit
          current_byte >>= 1;
          crc >>= 1;
          if (bit_val) {
            crc ^= 0xEDB88320;
          }
        }
      }

      crc = ~crc;
    }
  };

  // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
  struct hash_fnv_1a_64 {
    template<typename TStr, s2n_sz_t StrCount, typename TChar>
    constexpr static void to_num(s2n_sz_t &hash, const TStr& src) noexcept {
      constexpr auto FNV_offset_basis = 0xcbf29ce484222325ULL;
      constexpr auto FNV_prime = 0x100000001b3ULL;

      hash = FNV_offset_basis;

      // this is illegal syntax: char my_array[0] >>> size must be > 0
      // instead we get a string of length 1 whose 1st char is null
      if (1 == StrCount && !src[0]) {
        return;
      }

      for (s2n_sz_t src_idx = 0; src_idx < StrCount; src_idx++) {
        hash = hash ^ src[src_idx];
        hash = hash * FNV_prime;
      }
    }
  };

}


// helper function to create s2n instance and avoid manually specifying the string type
template<typename TConverter, typename TStr>
constexpr static auto s2n(const TStr &str) {
  return s2n_basic<TConverter, TStr>(str);
}


#endif
