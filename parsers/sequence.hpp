#pragma once

#include <memory>
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

            struct SequenceErr : Error
            {
                std::unique_ptr<Error> prev;

                size_t n;

                size_t inner_offset;

                SequenceErr(size_t self_offset, std::unique_ptr<Error> prev, size_t n)
                    : prev(std::move(prev))
                    , n(n)
                    , inner_offset(self_offset) {}

                virtual string description(size_t offset = 0) override
                {
                    std::stringstream sstream;
                    sstream << "Sequence error at " << offset
                            << " because parser number " << n << " failed:" << endl
                            << prev->description(offset + inner_offset);


                    return sstream.str() ;
                }

                virtual Error& previous() override
                {
                    return *prev;
                }
            };

            template< typename I, typename T, typename... Ts >
                auto sequence_impl(size_t n, I b, I e, T& head, Ts&... tail) -> Result<tuple<typename T::Ok, typename Ts::Ok...>, SequenceErr, I>
            {
                auto head_res = head.parse(b, e);
                if (is_ok(head_res))
                {
                    auto head_ok = unwrap_ok(move(head_res));

                    if constexpr (sizeof...(Ts) == 0)
                                 {
                                     return ok(make_tuple(move(head_ok.res)), head_ok.pos);
                                 }
                    else
                    {
                        auto tail_res = sequence_impl(n+1, head_ok.pos, e, tail...);
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

                    return err(SequenceErr(
                                   distance(b, err_pos),
                                   unique_ptr<Error>(new remove_reference_t<decltype(head_err.err)> (move(head_err.err))),
                                   n
                                   ),
                               err_pos
                        );
                }
                else // eoi
                {
                    auto head_eoi = unwrap_eoi(move(head_res));

                    head_eoi.trace.push_back("Sequence position "s + std::to_string(n));

                    return move(head_eoi);
                }
            }

            template< typename... Ts >
                struct Sequence
            {
                tuple<Ts...> parsers;

                using Ok = typename misc::NoNils<typename Ts::Ok...>::type;
                using Err = SequenceErr;

                Sequence(Ts... parsers) : parsers(move(parsers)...) {}

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    auto res = apply([&b, &e](auto head, auto... tail)
                                     {
                                         return sequence_impl(0, b, e, head, tail...);
                                     }, parsers);

                    if (is_ok(res))
                    {
                        auto res_ok = unwrap_ok(move(res));

                        I res_end = res_ok.pos;

                        return ok(move(misc::no_nils(move(res_ok.res))), res_end);
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

        template< typename... Ts >
        auto sequence(Ts... parsers)
        {
            return sequence_ns::Sequence<Ts...>(move(parsers)...);
        }
    }
}
