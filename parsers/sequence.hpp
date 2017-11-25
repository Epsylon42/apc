#pragma once

#include <sstream>
#include <tuple>
#include <type_traits>

#include <iostream>

namespace apc
{
    namespace parsers
    {
        namespace sequence_ns
        {
            using namespace res;
            using namespace misc;


            template< typename... Es >
            struct SequenceErr
            {
                variant<Es...> prev;

                size_t n;
                size_t inner_offset;

                SequenceErr(size_t inner_offset, variant<Es...> prev, size_t n)
                    : prev(move(prev))
                    , n(n)
                    , inner_offset(inner_offset) {}

                tuple<string, size_t> description()
                {
                    stringstream sstream;
                    sstream << "Sequence error because parser number " << n+1 << " failed";

                    return { sstream.str(), inner_offset } ;
                }
            };

            template< typename I, typename E, typename P, typename... Ps >
                auto sequence_impl(size_t n, I b, I e, P& head, Ps&... tail) -> Result<tuple<typename P::Ok, typename Ps::Ok...>, E, I>
            {
                using RetType = Result<tuple<typename P::Ok, typename Ps::Ok...>, E, I>;

                return head.parse(b, e)
                    .map_err([n](auto& head_err)
                    {
                        return err(
                            E(0, move(head_err.err), n),
                            head_err.pos
                        );
                    })
                    .fmap_ok([&](auto& head_ok) -> RetType
                    {
                        if constexpr (sizeof...(Ps) == 0)
                        {
                            return ok(make_tuple(move(head_ok.res)), head_ok.pos);
                        }
                        else
                        {
                            return sequence_impl<I, E>(n+1, head_ok.pos, e, tail...)
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
                    })
                    .visit_eoi([n](auto& head_eoi)
                    {
                        head_eoi.trace.push_back("Sequence position "s + std::to_string(n+1));
                    });
            }

            template< typename... Ps >
            struct Sequence
            {
                tuple<Ps...> parsers;

                using RetTypes = without_t_t<NilOk, typename Ps::Ok...>;

                using Ok = conditional_t< tuple_size_v<RetTypes> == 1,
                                          tuple_element_t<0, RetTypes>,
                                          RetTypes
                                          >;

                using Err = change_wrapper_t<SequenceErr, remove_duplicates_t<typename Ps::Err...>>;

                Sequence(Ps... parsers) : parsers(move(parsers)...) {}

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    auto res = apply([&b, &e](auto&... parsers)
                                     {
                                         return sequence_impl<I, Err>(0, b, e, parsers...);
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
            return sequence_ns::Sequence<Ps...>(move(parsers)...);
        }
    }
}
