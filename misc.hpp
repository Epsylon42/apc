#pragma once

#include <sstream>
#include <type_traits>
#include <tuple>
#include <utility>
#include <experimental/type_traits>

namespace apc
{
    namespace misc
    {
        using namespace std;
        using experimental::fundamentals_v2::is_detected_v;

        template< typename T >
        using print_t = decltype(declval<ostream&>() << declval<T&>());

        template< typename T >
        constexpr bool is_printable_v = is_detected_v<print_t, T>;

        template< typename T, typename... Ts >
        constexpr bool are_printable_v = (!is_printable_v<T> ? false : are_printable_v<Ts...>);

        template< typename T >
        constexpr bool are_printable_v<T> = is_printable_v<T>;

        template< typename... Ts >
        tuple<Ts&...> ref_tuple(tuple<Ts...>& t)
        {
            return apply([](auto&... elems)
                         {
                             return tie(elems...);
                         }, t);
        }

        template< typename... Ts >
        tuple<Ts...> move_ref_tuple(tuple<Ts&...> t)
        {
            return apply([](auto&... elems)
                         {
                             return make_tuple(move(elems)...);
                         }, t);
        }


        template< typename T, typename Tuple >
        struct AppendTuple;

        template< typename T, typename... Ts >
        struct AppendTuple<T, tuple<Ts...>>
        {
            using type = tuple<T, Ts...>;
        };


        template< typename... Ts >
        struct WithoutNils;

        template< typename T, typename... Ts >
        struct WithoutNils<T, Ts...>
        {
            using type = typename AppendTuple<T, typename WithoutNils<Ts...>::type>::type;
        };

        template< typename... Ts >
        struct WithoutNils<res::NilOk, Ts...>
        {
            using type = typename WithoutNils<Ts...>::type;
        };

        template< typename... Ts >
        struct WithoutNils<res::NilOk&, Ts...>
        {
            using type = typename WithoutNils<Ts...>::type;
        };

        template<>
        struct WithoutNils<>
        {
            using type = tuple<>;
        };

        template< typename... Ts >
        using without_nils_t = typename WithoutNils<Ts...>::type;


        template< typename T, typename... Ts >
        constexpr without_nils_t<T&, Ts&...> without_nils(tuple<T&, Ts&...> t)
        {
            if constexpr (is_same_v<T, res::NilOk>)
            {
                if constexpr (sizeof...(Ts) != 0)
                {
                    return apply([](auto& head, auto&... tail)
                                 {
                                     return without_nils(tie(tail...));
                                 }, t);
                }
                else
                {
                    return tie();
                }
            }
            else
            {
                if constexpr (sizeof...(Ts) != 0)
                {
                    return apply([](auto& head, auto&... tail)
                                 {
                                     return tuple_cat
                                         (
                                             tie(head),
                                             without_nils(tie(tail...))
                                             );
                                 }, t);
                }
                else
                {
                    return tie(get<0>(t));
                }
            }
        }
    }
}
