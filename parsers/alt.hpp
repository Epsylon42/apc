#pragma once

#include <sstream>
#include <optional>
#include <variant>
#include <tuple>

namespace apc
{
    namespace alt_ns
    {
        using namespace res;
        using namespace misc;

        struct AltErr
        {
            // possibly replace with error tuple
            NilErr prev;

            size_t n_err;
            size_t n_eoi;

            AltErr(size_t n_err, size_t n_eoi)
                : prev()
                , n_err(n_err)
                , n_eoi(n_eoi) {}

            tuple<string, size_t> description()
            {
                // n_err can't be equal to zero

                if (n_eoi == 0)
                {
                    return { "Alt error because all alternatives failed", 0 };
                }
                else
                {
                    stringstream sstream;
                    sstream << "Alt error because "
                            << n_err << (n_err == 1 ? "alternative" : "alternatives") << "failed and "
                            << n_eoi << (n_eoi == 1 ? "alternative" : "alternatives") << "met end of input";

                    return { sstream.str(), 0 };
                }
            }
        };

        template< typename I, typename Ok_T, typename P, typename... Ps >
        auto alt_impl(size_t n_err, size_t n_eoi, I b, I e, P& head, Ps&... tail) -> variant<res::Ok<Ok_T, I>, tuple<size_t, size_t>>
        {
            auto head_res = head.parse(b, e);
            if (head_res.is_ok())
            {
                auto res_ok = move(head_res.unwrap_ok());

                return ok(Ok_T(res_ok.res), res_ok.pos);
            }
            else if (head_res.is_err())
            {
                if constexpr (sizeof...(Ps) == 0)
                {
                    return make_tuple(n_err, n_eoi);
                }
                else
                {
                    return alt_impl<I, Ok_T>(n_err+1, n_eoi, b, e, tail...);
                }
            }
            else
            {
                if constexpr (sizeof...(Ps) == 0)
                {
                    return make_tuple(n_err, n_eoi);
                }
                else
                {
                    return alt_impl<I, Ok_T>(n_err, n_eoi+1, b, e, tail...);
                }
            }
        }

        template< typename... Ps >
        struct Alt
        {
            tuple<Ps...> parsers;

            using RetTypes = remove_duplicates_t<typename Ps::Ok...>;

            using Ok = conditional_t<
                tuple_size_v<RetTypes> == 1,
                tuple_element_t<0, RetTypes>,
                change_wrapper_t<variant, RetTypes>
                >;

            using Err = AltErr;

            Alt(Ps... parsers) : parsers(make_tuple(move(parsers)...)) {}

            template< typename I >
            Result<Ok, Err, I> parse(I b, I e)
            {
                auto res = apply([&b, &e](auto&... parsers)
                {
                    return alt_impl<I, Ok>(0, 0, b, e, parsers...);
                }, parsers);

                if (holds_alternative<res::Ok<Ok, I>>(res))
                {
                    return get<res::Ok<Ok, I>>(move(res));
                }
                else
                {
                    auto [ n_err, n_eoi ] = get<tuple<size_t, size_t>>(move(res));

                    if (n_err != 0)
                    {
                        return err(AltErr(n_err, n_eoi), b);
                    }
                    else
                    {
                        return EOI("Alt");
                    }
                }
            }
        };
    }

    template< typename... Ps >
    auto alt(Ps... parsers)
    {
        static_assert(sizeof...(Ps) > 0, "alt parser requires at least one argument");
        return alt_ns::Alt<Ps...>(move(parsers)...);
    }

}
