/* Copyright (c) 2018, rmawatson@hotmail.com
 *
 * Distributed under the Boost Software License, Version 1.0 (found in the
 * LICENSE file in the root directory of this source tree)
 */
 
/* this file has been automatically generated on 2023-02-03 03:08:32 and should not be edited manually. */
 
#pragma once
#include <algorithm>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <cstring>
#include <tuple>

 

# define UTF_PLATFORM_LITTLE_ENDIAN utf::little_endian
# define UTF_PLATFORM_BIG_ENDIAN utf::big_endian
# ifdef _WIN32
#   define UTF_PLATFORM_ENDIANESS UTF_PLATFORM_LITTLE_ENDIAN
#   define UTF_PLATFORM_BSWAP32(U32) _byteswap_ulong(U32)
#   define UTF_PLATFORM_BSWAP16(U16) _byteswap_ushort(U16)
# else
#   if defined(__linux__)
#     include <endian.h>
#   elif defined(OS_MACOSX)
#     include <machine/endian.h>
#   endif
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#     define UTF_PLATFORM_ENDIANESS UTF_PLATFORM_LITTLE_ENDIAN
#   elif __BYTE_ORDER  == __BIG_ENDIAN
#     define UTF_PLATFORM_ENDIANESS UTF_PLATFORM_BIG_ENDIAN
#   endif
#   define UTF_PLATFORM_BSWAP32(U32) __builtin_bswap32(U32)
#   define UTF_PLATFORM_BSWAP16(U16) __builtin_bswap16(U16)
# endif

namespace utf
{

#ifdef __cpp_char8_t
    using u8string = std::u8string;
#else 
    using char8_t = char;
    using u8string = std::string;
#endif
    using u16string = std::u16string;
    using u32string = std::u32string;

    namespace mp {

        template <typename...>
        using void_t = void;

        template <typename... Ts>
        struct list {};

        template <template <typename...> class Fn, typename... Ts>
        struct defer {
            using type = Fn<Ts...>;
        };

        template <typename T>
        struct identity {
            using type = T;
        };

        template <size_t V>
        using size_c = std::integral_constant<size_t, V>;

        template <bool V>
        using bool_c = std::integral_constant<bool, V>;

        template <bool V>
        using int_c = std::integral_constant<int, V>;

        template <bool V>
        using uint_c = std::integral_constant<unsigned int, V>;

        template <template <typename...> class Fn>
        struct func
        {
            template <typename... Ts>
            using fn = typename defer<Fn, Ts...>::type;
        };

        template <template <typename...> class Fn>
        using wrap = func<Fn>;
        
        template <template <typename...> class T>
        struct trait
        {
            template <typename... Ts>
            using fn = typename T<Ts...>::type;
        };

        namespace detail {
            template <typename, template <typename...> class Fn, typename... Ts >
            struct is_valid_impl : std::false_type {};

            template <template <typename...> class Fn, typename... Ts>
            struct is_valid_impl<void_t<Fn<Ts...>>, Fn, Ts...> : std::true_type {};
        }

        template <template <typename...> class Fn, typename... Ts>
        using is_valid = detail::is_valid_impl<void, Fn, Ts...>;

        template < typename Q, typename... Ts>
        using is_valid_q = is_valid<Q::template fn, Ts...>;


		namespace detail {
			template <typename, template<typename...> class Fn, typename... Ts >
			struct is_instantiable_impl : std::false_type {};

			template <template<typename...> class Fn, typename... Ts>
			struct is_instantiable_impl<mp::void_t<decltype(Fn<Ts...>())>, Fn, Ts...> : std::true_type {};
		}

		template <template<typename...> class Fn, typename... Ts>
		using is_instantiable = detail::is_instantiable_impl<void, Fn, Ts...>;

		template < typename Q, typename... Ts>
		using is_instantiable_q = is_instantiable<Q::template fn, Ts...>;

        template <template <typename...> class F, typename... Ts>
        struct bind_back
        {
            template <typename... Ys>
            using fn = typename defer<F, Ys..., Ts...>::type;
        };

        template <template <typename...> class F, typename... Ts>
        struct bind_front
        {
            template <typename... Ys>
            using fn = typename defer<F, Ts..., Ys...>::type;
        };

        namespace detail
        {
            template <typename F, typename S>
            struct transform_impl;

            template <typename F, template <typename...> class S, typename... Ts>
            struct transform_impl<F, S<Ts...>>
            {
                using type = S<typename defer<F::template fn, Ts>::type...>;
            };
        }

        template <typename F, typename S>
        using transform = typename detail::transform_impl<F, S>::type;

        namespace detail {

            template <typename, template <typename...> class>
            struct rename_impl;

            template <template <typename...> class A, typename... Ts, template <typename...> class B>
            struct rename_impl<A<Ts...>, B>
            {
                using type = B<Ts...>;
            };
        }

        template <typename A, template <typename...> class B>
        using rename = typename detail::rename_impl<A, B>::type;

        namespace detail
        {
            template <bool v, typename T, typename... E>
            struct if_impl;

            template <typename T, typename... E>
            struct if_impl<true, T, E...>
            {
                using type = T;
            };

            template <typename T, typename E>
            struct if_impl<false, T, E>
            {
                using type = E;
            };
        }

        template <typename C, typename T, typename E>
        using if_ = typename detail::if_impl<static_cast<bool>(C::value), T, E>::type;

        template <bool v, typename T, typename E>
        using if_v = typename  detail::if_impl<v, T, E>::type;

        namespace detail
        {
            template<typename S>
            struct first_impl;

            template<template <typename...> class S, typename T, typename... Ts>
            struct first_impl<S<T, Ts...>>
            {
                using type = T;
            };

            template<typename S>
            struct second_impl;

            template<template <typename...> class S, typename T, typename T1, typename... Ts>
            struct second_impl<S<T, T1, Ts...>>
            {
                using type = T1;
            };
        }

        template <typename S>
        using first = typename detail::first_impl<S>::type;

        template <typename S>
        using second = typename detail::second_impl<S>::type;

        namespace detail {

            template <typename... Ts>
            struct inherit : Ts...{};

            template <typename M, typename T>
            struct map_find_impl;

            template <template <typename...> class M, typename... Ys, typename T>
            struct map_find_impl<M<Ys...>, T>
            {
                using _kvs = inherit<identity<Ys>...>;

                template <template <typename...> class P, typename... Vs>
                static constexpr identity<P<T, Vs...>> _f(identity<P<T, Vs...>>*);
                static constexpr identity<void> _f(...);
                using _result = decltype(f(reinterpret_cast<_kvs*>(nullptr)));
                using type = typename _result::type;
            };
        }

        template <typename M, typename T>
        using map_find = typename detail::map_find_impl<M, T>::type;

        namespace detail {

            template<
                class S1 = list<>, class S2 = list<>, class S3 = list<>, class S4 = list<>, class S5 = list<>, class S6 = list<>,
                class S7 = list<>, class S8 = list<>, class S9 = list<>, class S10 = list<>, class S11 = list<>
            > struct append_impl
            {};

            template<
                template<class...> class S1, template<class...> class S2, template<class...> class S3, template<class...> class S4,
                template<class...> class S5, template<class...> class S6, template<class...> class S7, template<class...> class S8,
                template<class...> class S9, template<class...> class S10, template<class...> class S11,
                typename... T1, typename... T2, typename... T3, typename... T4, typename... T5, typename... T6, typename... T7,
                typename... T8, typename... T9, typename... T10, typename... T11>
                struct append_impl<S1<T1...>, S2<T2...>, S3<T3...>, S4<T4...>, S5<T5...>, S6<T6...>, S7<T7...>, S8<T8...>, S9<T9...>, S10<T10...>, S11<T11...>>
            {
                using type = S1<T1..., T2..., T3..., T4..., T5..., T6..., T7..., T8..., T9..., T10..., T11...>;
            };
        }

        template<typename... Ls>
        using append = typename detail::append_impl<Ls...>::type;

        namespace detail {
            template<typename, template <typename...> class >
            struct filter_if_impl;

            template <template <typename...> class S, template <typename...> class F, typename...Ts >
            struct filter_if_impl<S<Ts...>, F>
            {
                template<class Y, class... Ys>
                using _filter = mp::if_<typename F<Y>::type, mp::list<>, mp::list<Y>>;

                using _filtered = mp::transform<mp::func<_filter>, S<Ts...>>;
                using type = mp::rename< mp::transform<mp::func<_filter>, S<Ts...>>, mp::append>;
            };
        }

        template <typename S, template <typename...> class F>
        using filter_if = typename detail::filter_if_impl<S, F>::type;

        template <typename S, typename  F>
        using filter_if_q = typename  detail::filter_if_impl<S, F::template fn>::type;

        namespace detail
        {
            template <typename... >
            struct make_map;

            template <typename... Ts, size_t... Is>
            struct make_map<std::index_sequence<Is...>, Ts...>
            {
                using type = list<list<size_c<Is>, Ts>...>;
            };

            template <typename...>
            struct at_impl;

            template <template <typename...> class S, typename I, typename... Ts>
            struct at_impl<S<Ts...>, I>
            {
                using type = second<map_find<typename make_map<std::index_sequence_for<Ts...>, Ts...>::type, I>>;
            };

        }

        template <typename S, typename I>
        using at = typename detail::at_impl<S, I>::type;

        template <typename S, size_t I>
        using at_v = typename detail::at_impl<S, size_c<I>>::type;

        template <typename S>
        struct size;

        template <template <typename...> class S, typename... Ts>
        struct size<S<Ts...>> : size_c<sizeof...(Ts)> {};

        template <typename S>
        using front = first<S>;

        template <typename S>
        using back = at_v<S, size<S>::value - 1>;

        template <typename B>
        using not_c = bool_c<!static_cast<bool>(B::value)>;

        namespace detail
        {
            constexpr size_t accumulate(...)
            {
                return 0;
            }

            template <typename V, typename ... Vs>
            constexpr size_t accumulate(V v, Vs... vs)
            {
                return static_cast<size_t>(v) + accumulate(vs...);
            }

            template <typename S, typename T>
            struct count_impl;

            template <template <typename...> class S, typename... Ys, typename T>
            struct count_impl<S<Ys...>, T>
            {
                static constexpr size_t _result = accumulate(static_cast<bool>(std::is_same<Ys, T>::value)... );
                using type = size_c<_result>;
            };
        }

        template <typename S, typename T>
        using count = typename detail::count_impl<S, T>::type;

        namespace detail
        {
            template <typename S, template <typename...> class P>
            struct count_if_impl;

            template <template <typename...> class S, typename... Ys, template <typename...> class P>
            struct count_if_impl<S<Ys...>, P>
            {
                static constexpr size_t _result = accumulate(static_cast<bool>(P<Ys>::value)...);
                using type = size_c<_result>;
            };
        }


        template<typename S, template <typename...> class P>
        using count_if = typename detail::count_if_impl<S, P>::type;

        template<typename S, typename P>
        using count_if_q = typename detail::count_if_impl<S, P::template fn>::type;

        template <typename S, template <typename...> class P>
        using any_of = bool_c<static_cast<bool>(count_if<S, P>::value)>;

        template <typename S, typename P>
        using any_of_q = any_of<S, P::template fn>;

        template<typename... Ts>
        using any = bool_c<(count_if_q < mp::list<bool_c<static_cast<bool>(Ts::value)>...>,
            bind_front<std::is_same, bool_c<true> >>::value != 0)>;

        template <typename S, template <typename...> class P>
        using all_of = bool_c<count_if<S, P>::value == size<S>::value>;

        template <typename S, typename P>
        using all_of_q = all_of<S, P::template fn>;

        template<typename... Ts>
        using all = bool_c<(count_if_q < mp::list<bool_c<static_cast<bool>(Ts::value)>...>,
            bind_front<std::is_same, bool_c<true> >>::value == sizeof...(Ts))>;

        template <typename S, template <typename...> class P>
        using none_of = not_c<all_of<S, P>>;

        template <typename S, typename P>
        using none_of_q = none_of<S, P::template fn>;

        template<typename... Ts>
        using none = not_c<all<Ts...>>;

        template <typename S, typename T>
        using contains = bool_c < static_cast<bool>(count<S, T>::value)>;
    }

    template < typename impl, template <typename...> class tag_tmpl>
    struct policy_default;

#define UTF_POLICY_DEFAULT(ITER,POLICYTYPE,VALUE) \
    template <typename base_iterator,typename...Ts> struct policy_default<ITER<base_iterator,Ts...>,POLICYTYPE> { using type = VALUE; }  


    namespace detail
    {
        template <typename category, typename... catagories>
		struct category_any_of : mp::any_of_q<mp::list<catagories...>, mp::bind_front< std::is_same, category>> {};

        template <typename category, typename... categories>
        using enable_if_category = typename std::enable_if<category_any_of<category, categories...>::value>::type;

        template <typename category, typename... catagories>
        using enable_if_category_b = typename std::enable_if<category_any_of<category, catagories...>::value, bool>::type;

        template< template <typename...> class tmpl, typename inst>
        struct is_instance_of_template : mp::bool_c<false> {};

        template<template <typename...> class tmpl, typename... Ts>
        struct is_instance_of_template <tmpl, tmpl<Ts...>> : mp::bool_c<true> {};

        template <typename T>
        struct is_char_type_impl {
            using type = mp::any_of_q< mp::list<char, char8_t, wchar_t, char16_t, char32_t>,
                mp::bind_front<std::is_same, T>>;
        };

        template <typename T>
        using is_char_type = typename is_char_type_impl<T>::type;

        template <typename impl, template <typename...> class policy_tmpl, typename policy_list>
        struct select_policy_handler_impl
        {
            template <typename policy>
            using _is_not_policy_q = mp::not_c<is_instance_of_template<policy_tmpl, policy>>;
            using _filtered = mp::filter_if<policy_list, _is_not_policy_q>;
            static_assert(mp::size<_filtered>::value < 2, "Multiple overlapping policies specified, single policy expected.");

            using type = typename policy_tmpl<void>::template handler< impl,
                typename mp::if_ <
                mp::size<_filtered>,
                mp::defer<mp::first, _filtered>,
                mp::identity<policy_default<impl, policy_tmpl>>
                >::type::type
            >;
        };

        template < typename impl, template <typename...> class policy_tmpl, typename... policies>
        using select_policy_handler = typename select_policy_handler_impl<impl, policy_tmpl, mp::list<policies...>>::type;

        template <template <typename...> class test, typename default_, typename... args>
        struct select_argument_or_default_impl
        {
            template <typename T>
            using not_test = mp::bool_c<!static_cast<bool>(test<T>::value)>;

            using _filtered = mp::filter_if<mp::list<args...>, not_test>;
            using type = typename mp::if_<
                mp::size<_filtered>,
                mp::defer<mp::first, _filtered>,
                mp::identity<default_>
            >::type;
        };

        template <template <typename...> class test, typename...Ts>
        using select_argument_or_default = typename select_argument_or_default_impl<test, Ts...>::type;

        template <typename T, typename enable = void>
        struct has_value_type : std::false_type {};

        template <typename T>
        struct has_value_type<T, mp::void_t<typename T::value_type>>
            : std::true_type {};

        template <typename T, typename enable = void>
        struct has_begin_end : std::false_type {};

        template <typename T>
        struct has_begin_end<T, mp::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
            : std::true_type {};
    }

    template <uint32_t value_>
    struct replace_with {
        static constexpr uint32_t value = value_;
    };
    using replace_with_fffd = replace_with<0xfffd>;

    struct throw_exception {};

    struct little_endian {};
    struct big_endian {};
    using native_endian = UTF_PLATFORM_ENDIANESS;

    namespace detail
    {
        template <typename impl, typename policy, typename enable = void>
        struct onerror_handler;

        template <typename impl, typename policy, typename enable = void>
        struct to_handler;

        template <typename impl, typename policy, typename enable = void>
        struct from_handler;
    }

    template <typename T>
    struct onerror
    {
        using type = T;
        template <typename...Ts>
        using handler = detail::onerror_handler<Ts...>;
    };


    template <typename T>
    struct from
    {
        using type = T;
        template <typename...Ts>
        using handler =  detail::from_handler<Ts...>;
    };

    template <typename T>
    struct to
    {
        using type = T;
        template <typename...Ts>
        using handler = detail::to_handler<Ts...>;
    };

    constexpr uint32_t utf_not_a_char = 0x0000FFFE; // { 0x00, 0x00, 0xFF, 0xFE };
    constexpr uint32_t utf_replacement_char = 0x0000FFFD; //{ 0x00,0x00,0xFF,0xFD };

   template <
        typename to_endianness,
        typename from_endianness = native_endian,
        typename std::enable_if< std::is_convertible<from_endianness, to_endianness>::value,bool>::type = true
    >
    uint32_t codepoint_as(uint32_t codepoint)
    {
       return codepoint;
    }

   template <
       typename to_endianness,
       typename from_endianness = native_endian,
       typename std::enable_if< !std::is_convertible<from_endianness, to_endianness>::value, bool>::type = true
   >
       uint32_t codepoint_as(uint32_t codepoint)
   {
       return UTF_PLATFORM_BSWAP32(codepoint);
   }

   template <
       typename to_endianness,
       typename from_endianness = native_endian,
       typename std::enable_if< std::is_convertible<from_endianness, to_endianness>::value, bool>::type = true
   >
       uint16_t codeunit_as(uint16_t codeunit)
   {
       return codeunit;
   }

   template <
       typename to_endianness,
       typename from_endianness = native_endian,
       typename std::enable_if< !std::is_convertible<from_endianness, to_endianness>::value, bool>::type = true
   >
       uint32_t codeunit_as(uint16_t codeunit)
   {
       return UTF_PLATFORM_BSWAP16(codeunit);
   }

   constexpr uint8_t  utf8_bom[] = { 0xEF,0xBB,0xBF };
   constexpr uint8_t  utf16_little_endian_bom[] = { 0xFF,0xFE };
   constexpr uint8_t  utf16_big_endian_bom[] = { 0xFE,0xFF };
   constexpr uint8_t  utf32_little_endian_bom[] = { 0xFF,0xFE,0x00,0x00 };
   constexpr uint8_t  utf32_big_endian_bom[] = { 0x00,0x00,0xFE,0xFF };

   enum class bom_type
   {
       none,
       utf8,
       utf16_little_endian,
       utf16_big_endian,
       utf32_little_endian,
       utf32_big_endian,
   };
}


namespace utf
{
    struct utf_iterator_access
    {
        template <typename impl_type>
        static void increment(impl_type& impl)
        {
            impl.increment();
        }

        template <typename impl_type>
        static void decrement(impl_type& impl)
        {
            impl.decrement();
        }

        template <typename impl_type>
        static bool equal(const impl_type& impl, const impl_type& other)
        {
            return impl.equal(other);
        }

        template <typename impl_type>
        static typename impl_type::reference dereference(const impl_type& impl)
        {
            return impl.dereference();
        }

    };

    template <
        typename impl,
        typename base_iterator,
        typename impl_category,
        typename fn_category,
        typename value_type_,
        typename reference_type_,
        typename enable=void
    >
     struct utf_iterator_base
    {
        using iterator_category = impl_category;
        using value_type = value_type_;
        using difference_type = std::ptrdiff_t;
        using pointer = typename std::add_pointer<value_type_>::type;
        using reference = reference_type_;

    protected:
        impl& derived()
        {
            return *static_cast<impl*>(this);
        }

        const impl& derived() const
        {
            return *static_cast<const impl*>(this);
        }
    public:


        reference operator*() const
        {
            return utf_iterator_access::dereference(derived());
        }

        friend bool operator==(const impl& first,const impl& second)
        {
            return utf_iterator_access::equal(first,second);
        }

        friend bool operator!=(const impl& first, const impl& second)
        {
            return !utf_iterator_access::equal(first, second);
        }

        impl& operator++()
        {
            utf_iterator_access::increment(this->derived());
            return this->derived();
        }


        template <typename iterator_type = impl>
        impl operator++(int)
        {
            impl result(this->derived());
            ++(*this);
            return result;
        }
    };

    template <typename iterator_type>
    struct resultof_postfix_increment : iterator_type
    {
        template<typename T, typename enable = void>
        struct resultof_postfix_increment_impl 
        { using type = iterator_type; };

        template<typename T>
        struct resultof_postfix_increment_impl<T, mp::void_t<typename T::postfix_increment_result_type>>
        {
            using type = typename T::postfix_increment_result_type;
        };

        using type = typename resultof_postfix_increment_impl<resultof_postfix_increment>::type;
    };

    template <typename impl, typename base_iterator, typename impl_category, typename fn_category,  typename value, typename reference>
    struct utf_iterator_base < impl, base_iterator, impl_category,fn_category,value, reference,
        detail::enable_if_category<fn_category, std::input_iterator_tag>
    > : utf_iterator_base < impl, base_iterator, impl_category, void, value, reference >
    {

        impl& operator++()
        {
            utf_iterator_access::increment(this->derived());
            return this->derived();
        }


        template <typename iterator_type=impl>
        auto operator++(int)
        {
            typename resultof_postfix_increment<impl>::type result(this->derived());
            ++(*this);
            return result;
        }
    };

    template <typename impl, typename base_iterator, typename impl_category,typename fn_category,typename value, typename reference>
    struct utf_iterator_base < impl, base_iterator, impl_category, fn_category, value, reference,
        detail::enable_if_category<fn_category, std::bidirectional_iterator_tag, std::random_access_iterator_tag>
    > : utf_iterator_base<impl, base_iterator, impl_category, void, value, reference>
        
    {
        impl& operator--()
        {
            utf_iterator_access::decrement(this->derived());
            return this->derived();
        }

        impl operator--(int)
        {
            impl result(this->derived());
            --(*this);
            return result;
        }
    };

    template <typename impl, typename base_iterator, typename impl_category, typename value, typename reference>
    struct utf_iterator : utf_iterator_base<impl,base_iterator,impl_category, impl_category,value,reference>{};
}



namespace utf
{
    template <typename base_iterator, typename... policies>
    class utf8_to_utf32_iterator;

    UTF_POLICY_DEFAULT(utf8_to_utf32_iterator, onerror, throw_exception);
    UTF_POLICY_DEFAULT(utf8_to_utf32_iterator, to, little_endian);

    namespace detail {

        template<uint32_t value, typename base_iterator,typename... Ts>
        struct onerror_handler< utf8_to_utf32_iterator<base_iterator,Ts...>, replace_with<value>, void>
        {
            static uint32_t apply(const std::string& errmsg) {
                return value;
            }
        };

        template <typename base_iterator,typename... Ts>
        struct onerror_handler< utf8_to_utf32_iterator<base_iterator,Ts...>, throw_exception, void>
        {
            static uint32_t apply(const std::string& errmsg) { throw std::runtime_error(errmsg); }

        };

        template<typename endianness, typename base_iterator,typename... Ts>
        struct to_handler < utf8_to_utf32_iterator<base_iterator,Ts...>, endianness>
        {
            static uint32_t apply(uint32_t codepoint) {
                return codepoint_as<endianness>(codepoint);
            }
        };
    }

    /*** common iterator implementation */
    template <typename impl, typename base_iterator, typename impl_catagory, typename value_type, typename reference_type, typename... policies>
    struct utf8_to_utf32_iterator_base :
        utf_iterator< impl, base_iterator, impl_catagory, value_type, reference_type>
    {
     protected:


        using base_type = typename utf8_to_utf32_iterator_base::utf_iterator;

        friend utf_iterator_access;

        using error_policy = detail::select_policy_handler<impl, onerror, policies...>;
        using to_policy    = detail::select_policy_handler<impl, to, policies...>;
        
        utf8_to_utf32_iterator_base() {};
        
        utf8_to_utf32_iterator_base(base_iterator pos, const base_iterator end) :
            pos_(pos),
            end_(end),
            current_(0),
            replace_(0),
            size_(5)
        {};

        static inline size_t codepoint_bytes(uint8_t value)
        {
            if (value < 0x80)
                return 1;

            for (size_t i = 0; i < 5; ++i)
                if (!(value & 0x80 >> i))
                    return i;

            return 0;
        }

        template < typename onerror_policy, typename to_policy>
        static uint32_t read_next_impl(base_iterator& pos, const base_iterator& end, onerror_policy, to_policy, int8_t& replace, int8_t& consumed)
        {

            uint8_t start_byte = *pos++; ++consumed;

            if ((start_byte & 0xC0) == 0x80) {
                return onerror_policy::apply("invalid start byte decoding utf8 string.");
            }

            size_t byte_count = codepoint_bytes(start_byte);
            if (!byte_count)
                return onerror_policy::apply("invalid byte count decoding utf8 string.");

            uint32_t codepoint = start_byte;
            for (size_t i = 1; i < byte_count; ++i, ++pos, ++consumed)
            {
                if (pos == end)
                    return (replace = static_cast<int8_t>(i-1),onerror_policy::apply("unexpected eof deocding of ut8 string."));
                else if ((*pos & 0xC0) != 0x80)
                    return (replace = static_cast<int8_t>(i-1),onerror_policy::apply("invalid byte deocding of ut8 string. expected continuation byte."));

                codepoint <<= 6;
                codepoint |= static_cast<uint32_t>(*pos) & 0x3F;
            }

            static const uint32_t masks[] =
            {
               0x7Fu,
               0x7FFu,
               0xFFFFu,
               0x1FFFFFu,
            };

            return codepoint & masks[byte_count - 1];
        }

        template < typename onerror_policy, typename to_policy>
        static uint32_t read_next(base_iterator& pos, const base_iterator& end, onerror_policy, to_policy, int8_t& replace,int8_t& consumed)
        {
            consumed = 0;
            replace  = 0;
            return to_policy::apply(read_next_impl(pos, end, onerror_policy(), to_policy(), replace, consumed));
        }

        template < typename onerror_policy, typename to_policy>
        static uint32_t read_prev_impl(base_iterator& pos, const  base_iterator& start, onerror_policy, to_policy, int8_t& replace, int8_t& consumed)
        {
            base_iterator prev(pos);
            base_iterator end(pos);
            
            size_t byte_count = 1;
            for (--prev; byte_count < 5; ++byte_count, --prev)
            {
                if ((*prev & 0xC0) != 0x80)
                {
                    if (codepoint_bytes(*prev) == byte_count)
                        break;

                    return (--pos, onerror_policy::apply("invalid byte count decoding utf8 string."));
                }

                if (prev == start)
                    return (--pos, onerror_policy::apply("unexpected sof deocding of ut8 string."));
            }

            pos = prev;

            return read_next_impl(prev, end, onerror_policy(), to_policy(), replace, consumed);
        }

        template < typename onerror_policy, typename to_policy>
        static uint32_t read_prev(base_iterator& pos, const base_iterator& end, onerror_policy, to_policy, int8_t& replace, int8_t& consumed)
        {
            consumed = 0;
            replace = 0;
            return to_policy::apply(read_prev_impl(pos, end, onerror_policy(), to_policy(), replace, consumed));
        }

        typename base_type::reference dereference() const
        {
            if (!replace_ && read_required())
            {
                base_iterator pos(pos_);
                read(pos);
            }
            return this->current_;
        }

        bool equal(const impl& other) const
        {
            return pos_ == other.pos_;
        }

        void increment()
        {
            if (replace_ > 0)
                return (void)--replace_;

            if (read_required())
                read(pos_);
            else
                std::advance(pos_, size_);

            size_ = 5;
        }

        bool read_required() const
        {
            return size_ == 5;
        }

        void read(base_iterator& pos) const
        {
            current_ = read_next(
                pos,
                end_,
                error_policy(), to_policy(),
                replace_,
                size_
            );

        }

        mutable base_iterator pos_;
        base_iterator end_;

        mutable uint32_t current_;
        mutable int8_t size_;
        mutable int8_t replace_;
        

    };

    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf8_to_utf32_iterator_impl;

    /*** input iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf8_to_utf32_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< base_category, std::input_iterator_tag>
        , policies...> :
        utf8_to_utf32_iterator_base<impl, base_iterator, std::input_iterator_tag, uint32_t, const uint32_t&, policies...>
    {

    protected:
        using base_type = typename utf8_to_utf32_iterator_impl::utf8_to_utf32_iterator_base;
        friend utf_iterator_access;

        utf8_to_utf32_iterator_impl()
        {}

        utf8_to_utf32_iterator_impl(base_iterator pos, const base_iterator end) :
            base_type(pos,end)
        {
            this->size_ = 0;
            if (pos != end)
                this->read(this->pos_);
        }

        void increment()
        {
            if (this->replace_ > 0)
                return (void)--this->replace_;

            if (this->pos_ != this->end_)
                this->read(this->pos_);
            else 
                this->size_ = 0;
        }

        bool equal(const impl& other) const
        {
            return other.pos_ == this->pos_ && other.size_ == this->size_ && this->replace_ == other.replace_;
        }

        typename base_type::reference dereference() const
        {
            return this->current_;
        }

    };

    /***  forward iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf8_to_utf32_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< base_category, std::forward_iterator_tag>, policies...> 
        : utf8_to_utf32_iterator_base<impl, base_iterator, std::forward_iterator_tag, uint32_t, const uint32_t&, policies...>
    {
    protected:
        using base_type = typename utf8_to_utf32_iterator_impl::utf8_to_utf32_iterator_base;
        using base_type::base_type;
    };

    /*** bidirectional & random access iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf8_to_utf32_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category<base_category, std::bidirectional_iterator_tag, std::random_access_iterator_tag>, policies...>
        : utf8_to_utf32_iterator_base<impl, base_iterator, std::bidirectional_iterator_tag, uint32_t, const uint32_t&, policies...>
    {
    protected:
        friend utf_iterator_access;

        using base_type = typename utf8_to_utf32_iterator_impl::utf8_to_utf32_iterator_base;

        using typename base_type::error_policy;
        using typename base_type::to_policy;

        utf8_to_utf32_iterator_impl()
        {}

        utf8_to_utf32_iterator_impl(base_iterator pos, base_iterator start, base_iterator end) :
            base_type(pos,end),
            start_(start)
        {}

        void decrement()
        {
            if (this->replace_ < 0)
                return (void)++this->replace_;

            this->derived().current_ = this->read_prev(
                this->pos_,
                this->start_,
                error_policy(), to_policy(),
                this->replace_,
                this->size_
            );
        }

        base_iterator start_;
    };

    template <typename base_iterator, typename... policies>
    class utf8_to_utf32_iterator :
        public utf8_to_utf32_iterator_impl< 
            utf8_to_utf32_iterator<base_iterator, policies...>, 
            base_iterator, 
            typename std::iterator_traits<base_iterator>::iterator_category, 
            void, policies...
        >
    {
        using impl_type = typename utf8_to_utf32_iterator::utf8_to_utf32_iterator_impl;

        friend typename impl_type::base_type;

    public:

        utf8_to_utf32_iterator() {};

        template< typename category_type=typename std::iterator_traits<base_iterator>::iterator_category,
            typename bidirectional_or_random_access=detail::enable_if_category<category_type,std::bidirectional_iterator_tag,std::random_access_iterator_tag>>
        utf8_to_utf32_iterator(base_iterator pos, base_iterator start, base_iterator end) : impl_type(pos, start, end) {}

        template< typename category_type=typename std::iterator_traits<base_iterator>::iterator_category,
            typename input_or_forward=detail::enable_if_category<category_type, std::input_iterator_tag, std::forward_iterator_tag>>
        utf8_to_utf32_iterator(base_iterator pos, base_iterator end) : impl_type(pos, end) {}
    };

    template <typename... policies,typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::input_iterator_tag, std::random_access_iterator_tag> = true>
    auto make_utf8_to_utf32_iterator(base_iterator begin, base_iterator end)
    {
        return utf8_to_utf32_iterator<base_iterator, policies...>{begin, begin,end};
    }

    template <typename... policies, typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::input_iterator_tag, std::forward_iterator_tag> = true>
    auto make_utf8_to_utf32_iterator(base_iterator begin, base_iterator end)
    {
        return utf8_to_utf32_iterator<base_iterator, policies...>{begin, end};
    }
}

 

namespace utf
{

    template <typename base_iterator, typename... policies>
    class utf32_to_utf8_iterator;

    UTF_POLICY_DEFAULT(utf32_to_utf8_iterator, onerror, throw_exception);
    UTF_POLICY_DEFAULT(utf32_to_utf8_iterator, from,    little_endian);

    namespace detail {

        template<uint32_t value,typename base_iterator, typename... Ts>
        struct onerror_handler< utf32_to_utf8_iterator<base_iterator,Ts...>, replace_with<value>, void>
        {
            static uint32_t apply(const std::string& errmsg,uint32_t& codepoint) {
                return codepoint = value;
            }
        };

        template <typename base_iterator,typename... Ts>
        struct onerror_handler< utf32_to_utf8_iterator<base_iterator,Ts...>, throw_exception, void>
        {
            static uint32_t apply(const std::string& errmsg, uint32_t& codepoint) {
                throw std::runtime_error(errmsg); 
            }

        };

        template<typename endianness,typename base_iterator, typename... Ts>
        struct from_handler < utf32_to_utf8_iterator<base_iterator,Ts...>, endianness>
        {
            static uint32_t apply(uint32_t codepoint) {
                return codepoint_as<native_endian, endianness>(codepoint);
            }
        };
    }

    /*** common iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename value_type, typename reference_type, typename... policies>
    struct utf32_to_utf8_iterator_base :
        utf_iterator< impl, base_iterator, base_category, value_type, reference_type>
    {
        friend utf_iterator_access;
    protected:

        utf32_to_utf8_iterator_base() {};
        utf32_to_utf8_iterator_base(base_iterator pos) :
            current_{ 0 },
            size_(0),
            index_(4),
            pos_(pos)
        {}

        using error_policy = detail::select_policy_handler<impl, onerror, policies...>;
        using from_policy = detail::select_policy_handler<impl, from, policies...>;

        template <typename onerror_policy, typename from_policy>
        static size_t read_next(base_iterator& pos,  uint8_t* bytes,onerror_policy, from_policy)
        {
            uint32_t codepoint = from_policy::apply(*pos++);

            if (codepoint > 0x10FFFF)
                onerror_policy::apply("error encoding utf8 string, invalid codepoint.", codepoint);

            if (codepoint <= 0x007F)
            {
                bytes[0] = static_cast<uint8_t>(codepoint);
                bytes[1] = 0;
                bytes[2] = 0;
                bytes[3] = 0;

                return 1;
            }
            if (codepoint <= 0x07FF)
            {
                bytes[0] = static_cast<uint8_t>((codepoint >> 6 & 0x1F) | 0xC0);
                bytes[1] = static_cast<uint8_t>((codepoint & 0x3F) | 0x80);
                bytes[2] = 0;
                bytes[3] = 0;
                return 2;
            }

            if (codepoint <= 0xFFFF)
            {
                bytes[0] = static_cast<uint8_t>((codepoint >> 12 & 0x0F) | 0xE0);
                bytes[1] = static_cast<uint8_t>((codepoint >> 6 & 0x3F) | 0x80);
                bytes[2] = static_cast<uint8_t>((codepoint & 0x3F) | 0x80);
                bytes[3] = 0;
                return 3;
            }
            if (codepoint <= 0x10FFFF)
            {
                bytes[0] = static_cast<uint8_t>((codepoint >> 18 & 0x0E) | 0xF0);
                bytes[1] = static_cast<uint8_t>((codepoint >> 12 & 0x3F) | 0x80);
                bytes[2] = static_cast<uint8_t>((codepoint >> 6 & 0x3F) | 0x80);
                bytes[3] = static_cast<uint8_t>((codepoint & 0x3F) | 0x80);
                return 4;
            }
            return 0;
        }

        template < typename onerror_policy, typename from_policy>
        static size_t read_prev(base_iterator& pos, uint8_t* bytes,onerror_policy, from_policy)
        {
            base_iterator prev(--pos);
            return read_next(prev,bytes, onerror_policy(), from_policy());
        }

        using base_type = typename utf32_to_utf8_iterator_base::utf_iterator;


        typename base_type::reference dereference() const
        {
            read_pending();
            return this->current_[this->index_];
        }

        bool equal(const impl& other) const
        {
            return this->pos_ == other.pos_ && ((this->index_ | other.index_) & 3) == 0;
        }

        void increment()
        {
            read_pending();
            ++this->index_;

            if (this->index_ == this->size_)
                this->index_ = 4;
        }

        inline void read_pending() const
        {
            if (this->index_ == 4) {
                this->size_ = read_next(this->pos_, this->current_, error_policy(), from_policy());
                this->index_ = 0;
            }
        }

        mutable uint8_t current_[4];
        mutable size_t index_;
        mutable size_t size_;
        mutable base_iterator pos_;

    };

    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf8_iterator_impl;

    /*** input iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf8_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< typename std::iterator_traits<base_iterator>::iterator_category, std::input_iterator_tag>
        , policies...> :
        utf32_to_utf8_iterator_base<impl, base_iterator, base_category, uint8_t, const uint8_t&, policies...>
    {

    protected:

        struct postfix_increment_result_type {

            postfix_increment_result_type(const impl& iter) :
                value_(*iter)
            {}

            uint8_t operator*() const { return value_; }

        private:
            uint8_t value_;
        };


        friend utf_iterator_access;


        using base_type = typename utf32_to_utf8_iterator_impl::utf32_to_utf8_iterator_base;
        using base_type::base_type;




        typename base_type::reference dereference() const
        {
            this->read_pending();
            return this->current_[this->index_];
        }

        bool equal(const impl& other) const
        {
            return this->pos_ == other.pos_ && ((this->index_ | other.index_) & 3) == 0;
        }

        void increment()
        {
            this->read_pending();
            ++this->index_;

            if (this->index_ == this->size_)
                this->index_ = 4;
        }


        

    };

    /*** forward iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf8_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< typename std::iterator_traits<base_iterator>::iterator_category, std::forward_iterator_tag>
        , policies...> :
        utf32_to_utf8_iterator_base<impl, base_iterator, base_category, uint8_t, const uint8_t&, policies...>
    {
    protected:
        using base_type = typename utf32_to_utf8_iterator_impl::utf32_to_utf8_iterator_base;
        using base_type::base_type;

    };


    /*** bidirectional & random access iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf8_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category<  typename std::iterator_traits<base_iterator>::iterator_category, std::bidirectional_iterator_tag, std::random_access_iterator_tag>
        , policies...> :
        utf32_to_utf8_iterator_base<impl, base_iterator, std::bidirectional_iterator_tag, uint8_t, const uint8_t&, policies...>
    {
    protected:
        friend utf_iterator_access;

        using base_type = typename utf32_to_utf8_iterator_impl::utf32_to_utf8_iterator_base;
        using base_type::base_type;
        using typename base_type::error_policy;
        using typename base_type::from_policy;
        
        void decrement()
        {
            if ((this->index_ & 3) == 0) {
                this->size_ = base_type::read_prev(this->pos_, this->current_, error_policy(), from_policy());
                this->index_ = this->size_;
            }
            --this->index_;
        }
    };

    template <typename base_iterator, typename... policies>
    class utf32_to_utf8_iterator :
        public utf32_to_utf8_iterator_impl<
        utf32_to_utf8_iterator<base_iterator, policies...>,
        base_iterator,
        typename std::iterator_traits<base_iterator>::iterator_category,
        void, policies...
        >
    {

        using impl_type = typename utf32_to_utf8_iterator::utf32_to_utf8_iterator_impl;
    public:
        utf32_to_utf8_iterator() {};
        utf32_to_utf8_iterator(base_iterator pos) : impl_type(pos) {};

    };

    template <typename... policies, typename base_iterator>
    auto make_utf32_to_utf8_iterator(base_iterator iter)
    {
        return utf32_to_utf8_iterator<base_iterator, policies...>{iter};
    }
}


 

namespace utf
{
    template <typename base_iterator, typename... policies>
    class utf16_to_utf32_iterator;

    UTF_POLICY_DEFAULT(utf16_to_utf32_iterator, onerror, throw_exception);
    UTF_POLICY_DEFAULT(utf16_to_utf32_iterator, to, little_endian);
    UTF_POLICY_DEFAULT(utf16_to_utf32_iterator, from, little_endian);

    namespace detail {

        template<uint32_t value,typename base_iterator, typename... Ts>
        struct onerror_handler< utf16_to_utf32_iterator<base_iterator,Ts...>, replace_with<value>, void>
        {
            static uint32_t apply(const std::string& errmsg) {
                return value;
            }
        };

        template <typename base_iterator,typename... Ts>
        struct onerror_handler< utf16_to_utf32_iterator<base_iterator,Ts...>, throw_exception, void>
        {
            static uint32_t apply(const std::string& errmsg) { throw std::runtime_error(errmsg); }

        };

        template<typename endianness,typename base_iterator, typename... Ts>
        struct to_handler < utf16_to_utf32_iterator<base_iterator,Ts...>, endianness>
        {
            static uint32_t apply(uint32_t codepoint) {
                return codepoint_as<endianness>(codepoint);
            }
        };

        template<typename endianness,typename base_iterator, typename... Ts>
        struct from_handler < utf16_to_utf32_iterator<base_iterator,Ts...>, endianness>
        {
            static uint16_t apply(uint16_t codeunit) {
                return codeunit_as<native_endian,endianness>(codeunit);
            }
        };

    }

    /*** common iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename value_type, typename reference_type, typename... policies>
    struct utf16_to_utf32_iterator_base :
        utf_iterator< impl, base_iterator, base_category, value_type, reference_type>
    {

    protected:
        utf16_to_utf32_iterator_base() {};

        using base_type = typename utf16_to_utf32_iterator_base::utf_iterator;

        friend utf_iterator_access;

        using error_policy = detail::select_policy_handler<impl, onerror, policies...>;
        using from_policy = detail::select_policy_handler<impl, from, policies...>;
        using to_policy = detail::select_policy_handler<impl, to, policies...>;

        template < typename onerror_policy,typename from_policy, typename to_policy>
        static uint32_t read_next_impl(base_iterator& pos, const base_iterator& end, onerror_policy, from_policy,to_policy, int8_t& replace, int8_t& consumed)
        {
            uint32_t value = from_policy::apply(*pos++); ++consumed;

            if ((value & 0xFC00) == 0xD800)
            {
                if (pos == end)
                    return (replace=0,onerror_policy::apply("unexpected end of utf16 string"));

                uint32_t lowsurrogate = from_policy::apply(*pos); 

                if ((lowsurrogate & 0xFC00) != 0xDC00)
                    return (replace=0,onerror_policy::apply("unpaired surrogate found"));

                value  = (value - 0xD7C0) << 10;
                value |= (lowsurrogate & 0x3FF);

                ++pos; ++consumed;

            }
            else {

                if ((value & 0xF800) == 0xD800)
                    return (replace=0,onerror_policy::apply("unpaired surrogate found"));

            }
            return value;
        }

        template <typename onerror_policy, typename from_policy,typename to_policy>
        static uint32_t read_next(base_iterator& pos, const base_iterator& end, onerror_policy, from_policy,to_policy, int8_t& replace, int8_t& consumed)
        {
            consumed = 0;
            replace = 0;
            return to_policy::apply(read_next_impl(pos, end, onerror_policy(), from_policy(),to_policy(), replace, consumed));
        }

        template <  typename onerror_policy, typename from_policy, typename to_policy>
        static uint32_t read_prev_impl(base_iterator& pos,const base_iterator& start, onerror_policy, from_policy, to_policy, int8_t& replace, int8_t& consumed)
        {
            base_iterator prev(pos);
            base_iterator end(pos);

            uint16_t value = from_policy::apply(*--prev);

            if ((value & 0xFC00) == 0xDC00) {
                if (prev == start)
                    return (--pos, onerror_policy::apply("unexpected start of utf16 string"));
                value = *--prev;
                if ((value & 0xFC00) != 0xD800)
                    return (--pos, onerror_policy::apply("unexpected start of utf16 string"));
            }
            else if ((value & 0xF800) == 0xD800)
                return (--pos,onerror_policy::apply("unpaired surrogate found"));
            
            pos = prev;
            return read_next_impl(prev, end, onerror_policy(),from_policy(), to_policy(), replace, consumed);
        }

        template < typename onerror_policy, typename from_policy, typename to_policy>
        static uint32_t read_prev(base_iterator& pos, const base_iterator& start,onerror_policy, from_policy, to_policy, int8_t& replace, int8_t& consumed)
        {
            consumed = 0;
            replace = 0;
            return to_policy::apply(read_prev_impl(pos, start, onerror_policy(), from_policy(), to_policy(),replace,consumed));
        }

        utf16_to_utf32_iterator_base(base_iterator pos,  const base_iterator end) :
            pos_(pos),
            end_(end),
            current_(0),
            replace_(0),
            size_(2)
        {};

        typename base_type::reference dereference() const
        {
            if (!replace_ && read_required())
            {
                base_iterator pos(pos_);
                read(pos);
            }
            return this->current_;
        }

        bool equal(const impl& other) const
        {
            return pos_ == other.pos_;
        }

        void increment()
        {
            if (replace_ > 0)
                return (void)--replace_;

            if (read_required())
                read(pos_);
            else
                std::advance(pos_, size_);

            size_ = 2;
        }

        bool read_required() const
        {
            return size_ == 2;
        }

        void read(base_iterator& pos) const
        {
            current_ = this->read_next(
                pos,
                end_,
                error_policy(), from_policy(), to_policy(),
                replace_,
                size_
            );

        }

        mutable base_iterator pos_;
        base_iterator end_;

        mutable uint32_t current_;
        mutable int8_t size_;
        mutable int8_t replace_;
    };

    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf16_to_utf32_iterator_impl;

    ///*** input iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf16_to_utf32_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< base_category, std::input_iterator_tag >
        , policies...> :
        utf16_to_utf32_iterator_base<impl, base_iterator, base_category, uint32_t, const uint32_t&, policies...>
    {

    protected:
        using base_type = typename utf16_to_utf32_iterator_impl::utf16_to_utf32_iterator_base;
        friend utf_iterator_access;

        utf16_to_utf32_iterator_impl() {};

        utf16_to_utf32_iterator_impl(base_iterator pos, const base_iterator end) :
            base_type(pos, end)
        {
            this->size_ = 0;
            if (pos != end)
                this->read(this->pos_);
        }

        void increment()
        {
            if (this->replace_ > 0)
                return (void)--this->replace_;

            if (this->pos_ != this->end_)
                this->read(this->pos_);
            else
                this->size_ = 0;
        }

        bool equal(const impl& other) const
        {
            return other.pos_ == this->pos_ && other.size_ == this->size_ && this->replace_ == other.replace_;
        }

        typename base_type::reference dereference() const
        {
            return this->current_;
        }
    };

    /***  forward iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf16_to_utf32_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< base_category, std::forward_iterator_tag>, policies...>
        : utf16_to_utf32_iterator_base<impl, base_iterator, base_category, uint32_t, const uint32_t&, policies...>
    {

    protected:
        using base_type = typename utf16_to_utf32_iterator_impl::utf16_to_utf32_iterator_base;
        using base_type::base_type;

    };

    /*** bidirectional & random access iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf16_to_utf32_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category<
        base_category, std::bidirectional_iterator_tag, std::random_access_iterator_tag>
        , policies...> :
        utf16_to_utf32_iterator_base<impl, base_iterator, std::bidirectional_iterator_tag, uint32_t, const uint32_t&, policies...>
    {
    protected:

        using base_type = typename utf16_to_utf32_iterator_impl::utf16_to_utf32_iterator_base;
        using typename base_type::error_policy;
        using typename base_type::to_policy;
        using typename base_type::from_policy;
        
        friend utf_iterator_access;

        utf16_to_utf32_iterator_impl() {};

        utf16_to_utf32_iterator_impl(base_iterator pos,const base_iterator start, const base_iterator end) :
            base_type(pos,end),
            start_(start)
        {};

        void decrement()
        {
            if (this->replace_ < 0)
                return (void)++this->replace_;

            this->derived().current_ = this->read_prev(
                this->pos_,
                this->start_,
                error_policy(), from_policy(),to_policy(),
                this->replace_,
                this->size_
            );
        }

        base_iterator start_;

    };

    template <typename base_iterator, typename... policies>
    class utf16_to_utf32_iterator :
        public utf16_to_utf32_iterator_impl<
        utf16_to_utf32_iterator<base_iterator, policies...>,
        base_iterator,
        typename std::iterator_traits<base_iterator>::iterator_category,
        void, policies...
        >
    {
        using impl_type = typename utf16_to_utf32_iterator::utf16_to_utf32_iterator_impl;

        friend typename impl_type::base_type;


    public:
        utf16_to_utf32_iterator() {};

        template<typename category_type=typename std::iterator_traits<base_iterator>::iterator_category,
            typename bidirectional_or_random_access = detail::enable_if_category<category_type, std::bidirectional_iterator_tag, std::random_access_iterator_tag>>
        utf16_to_utf32_iterator(base_iterator pos, base_iterator start, base_iterator end) : impl_type(pos, start, end) {}

        template<typename category_type=typename std::iterator_traits<base_iterator>::iterator_category,
            typename input_or_forward = detail::enable_if_category<category_type, std::input_iterator_tag, std::forward_iterator_tag>>
        utf16_to_utf32_iterator(base_iterator pos, base_iterator end) : impl_type(pos, end) {}

    };

    template <typename... policies, typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::bidirectional_iterator_tag, std::random_access_iterator_tag> = true>
    auto make_utf16_to_utf32_iterator(base_iterator begin, base_iterator end)
    {
        return utf16_to_utf32_iterator<base_iterator, policies...>{begin, begin, end};
    }

    template <typename... policies, typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::input_iterator_tag, std::forward_iterator_tag> = true>
    auto make_utf16_to_utf32_iterator(base_iterator begin, base_iterator end)
    {
        return utf16_to_utf32_iterator<base_iterator, policies...>{begin, end};
    }

}


namespace utf
{

    template <typename base_iterator, typename... policies>
    class utf32_to_utf16_iterator;

    UTF_POLICY_DEFAULT(utf32_to_utf16_iterator, onerror, throw_exception);
    UTF_POLICY_DEFAULT(utf32_to_utf16_iterator, from, little_endian);
    UTF_POLICY_DEFAULT(utf32_to_utf16_iterator, to, little_endian);

    namespace detail {

        template<uint32_t value, typename base_iterator, typename... Ts>
        struct onerror_handler< utf32_to_utf16_iterator<base_iterator,Ts...>, replace_with<value>, void>
        {
            static uint32_t apply(const std::string& errmsg, uint32_t& codepoint) {
                return codepoint = value;
            }
        };

        template <typename base_iterator, typename... Ts>
        struct onerror_handler< utf32_to_utf16_iterator<base_iterator,Ts...>, throw_exception, void>
        {
            static uint32_t apply(const std::string& errmsg, uint32_t& codepoint) {
                throw std::runtime_error(errmsg);
            }
        };

        template<typename endianness,typename base_iterator,  typename... Ts>
        struct to_handler < utf32_to_utf16_iterator<base_iterator,Ts...>, endianness>
        {
            static uint16_t apply(uint16_t codeunit) {
                return codeunit_as<endianness>(codeunit);
            }
        };

        template<typename endianness,typename base_iterator, typename... Ts>
        struct from_handler < utf32_to_utf16_iterator<base_iterator,Ts...>, endianness>
        {
            static uint32_t apply(uint32_t codepoint) {
                return codepoint_as<native_endian, endianness>(codepoint);
            }
        };
    }

    /*** common iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename value_type, typename reference_type, typename... policies>
    struct utf32_to_utf16_iterator_base :
        utf_iterator< impl, base_iterator, base_category, value_type, reference_type>
    {
        friend utf_iterator_access;
    protected:

        utf32_to_utf16_iterator_base() {};
        utf32_to_utf16_iterator_base(base_iterator pos) :
            current_{ 0 },
            size_(0),
            index_(2),
            pos_(pos)
        {}

        using error_policy = detail::select_policy_handler<impl, onerror, policies...>;
        using from_policy  = detail::select_policy_handler<impl, from, policies...>;
        using to_policy    = detail::select_policy_handler<impl, to, policies...>;

        template <typename onerror_policy, typename from_policy,typename to_policy>
        static size_t read_next(base_iterator& pos, uint16_t* bytes, onerror_policy, from_policy, to_policy)
        {

            uint32_t codepoint = from_policy::apply(*pos++);
            size_t size = 0;
            if (codepoint > 0x10FFFF)
                codepoint = onerror_policy::apply("error encoding utf16 string, invalid codepoint.", codepoint);
            

            if (codepoint >= 0x10000)
            {
                bytes[0] = static_cast<uint16_t>(codepoint >> 10) + 0xD7C0;
                bytes[1] = static_cast<uint16_t>(codepoint & 0x3FF) + 0xDC00;
                size = 2;
            }
            else
            {
                if ((codepoint & 0xFFFFF800) == 0xD800)
                    codepoint = onerror_policy::apply("error encoding utf16 string, invalid surrogate codepoint.", codepoint);

                bytes[0] = static_cast<uint16_t>(codepoint);
                bytes[1] = 0;
                size  = 1;
            }

            for (size_t i = 0; i < size; ++i)
                bytes[i] = to_policy::apply(bytes[i]);

            return size;
        }


        template <  typename onerror_policy, typename from_policy, typename to_policy>
        static size_t read_prev(base_iterator& pos, uint16_t* bytes, onerror_policy, from_policy,to_policy)
        {
            base_iterator prev(--pos);
            return read_next(prev, bytes, onerror_policy(), from_policy(), to_policy());
        }

        using base_type = typename utf32_to_utf16_iterator_base::utf_iterator;
        using base_type::base_type;

        typename base_type::reference dereference() const
        {
            read_pending();
            return this->current_[this->index_];
        }

        bool equal(const impl& other) const
        {
            return this->pos_ == other.pos_ && ((this->index_ | other.index_) & 1) == 0;
        }

        void increment()
        {
            read_pending();
            ++this->index_;

            if (this->index_ == this->size_)
                this->index_ = 2;
        }

        inline void read_pending() const
        {
            if (this->index_ == 2) {
                this->size_ = read_next(this->pos_, this->current_, error_policy(), from_policy(), to_policy());
                this->index_ = 0;
            }
        }

        mutable uint16_t current_[2];
        mutable size_t index_;
        mutable size_t size_;
        mutable base_iterator pos_;

    };

    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf16_iterator_impl;

    /*** input iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf16_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< typename std::iterator_traits<base_iterator>::iterator_category, std::input_iterator_tag>
        , policies...> :
        utf32_to_utf16_iterator_base<impl, base_iterator, base_category, uint16_t, const uint16_t&, policies...>
    {

    protected:

        struct postfix_increment_result_type {

            postfix_increment_result_type(const impl& iter) :
                value_(*iter)
            {}

            uint16_t operator*() const { return value_; }

        private:
            uint16_t value_;
        };


        friend utf_iterator_access;

        using base_type = typename utf32_to_utf16_iterator_impl::utf32_to_utf16_iterator_base;
        using base_type::base_type;

        typename base_type::reference dereference() const
        {
            this->read_pending();
            return this->current_[this->index_];
        }

        bool equal(const impl& other) const
        {
            return this->pos_ == other.pos_ && ((this->index_ | other.index_) & 1) == 0;
        }

        void increment()
        {
            this->read_pending();
            ++this->index_;

            if (this->index_ == this->size_)
                this->index_ = 2;
        }




    };

    /*** forward iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf16_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category< typename std::iterator_traits<base_iterator>::iterator_category, std::forward_iterator_tag>
        , policies...> :
        utf32_to_utf16_iterator_base<impl, base_iterator, base_category, uint16_t, const uint16_t&, policies...>
    {
    protected:
        using base_type = typename utf32_to_utf16_iterator_impl::utf32_to_utf16_iterator_base;
        using base_type::base_type;

    };

    /*** bidirectional & random access iterator implementation */
    template <typename impl, typename base_iterator, typename base_category, typename... policies>
    struct utf32_to_utf16_iterator_impl<impl, base_iterator, base_category,
        detail::enable_if_category<  typename std::iterator_traits<base_iterator>::iterator_category, std::bidirectional_iterator_tag, std::random_access_iterator_tag>
        , policies...> :
        utf32_to_utf16_iterator_base<impl, base_iterator, std::bidirectional_iterator_tag, uint16_t, const uint16_t&, policies...>
    {
    protected:
        friend utf_iterator_access;

        using base_type = typename utf32_to_utf16_iterator_impl::utf32_to_utf16_iterator_base;
        using base_type::base_type;
        using typename base_type::error_policy;
        using typename base_type::from_policy;
        using typename base_type::to_policy;
        
        void decrement()
        {
            if ((this->index_ & 1) == 0) {
                this->size_ = base_type::read_prev(this->pos_, this->current_, error_policy(), from_policy(), to_policy());
                this->index_ = this->size_;
            }
            --this->index_;
        }
    };

    template <typename base_iterator, typename... policies>
    class utf32_to_utf16_iterator :
        public utf32_to_utf16_iterator_impl<
        utf32_to_utf16_iterator<base_iterator, policies...>,
        base_iterator,
        typename std::iterator_traits<base_iterator>::iterator_category,
        void, policies...
        >
    {

        using impl_type = typename utf32_to_utf16_iterator::utf32_to_utf16_iterator_impl;
    public:
        utf32_to_utf16_iterator() {};
        utf32_to_utf16_iterator(base_iterator pos) : impl_type(pos) {};

    };

    template <typename... policies, typename base_iterator>
    auto make_utf32_to_utf16_iterator(base_iterator iter)
    {
        return utf32_to_utf16_iterator<base_iterator, policies...>{iter};
    }
}


 

namespace utf
{
    template <typename T,typename base_iterator>
    class stride_iterator;

    /*** common iterator implementation */
    template <typename T,typename impl, typename base_iterator, typename impl_catagory, typename value_type, typename reference_type>
    struct stride_iterator_base :
        utf_iterator< impl, base_iterator, impl_catagory, value_type, reference_type>
    {
    protected:

        using base_type = typename stride_iterator_base::utf_iterator;

        friend utf_iterator_access;

        stride_iterator_base() {};

        stride_iterator_base(base_iterator pos, const base_iterator end) :
            pos_(pos),
            end_(end),
            current_(0),
            pending_(true)
        {

        };


        typename base_type::reference dereference() const
        {
            if (pending_) {
                base_iterator pos(pos_);
                read_next(pos, end_);
            }

            return current_;
        }

        bool equal(const impl& other) const
        {
            return pos_ == other.pos_;
        }

        void increment()
        {
            for (size_t i = 0; i < sizeof(T); ++i,++pos_) {
                if (pos_ == end_)
                    throw std::runtime_error("invalid byte count, byte count must be divisbile by 2");
            }
            pending_ = true;
        }

        void read_next(base_iterator& pos, base_iterator end) const
        {
            current_ = 0;
            for (size_t i = 0; i < sizeof(T); ++i)
            {
                if (pos == end)
                    throw std::runtime_error("invalid byte count, byte count must be divisbile by 2");
                auto val = static_cast<typename std::make_unsigned<typename base_iterator::value_type>::type>(*pos++);
                current_ |=  val << (8 * i);
            }
        }

        mutable base_iterator pos_;
        mutable bool pending_;
        base_iterator end_;

        mutable T current_;
    };

    template <typename T,typename impl, typename base_iterator, typename base_category,typename enable=void>
    struct stride_iterator_impl;

    /*** input iterator implementation */
    template <typename T,typename impl, typename base_iterator, typename base_category>
    struct stride_iterator_impl<T,impl, base_iterator, base_category,
        detail::enable_if_category< base_category, std::input_iterator_tag >
    > : stride_iterator_base<T,impl, base_iterator, std::input_iterator_tag, T, const T&>
    {

    protected:
        using base_type = typename stride_iterator_impl::stride_iterator_base;
        friend utf_iterator_access;

        stride_iterator_impl()
        {}

        stride_iterator_impl(base_iterator pos, const base_iterator end) :
            base_type(pos, end)
        {
            if (pos != end)
            {
                this->read_next(this->pos_, this->end_);
                this->pending_ = false;
            } 
        }

        void increment()
        {
            if (this->pos_ != this->end_)
                this->read_next(this->pos_,this->end_);
            else
                this->pending_ = true;
        }

        bool equal(const impl& other) const
        {
            return this->pos_ == other.pos_ && this->pending_ == other.pending_;
        }

        typename base_type::reference dereference() const
        {
            return this->current_;
        }

    };

    /***  forward/input iterator implementation */
    template <typename T,typename impl, typename base_iterator, typename base_category>
    struct stride_iterator_impl<T,impl, base_iterator, base_category,
        detail::enable_if_category< base_category, std::forward_iterator_tag>>
        : stride_iterator_base<T,impl, base_iterator, base_category,T, const T&>
    {
    protected:
        using base_type = typename stride_iterator_impl::stride_iterator_base;
        using base_type::base_type;
    };

    /*** bidirectional & random access iterator implementation */
    template <typename T,typename impl, typename base_iterator, typename base_category>
    struct stride_iterator_impl<T,impl, base_iterator, base_category,
        detail::enable_if_category<base_category, std::bidirectional_iterator_tag, std::random_access_iterator_tag>
    > : stride_iterator_base<T,impl, base_iterator, std::bidirectional_iterator_tag, T, const T&>
    {
    protected:
        friend utf_iterator_access;

        using base_type = typename stride_iterator_impl::stride_iterator_base;

        stride_iterator_impl()
        {}

        stride_iterator_impl(base_iterator pos, base_iterator start, base_iterator end) :
            base_type(pos, end),
            start_(start)
        {}

        void decrement()
        {
            this->current_ = 0;
            for (size_t i = 0; i < sizeof(T); ++i, --this->pos_){
                if (this->pos_ == start_)
                    throw std::runtime_error("invalid byte count, byte count must be divisbile by 2");
            }
        }

        base_iterator start_;
    };

    template <typename T,typename base_iterator>
    class stride_iterator :
        public stride_iterator_impl<T,stride_iterator<T,base_iterator>,base_iterator,typename std::iterator_traits<base_iterator>::iterator_category>
    {
        using impl_type = typename stride_iterator::stride_iterator_impl;

        friend typename impl_type::base_type;

    public:

        stride_iterator() {};

        template<typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
            typename bidirectional_or_random_access = detail::enable_if_category<category_type, std::bidirectional_iterator_tag, std::random_access_iterator_tag>>
        stride_iterator(base_iterator pos, base_iterator start, base_iterator end) : impl_type(pos, start, end) {}

        template<typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
            typename input_or_forward = detail::enable_if_category<category_type, std::input_iterator_tag, std::forward_iterator_tag>>
        stride_iterator(base_iterator pos, base_iterator end) : impl_type(pos, end) {}
    };

    template <typename base_iterator>
    using stride_short_iterator = stride_iterator<uint16_t, base_iterator>;

    template <typename base_iterator>
    using stride_long_iterator = stride_iterator<uint32_t, base_iterator>;

    template<typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::bidirectional_iterator_tag, std::random_access_iterator_tag> = true>
    auto make_stride_long_iterator(base_iterator start, base_iterator end)
    {
        return stride_long_iterator<base_iterator>(start, start, end);
    }

    template<typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::input_iterator_tag, std::forward_iterator_tag> = true>
        auto make_stride_long_iterator(base_iterator start, base_iterator end)
    {
        return stride_long_iterator<base_iterator>(start, end);
    }

    template<typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::bidirectional_iterator_tag, std::random_access_iterator_tag> = true>
        auto make_stride_short_iterator(base_iterator start, base_iterator end)
    {
        return stride_short_iterator<base_iterator>(start, start, end);
    }

    template<typename base_iterator,
        typename category_type = typename std::iterator_traits<base_iterator>::iterator_category,
        detail::enable_if_category_b<category_type, std::input_iterator_tag, std::forward_iterator_tag> = true>
        auto make_stride_short_iterator(base_iterator start, base_iterator end)
    {
        return stride_short_iterator<base_iterator>(start, end);
    }

}
 


namespace utf
{
    template <typename T>
    struct result_type : mp::identity<T> {};

    namespace detail
    {

        template <typename T, typename = void>
        struct has_push_back : mp::bool_c<false>
        {};


        template <typename T>
        struct has_push_back<T, mp::void_t<decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))>>
            : mp::bool_c<true>{};


        template <typename T>
        using is_error_policy = mp::bool_c< is_instance_of_template<onerror, T>::value >;

        template <typename T>
        using is_result_type = mp::bool_c< has_begin_end<T>::value && has_value_type<T>::value>;


        struct _utf8;
        struct _utf16;
        struct _utf32;

        template <typename,typename>
        struct utf_conversion_traits;

        template <>
        struct utf_conversion_traits<_utf8, _utf16>
        {
            using iterators = mp::list<
                mp::wrap<utf8_to_utf32_iterator>,
                mp::wrap<utf32_to_utf16_iterator>>;
            using string_type = std::u16string;
        };

        template <>
        struct utf_conversion_traits<_utf8, _utf32>
        {
            using iterators = mp::list<mp::wrap<utf8_to_utf32_iterator>>;
            using string_type = std::u32string;
        };

        template <>
        struct utf_conversion_traits<_utf16, _utf8>
        {
            using iterators = mp::list<
                mp::wrap<utf16_to_utf32_iterator>,
                mp::wrap<utf32_to_utf8_iterator>>;
            using string_type = std::string;
        };

        template <>
        struct utf_conversion_traits<_utf16, _utf32>
        {
            using iterators = mp::list<mp::wrap<utf16_to_utf32_iterator>>;
            using string_type = std::u32string;
        };

        template <>
        struct utf_conversion_traits<_utf32, _utf8>
        {
            using iterators = mp::list<mp::wrap<utf32_to_utf8_iterator>>;
            using string_type = std::string;
        };

        template <>
        struct utf_conversion_traits<_utf32, _utf16>
        {
            using iterators = mp::list<mp::wrap<utf32_to_utf16_iterator>>;
            using string_type = std::u16string;
        };


        template<typename iterator_type,
            typename base_iterator,
            typename iterator_value_type = typename std::iterator_traits< iterator_type >::value_type,
            typename base_iterator_value_type = typename std::iterator_traits< base_iterator>::value_type,
            typename iterator_category = typename std::iterator_traits< iterator_type >::iterator_category,
            typename std::enable_if<
            category_any_of<iterator_category, std::forward_iterator_tag, std::input_iterator_tag>::value &&
            (sizeof(iterator_value_type) > sizeof(base_iterator_value_type))
            , bool>::type = true>
            std::tuple<iterator_type, iterator_type> make_utf_iterators_for_iterable(base_iterator begin, base_iterator end)
        {
            return std::make_pair(iterator_type{ begin,end }, iterator_type{ end,end });
        }

        template<typename iterator_type,
            typename base_iterator,
            typename iterator_value_type = typename std::iterator_traits< iterator_type >::value_type,
            typename base_iterator_value_type = typename std::iterator_traits< base_iterator>::value_type,
            typename iterator_category = typename std::iterator_traits< iterator_type >::iterator_category,
            typename std::enable_if<
            category_any_of<iterator_category, std::bidirectional_iterator_tag, std::random_access_iterator_tag>::value &&
            (sizeof(iterator_value_type) > sizeof(base_iterator_value_type))
            , bool>::type = true>
            std::tuple<iterator_type, iterator_type> make_utf_iterators_for_iterable(base_iterator begin, base_iterator end)
        {
            return std::make_pair(iterator_type{ begin,begin,end }, iterator_type{ end,begin,end });
        }

        template<typename iterator_type,
            typename base_iterator,
            typename iterator_value_type = typename std::iterator_traits< iterator_type >::value_type,
            typename base_iterator_value_type = typename std::iterator_traits< base_iterator>::value_type,
            typename std::enable_if< (sizeof(iterator_value_type) < sizeof(base_iterator_value_type))
            , bool>::type = true>
            std::tuple<iterator_type, iterator_type> make_utf_iterators_for_iterable(base_iterator begin, base_iterator end)
        {
            return std::make_pair(iterator_type{ begin }, iterator_type{ end });
        }

        template<typename result_type,typename iterator_type,
            typename std::enable_if_t<has_push_back<result_type>::value,int> = 0>
        result_type apply_iterators(iterator_type begin, iterator_type end)
        {
            result_type result;
            std::copy(begin, end, std::back_inserter(result));
            return result;
        }

        template<typename result_type, typename iterator_type,
            typename std::enable_if_t<!has_push_back<result_type>::value && has_begin_end<result_type>::value, int> = 0>
            result_type apply_iterators(iterator_type iter, iterator_type end)
        {
            result_type result;
            
            auto riter = result.begin();
            auto rend = result.end();
            while (iter != end || riter != rend)
                *riter++ = *iter++;

            return result;
        }

        template<typename result_type, typename iterator_type,
            typename std::enable_if_t<std::is_constructible<result_type,typename iterator_type::value_type>::value, int> = 0>
            result_type apply_iterators(iterator_type iter, iterator_type end)
        {
            return (iter != end) ? result_type(*iter) : result_type{};
        }

        template <  typename result_type, template <typename...> class iterator_tmpl,typename base_iterator, typename... policy>
        result_type utf_convert_impl(base_iterator begin, base_iterator end, mp::list<mp::wrap<iterator_tmpl>>, mp::list< policy...>)
        {
            using iterator_type = iterator_tmpl<base_iterator, policy...>;
            iterator_type upos, uend;
            std::tie(upos, uend) = make_utf_iterators_for_iterable<iterator_type>(begin, end);
            return apply_iterators<result_type>(upos, uend);
        }

        template < typename result_type, template <typename...> class iterator_tmpl_first, template <typename...> class iterator_tmpl_second, typename base_iterator, typename... policy>
        result_type utf_convert_impl(base_iterator begin, base_iterator end, mp::list<mp::wrap<iterator_tmpl_first>,mp::wrap<iterator_tmpl_second>>  ,mp::list< policy...>)
        {
            using error_policy = select_argument_or_default< is_error_policy, void, policy...>;

            using iterator_type_first = iterator_tmpl_first<base_iterator, error_policy>;
            iterator_type_first uposf, uendf;
            std::tie(uposf, uendf) = make_utf_iterators_for_iterable<iterator_type_first>(begin, end);

            using iterator_type_second = iterator_tmpl_second<iterator_type_first, error_policy, policy...>;
            iterator_type_second uposs, uends;
            std::tie(uposs, uends) = make_utf_iterators_for_iterable<iterator_type_second>(uposf, uendf);

            return apply_iterators<result_type>(uposs, uends);
        }

        template <typename F, typename T, typename... args, typename iter_type>
        auto  utf_convert(iter_type begin, iter_type end)
        {
            using traits = utf_conversion_traits<F,T>;

            using result_type = select_argument_or_default<
                is_result_type,
                typename traits::string_type,
                args...>;

            return utf_convert_impl<result_type>(begin, end,
                typename traits::iterators(),
                mp::list<args...>());
        }

        template <typename F, typename T, typename... args, typename source_type,
            typename std::enable_if<is_char_type<source_type>::value, int>::type = 0>
            auto utf_convert(source_type source)
        {
            using traits = utf_conversion_traits<F,T>;
            using result_type = select_argument_or_default<
                is_result_type,
                typename traits::string_type::value_type,
                args...>;

            return utf_convert_impl<result_type>(&source, &source + 1,
                typename traits::iterators(),
                mp::list<args...>());
        }

        template <typename F, typename T, typename... args, typename source_type,
            typename std::enable_if<has_begin_end<source_type>::value, int>::type = 0>
            auto  utf_convert(const source_type& source)
        {
            return utf_convert<F, T, args...>(source.begin(), source.end());
        }

        template <typename F, typename T, typename... args, typename source_type,
            typename std::enable_if<is_char_type<source_type>::value, int>::type = 0>
            auto  utf_convert(const source_type* source)
        {
            return utf_convert<F, T, args...>(source, source + std::char_traits<source_type>::length(source));

        }
    }

    template <typename...Ts, typename iter_type>
    auto utf8_to_utf32(iter_type first, iter_type second)
    {
        return detail::utf_convert<detail::_utf8, detail::_utf32, Ts...>(
            first,
            second);
    }
    template <typename...Ts, typename source_type>
    auto utf8_to_utf32(source_type&& source)
    {
        return detail::utf_convert<detail::_utf8, detail::_utf32, Ts...>(
            std::forward<source_type>(source));
    }

    template <typename...Ts, typename iter_type>
    auto utf8_to_utf16(iter_type first, iter_type second)
    {
        return detail::utf_convert<detail::_utf8, detail::_utf16, Ts...>(
            first,
            second);
    }

    template <typename...Ts, typename source_type>
    auto utf8_to_utf16(source_type&& source)
    {
        return detail::utf_convert<detail::_utf8, detail::_utf16, Ts...>(
            std::forward<source_type>(source));
    }

    template <typename...Ts, typename iter_type>
    auto utf16_to_utf8(iter_type first, iter_type second)
    {
        return detail::utf_convert<detail::_utf16, detail::_utf8, Ts...>(
            first,
            second);
    }

    template <typename...Ts, typename source_type>
    auto utf16_to_utf8(source_type&& source)
    {
        return detail::utf_convert<detail::_utf16, detail::_utf8, Ts...>(
            std::forward<source_type>(source));
    }

    template <typename...Ts, typename iter_type>
    auto utf16_to_utf32(iter_type first, iter_type second)
    {
        return detail::utf_convert<detail::_utf16, detail::_utf32, Ts...>(
            first,
            second);
    }

    template <typename...Ts, typename source_type>
    auto utf16_to_utf32(source_type&& source)
    {
        return detail::utf_convert<detail::_utf16, detail::_utf32, Ts...>(
            std::forward<source_type>(source));
    }

    template <typename...Ts, typename iter_type>
    auto utf32_to_utf16(iter_type first, iter_type second)
    {
        return detail::utf_convert<detail::_utf32, detail::_utf16, Ts...>(
            first,
            second);
    }

    template <typename...Ts, typename source_type>
    auto utf32_to_utf16(source_type&& source)
    {
        return detail::utf_convert<detail::_utf32, detail::_utf16, Ts...>(
            std::forward<source_type>(source));
    }

    template <typename...Ts, typename iter_type>
    auto utf32_to_utf8(iter_type first, iter_type second)
    {
        return detail::utf_convert<detail::_utf32, detail::_utf8, Ts...>(
            first,
            second);
    }

    template <typename...Ts, typename source_type>
    auto utf32_to_utf8(source_type&& source)
    {
        return detail::utf_convert<detail::_utf32, detail::_utf8, Ts...>(
            std::forward<source_type>(source));
    }

    /*** Potential ambiguity with utf16 and utf32 if the utf16 string
     starts with two null characters it would be detected as utf32. */
    template <typename iterator_type>
    bom_type detect_bom(iterator_type pos, iterator_type end)
    {
        size_t byte_count = 0;
        uint8_t bytes[4]{};

        for (; byte_count < 4; ++pos) {
            if (pos == end) break;
            typename std::iterator_traits<iterator_type>::value_type value = *pos;
            std::memcpy(bytes + byte_count, &value, sizeof(value));
            byte_count += sizeof(value);
        }

        if (byte_count > 2 && !std::memcmp(bytes, utf8_bom, 3))
            return bom_type::utf8;

        if (byte_count > 3 && !std::memcmp(bytes, utf32_little_endian_bom, 4))
            return bom_type::utf32_little_endian;

        if (byte_count > 3 && !std::memcmp(bytes, utf32_big_endian_bom, 4))
            return bom_type::utf32_big_endian;

        if (byte_count > 1 && !std::memcmp(bytes, utf16_little_endian_bom, 2))
            return bom_type::utf16_little_endian;

        if (byte_count > 1 && !std::memcmp(bytes, utf16_big_endian_bom, 2))
            return bom_type::utf16_big_endian;

        return bom_type::none;
    }



}