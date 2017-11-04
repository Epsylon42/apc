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
                auto head_res = head.parse(b, e);
                if (is_ok(head_res))
                {
                    auto head_ok = unwrap_ok(move(head_res));

                    if constexpr (sizeof...(Ps) == 0)
                    {
                        return ok(make_tuple(move(head_ok.res)), head_ok.pos);
                    }
                    else
                    {
                        auto tail_res = sequence_impl<I, E>(n+1, head_ok.pos, e, tail...);
                        if (is_ok(tail_res))
                        {
                            auto tail_ok = unwrap_ok(move(tail_res));

                            I tail_end = tail_ok.pos;

                            auto ret = tuple_cat(
                                make_tuple(move(head_ok.res)),
                                move(tail_ok.res)
                                );

                            return ok(move(ret), tail_end);
                        }
                        else if (is_err(tail_res))
                        {
                            auto tail_err = unwrap_err(move(tail_res));

                            I err_pos = tail_err.pos;

                            tail_err.err.inner_offset += distance(b, err_pos);

                            return err(move(tail_err.err), err_pos);
                        }
                        else // eoi
                        {
                            return unwrap_eoi(move(tail_res));
                        }
                    }
                }
                else if (is_err(head_res))
                {
                    auto head_err = unwrap_err(move(head_res));

                    I err_pos = head_err.pos;

                    return err(E(
                                   0,
                                   move(head_err.err),
                                   n
                                   ),
                               err_pos
                        );
                }
                else // eoi
                {
                    auto head_eoi = unwrap_eoi(move(head_res));

                    head_eoi.trace.push_back("Sequence position "s + std::to_string(n+1));

                    return head_eoi;
                }
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

                    if (is_ok(res))
                    {
                        auto res_ok = unwrap_ok(move(res));

                        I res_end = res_ok.pos;

                        if constexpr (is_same_v<Ok, RetTypes>)
                        {
                            return ok(move_ref_tuple(without_t<NilOk>(ref_tuple(res_ok.res))), res_end);
                        }
                        else
                        {
                            return ok(get<0>(without_t<NilOk>(ref_tuple(res_ok.res))), res_end);
                        }
                    }
                    else if (is_err(res))
                    {
                        auto res_err = unwrap_err(move(res));

                        return err(move(res_err.err), move(res_err.pos));
                    }
                    else
                    {
                        return unwrap_eoi(move(res));
                    }
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
