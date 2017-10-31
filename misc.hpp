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
        constexpr bool is_printable(T& t)
        {
            return is_detected_v<print_t, T>;
        }

        template< typename T >
        constexpr bool is_printable(const T& t)
        {
            return is_detected_v<print_t, T>;
        }

        template< typename T >
        constexpr bool is_printable()
        {
            return is_detected_v<print_t, T>;
        }


        template< typename T, typename Tuple >
        struct AppendTuple;

        template< typename T, typename... Ts >
        struct AppendTuple<T, tuple<Ts...>>
        {
            using type = tuple<T, Ts...>;
        };


        template< typename... Ts >
        struct NoNils;

        template< typename T, typename... Ts >
        struct NoNils<T, Ts...>
        {
            using type = typename AppendTuple<T, typename NoNils<Ts...>::type>::type;
        };

        template< typename... Ts >
        struct NoNils<res::NilOk, Ts...>
        {
            using type = typename NoNils<Ts...>::type;
        };

        template<>
        struct NoNils<>
        {
            using type = tuple<>;
        };

        template< typename... Ts >
        using no_nils_t = typename NoNils<Ts...>::type;


        template< typename T, typename... Ts >
        constexpr no_nils_t<T, Ts...> no_nils(tuple<T, Ts...> t)
        {
            if constexpr (is_same_v<T, res::NilOk>)
            {
                if constexpr (sizeof...(Ts) != 0)
                {
                    return apply([](auto head, auto... tail)
                                 {
                                     return no_nils(make_tuple(tail...));
                                 }, t);
                }
                else
                {
                    return make_tuple();
                }
            }
            else
            {
                if constexpr (sizeof...(Ts) != 0)
                {
                    return apply([](auto head, auto... tail)
                                 {
                                     return tuple_cat
                                         (
                                             make_tuple(head),
                                             no_nils(make_tuple(tail...))
                                             );
                                 }, t);
                }
                else
                {
                    return make_tuple(get<0>(t));
                }
            }
        }
    }
}
