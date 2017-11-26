#pragma once

#include <vector>
#include <functional>
#include <optional>
#include <sstream>
#include <string>

#include "any.hpp"
#include "nop.hpp"

namespace apc
{
    namespace parsers
    {
        namespace many_ns
        {
            using namespace res;
            using namespace std;

            enum class ManyErrCause
            {
                ParserFailed,
                ConditionFailed,
                DelimiterFailed,
            };

            template< typename E >
            struct ManyErr
            {
                optional<E> prev;

                unsigned int expected_at_least;
                unsigned int step;

                ManyErrCause cause;

                size_t inner_offset;

                ManyErr(optional<E> prev, unsigned int al, unsigned int step, ManyErrCause cause, size_t offset)
                    : prev(move(prev))
                    , expected_at_least(al)
                    , step(step)
                    , cause(cause)
                    , inner_offset(offset) {}

                tuple<string, size_t> description()
                {
                    stringstream sstream;
                    sstream << "Many error because ";

                    switch (cause)
                    {
                    case ManyErrCause::ParserFailed:
                        sstream << "parser number " << step+1
                                << " failed. Expected at least " << expected_at_least
                                << ' ' << (expected_at_least == 1 ? "value" : "values");
                        break;

                    case ManyErrCause::ConditionFailed:
                        sstream << "return value of parser number " << step+1
                                << " did not satisfy the condition. Expected at least "
                                << expected_at_least << ' '
                                << (expected_at_least == 1 ? "value" : "values");
                        break;

                    case ManyErrCause::DelimiterFailed:
                        sstream << "delimiter after parser number" << step+1
                                << " failed. Expected at least " << expected_at_least
                                << ' ' << (expected_at_least == 1 ? "value" : "values");
                        break;
                    }

                    return { sstream.str(), inner_offset };
                }
            };

            template< typename P, template< typename > class C,
                      bool hlb = false, // has lower bound
                      bool hub = false, // has upper bound
                      bool hc = false, // has condition
                      bool hd = false, // has delimiter
                      typename D = nop_ns::Nop>
            struct Many
            {
                P parser;

            private:

                // C++, do you even zero-sized types?
                using AtLeast = conditional_t< hlb, size_t, unsigned char >;
                using AtMost = conditional_t< hub, size_t, unsigned char >;

                using TakeWhile = conditional_t< hc, function<bool(typename P::Ok&)>, unsigned char >;
                using DelimParser = conditional_t< hd, D, unsigned char >;

                 AtLeast _at_least;
                 AtMost _at_most;

                 TakeWhile _take_while;

                 DelimParser delim_parser;

            public:

                Many(P parser, AtLeast _at_least, AtMost _at_most, TakeWhile _take_while, DelimParser delim_parser)
                    : parser(move(parser))
                    , _at_least(move(_at_least))
                    , _at_most(move(_at_most))
                    , _take_while(move(_take_while))
                    , delim_parser(move(delim_parser)) {}

                using Ok = C<typename P::Ok>;
                using Err = conditional_t< hd,
                                           ManyErr<variant< typename P::Err, typename D::Err >>,
                                           ManyErr<typename P::Err>
                                           >;

                //TODO: check if moving stuff is necessary
                auto at_least(size_t n)&&
                {
                    return Many<P, C, true, hub, hc, hd, D>(
                        move(parser),
                        n, _at_most,
                        move(_take_while), move(delim_parser)
                    );
                }

                auto at_most(size_t n)&&
                {
                    return Many<P, C, hlb, true, hc, hd, D>(
                        move(parser),
                        _at_least, n,
                        move(_take_while), move(delim_parser)
                    );
                }

                auto take_while(function<bool(typename P::Ok&)> pred)&&
                {
                    return Many<P, C, hlb, hub, true, hd, D>(
                        move(parser),
                        _at_least, _at_most,
                        move(pred), move(delim_parser)
                    );
                }

                auto take_until(function<bool(typename P::Ok&)> pred)&&
                {
                    return Many<P, C, hlb, hub, true, hd, D>(
                        move(parser),
                        _at_least, _at_most,
                        not_fn(move(pred)), move(delim_parser)
                    );
                }

                template< typename NewD >
                auto with_delim(NewD new_delim_parser)&&
                {
                    return Many<P, C, hlb, hub, hc, true, NewD>(
                        move(parser),
                        _at_least, _at_most,
                        move(_take_while), move(new_delim_parser)
                    );
                }

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    if (b >= e)
                    {
                        return EOI("Many position 1");
                    }

                    I iter = b;
                    size_t taken = 0;
                    C<typename P::Ok> ret;

                    do
                    {
                        if constexpr (hd)
                        {
                            if (taken > 0)
                            {
                                auto delim_res = delim_parser.parse(iter, e);
                                if (delim_res.is_eoi())
                                {
                                    if (taken < _at_least)
                                    {
                                        delim_res.unwrap_eoi().trace.push_back("Many position "s + to_string(taken+1) + " delimiter");
                                        return move(delim_res.unwrap_eoi());
                                    }

                                    return ok(move(ret), iter);
                                }
                                else if (delim_res.is_err())
                                {
                                    if constexpr (hlb)
                                    {
                                        if (taken < _at_least)
                                        {
                                            auto delim_err = move(delim_res.unwrap_err());

                                            return err(Err(
                                                            delim_err.err,
                                                            _at_least,
                                                            taken,
                                                            ManyErrCause::DelimiterFailed,
                                                            distance(b, iter)),
                                                       iter
                                                );
                                            }
                                    }

                                    return ok(move(ret), iter);
                                }
                                else // is ok
                                {
                                    iter = delim_res.unwrap_ok().pos;
                                }
                            }
                        }

                        auto res = parser.parse(iter, e);

                        if (res.is_ok())
                        {
                            auto res_ok = move(res.unwrap_ok());

                            bool take_while_res = true;
                            if constexpr (hc)
                            {
                                take_while_res = _take_while(res_ok.res);
                            }

                            if constexpr (hlb)
                            {
                                if (take_while_res == false &&
                                    taken < _at_least)
                                {
                                    return err(Err(
                                                   nullopt,
                                                   _at_least,
                                                   taken,
                                                   ManyErrCause::ConditionFailed,
                                                   distance(b, iter)),
                                               iter
                                        );
                                }
                            }

                            if (take_while_res == false)
                            {
                                return ok(move(ret), iter);
                            }

                            iter = res_ok.pos;

                            ret.push_back(move(res_ok.res));
                            taken++;
                        }
                        else if (res.is_err())
                        {
                            if constexpr (hlb)
                            {
                                auto res_err = move(res.unwrap_err());

                                if (taken < _at_least)
                                {
                                    return err(Err(
                                                    move(res_err.err),
                                                    _at_least,
                                                    taken,
                                                    ManyErrCause::ParserFailed,
                                                    distance(b, iter)),
                                                res_err.pos
                                        );
                                }
                            }

                            return ok(move(ret), iter);
                        }
                        else
                        {
                            if constexpr (hlb)
                            {
                                if (taken < _at_least)
                                {
                                    auto res_eoi = move(res.unwrap_eoi());

                                    res_eoi.trace.push_back("Many position "s + to_string(taken+1));

                                    return res_eoi;
                                }
                            }

                            return ok(move(ret), iter);
                        }
                    } while (!hub || taken < _at_most);

                    return ok(move(ret), iter);
                }
            };
        }

        template< typename P, template< typename > class C = vector >
        auto many(P parser)
        {
            return many_ns::Many<P, C>(move(parser), 0, 0, 0, 0);
        }

        template< typename T, template< typename > class C = vector >
        auto many()
        {
            return many<any_ns::Any<T>, C>(any<T>());
        }

        template< typename P >
        auto many_str(P parser)
        {
            return many_ns::Many<P, basic_string>(move(parser));
        }

        template< typename T >
        auto many_str()
        {
            return many<any_ns::Any<T>, basic_string>(any<T>());
        }

        auto many_space() {
            return many_str<char>()
                .take_while([](char& c) { return isspace(c); });
        }
    }
}
