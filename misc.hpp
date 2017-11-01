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

        template< typename T, typename Tuple >
        using append_tuple_t = typename AppendTuple<T, Tuple>::type;


        template< typename R, typename... Ts >
        struct WithoutT;

        template< typename R, typename T, typename... Ts >
        struct WithoutT<R, T, Ts...>
        {
            using type = conditional_t< is_same_v<remove_reference_t<T>, remove_reference_t<R>>,
                                        typename WithoutT<R, Ts...>::type,
                                        append_tuple_t<T, typename WithoutT<R, Ts...>::type>
                                        >;
        };

        template< typename R >
        struct WithoutT<R>
        {
            using type = tuple<>;
        };

        template< typename R, typename... Ts >
        using without_t_t = typename WithoutT<R, Ts...>::type;


        template< typename R, typename T, typename... Ts >
        constexpr without_t_t<R, T&, Ts&...> without_t(tuple<T&, Ts&...> t)
        {
            if constexpr (is_same_v<remove_reference_t<T>, remove_reference_t<R>>)
            {
                if constexpr (sizeof...(Ts) != 0)
                {
                    return apply([](auto& head, auto&... tail)
                                 {
                                     return without_t<R>(tie(tail...));
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
                                             without_t<R>(tie(tail...))
                                             );
                                 }, t);
                }
                else
                {
                    return tie(get<0>(t));
                }
            }
        }


        template< typename T, typename U, typename... Us >
        constexpr bool is_in_list_v = (is_same_v<T, U> ? true : is_in_list_v<T, Us...>);

        template< typename T, typename U >
        constexpr bool is_in_list_v<T, U> = is_same_v<T, U>;


        template< typename T, typename... Ts >
        struct RemoveDuplicates
        {
            using type = conditional_t< is_in_list_v<T, Ts...>,
                                        typename RemoveDuplicates<Ts...>::type,
                                        append_tuple_t<T, typename RemoveDuplicates<Ts...>::type>
                                        >;
        };

        template< typename T >
        struct RemoveDuplicates<T>
        {
            using type = tuple<T>;
        };

        template< typename... Ts >
        using remove_duplicates_t = typename RemoveDuplicates<Ts...>::type;


        template< template< typename... > class T, typename U >
        struct ChangeWrapper;

        template< template< typename... > class T, template< typename... > class U, typename... Ts >
        struct ChangeWrapper<T, U<Ts...>>
        {
            using type = T<Ts...>;
        };

        template< template< typename... > class T, typename U >
        using change_wrapper_t = typename ChangeWrapper<T, U>::type;


        template< typename I >
        size_t calc_inner_offset(I b, I err_pos)
        {
            return distance(b, err_pos);
        }


        template< typename T >
        constexpr bool is_optional(const T&)
        {
            return false;
        }

        template< typename T >
        constexpr bool is_optional(const optional<T>&)
        {
            return true;
        }

        template< typename T >
        constexpr bool is_variant(const T&)
        {
            return false;
        }

        template< typename... Ts >
        constexpr bool is_variant(const variant<Ts...>&)
        {
            return true;
        }
    }
}
