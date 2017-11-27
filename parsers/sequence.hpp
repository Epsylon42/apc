#pragma once

#include <sstream>
#include <tuple>
#include <type_traits>

#include <iostream>

#include "nop.hpp"

namespace apc::parsers
{
    namespace sequence_ns
    {
        using namespace res;
        using namespace misc;

        enum class SequenceErrCause
        {
            Parser,
            Delimiter,
        };

        template< typename... Es >
        struct SequenceErr
        {
            variant<Es...> prev;

            SequenceErrCause cause;

            size_t n;
            size_t inner_offset;

            SequenceErr(size_t inner_offset, SequenceErrCause cause, variant<Es...> prev, size_t n)
                : prev(move(prev))
                , cause(cause)
                , n(n)
                , inner_offset(inner_offset) {}

            tuple<string, size_t> description()
            {
                stringstream sstream;
                switch (cause)
                {
                    case SequenceErrCause::Parser:
                        sstream << "Sequence error because parser number " << n << " failed";
                        break;

                    case SequenceErrCause::Delimiter:
                        sstream << "Sequence error because delimited before parser " << n << " failed";
                        break;
                }

                return { sstream.str(), inner_offset } ;
            }
        };

        template< typename I, typename E, bool has_delim, typename D, typename P, typename... Ps >
        auto sequence_impl(size_t n, I b, I e, D& delim, P& head, Ps&... tail) -> Result<tuple<typename P::Ok, typename Ps::Ok...>, E, I>
        {
            using RetType = Result<tuple<typename P::Ok, typename Ps::Ok...>, E, I>;

            I real_b = b;

            if constexpr (has_delim)
            {
                if (n != 0)
                {
                    auto delim_res = delim.parse(b, e)
                        .map_err([n](auto& delim_err)
                        {
                            return err(
                                E(0, SequenceErrCause::Delimiter, move(delim_err.err), n+1),
                                delim_err.pos
                            );
                        })
                        .visit_eoi([n](auto& delim_eoi)
                        {
                            delim_eoi.trace.push_back("Sequence delimiter before position "s + std::to_string(n+1));
                        });

                    if (delim_res.is_ok())
                    {
                        real_b = delim_res.unwrap_ok().pos;
                    }
                    else
                    {
                        if (delim_res.is_err())
                        {
                            return delim_res.unwrap_err();
                        }
                        else
                        {
                            return delim_res.unwrap_eoi();
                        }
                    }
                }
            }

            return head.parse(real_b, e)
                .map_err([n](auto& head_err)
                {
                    return err(
                        E(0, SequenceErrCause::Parser, move(head_err.err), n+1),
                        head_err.pos
                    );
                })
                .visit_eoi([n](auto& head_eoi)
                {
                    head_eoi.trace.push_back("Sequence position "s + std::to_string(n+1));
                })
                .fmap_ok([&](auto& head_ok) -> RetType
                {
                    if constexpr (sizeof...(Ps) == 0)
                    {
                        return ok(make_tuple(move(head_ok.res)), head_ok.pos);
                    }
                    else
                    {
                        return sequence_impl<I, E, has_delim>(n+1, head_ok.pos, e, delim, tail...)
                            .map_ok([&head_ok](auto& tail_ok)
                            {
                                return ok(
                                    tuple_cat(
                                        make_tuple(move(head_ok.res)),
                                        move(tail_ok.res)
                                    ),
                                    tail_ok.pos
                                );
                            })
                            .visit_err([&b](auto& tail_err)
                            {
                                tail_err.err.inner_offset += distance(b, tail_err.pos);
                            });
                    }
                });
        }


        template< bool has_delim, typename D, typename... Ps >
        struct Sequence
        {
            tuple<Ps...> parsers;

            using DelimType = conditional_t< has_delim,
                                             D,
                                             unsigned char
                                             >;

            DelimType delim;

            using RetTypes = without_t_t<NilOk, typename Ps::Ok...>;

            using Ok = conditional_t< tuple_size_v<RetTypes> == 1,
                                      tuple_element_t<0, RetTypes>,
                                      RetTypes
                                      >;

            using Err = change_wrapper_t<SequenceErr,
                                         conditional_t< has_delim,
                                                        remove_duplicates_t<typename D::Err, typename Ps::Err...>,
                                                        remove_duplicates_t<typename Ps::Err...>
                                                        >
                                         >;

            Sequence(DelimType delim, tuple<Ps...> parsers)
                : parsers(move(parsers))
                , delim(move(delim)) {}

            template< typename NewD >
            auto with_delim(NewD new_delim_parser)&&
            {
                return Sequence<true, NewD, Ps...>(
                    move(new_delim_parser),
                    move(parsers)
                );
            }

            template< typename I >
            Result<Ok, Err, I> parse(I b, I e)
            {
                auto res = apply([&b, &e, this](auto&... parsers)
                {
                    return sequence_impl<I, Err, has_delim>(0, b, e, delim, parsers...);
                }, parsers);

                return res
                    .map_ok([](auto& res_ok)
                    {
                        if constexpr (is_same_v<Ok, RetTypes>)
                        {
                            return ok(
                                move_ref_tuple(
                                    without_t<NilOk>(
                                        ref_tuple(res_ok.res)
                                    )
                                ),
                                res_ok.pos
                            );
                        }
                        else
                        {
                            return ok(
                                move(get<0>(without_t<NilOk>(ref_tuple(res_ok.res)))),
                                res_ok.pos
                            );
                        }
                    });
            }
        };
    }

    template< typename... Ps >
    auto sequence(Ps... parsers)
    {
        static_assert(sizeof...(Ps) > 0, "sequence parser requires at least one argument");
        return sequence_ns::Sequence<false, nop_ns::Nop, Ps...>(0, make_tuple(move(parsers)...));
    }

}
