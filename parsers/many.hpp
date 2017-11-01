#pragma once

#include <vector>
#include <functional>
#include <optional>
#include <sstream>

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
                                << " did not satisfy the condition. Expected at least"
                                << expected_at_least << ' '
                                << (expected_at_least == 1 ? "value" : "values");
                        break;
                    }

                    return { sstream.str(), inner_offset };
                }
            };

            template< typename P >
            struct Many
            {
                P parser;

                optional<unsigned int> _at_least;
                optional<unsigned int> _at_most;

                function<bool(typename P::Ok&)> _take_while;

                using Ok = vector<typename P::Ok>;
                using Err = ManyErr<typename P::Err>;

                Many(P parser)
                    : parser(move(parser))
                    , _at_least(nullopt)
                    , _at_most(nullopt)
                    , _take_while([](auto& x) { return true; }) {}

                Many<P>& at_least(unsigned int n)
                {
                    _at_least = n;
                    return *this;
                }

                Many<P>& at_most(unsigned int n)
                {
                    _at_most = n;
                    return *this;
                }

                Many<P>& take_while(function<bool(typename P::Ok&)> pred)
                {
                    _take_while = pred;
                    return *this;
                }

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    if (b >= e)
                    {
                        return EOI("Many position 1");
                    }

                    I iter = b;
                    unsigned int taken = 0;
                    vector<typename P::Ok> ret;

                    do
                    {
                        auto res = parser.parse(iter, e);

                        if (is_ok(res))
                        {
                            auto res_ok = unwrap_ok(move(res));

                            bool take_while_res = _take_while(res_ok.res);

                            if (take_while_res == false &&
                                _at_least.has_value() &&
                                taken < _at_least)
                            {
                                return err(ManyErr<typename P::Err>(
                                               nullopt,
                                               *_at_least,
                                               taken,
                                               ManyErrCause::ConditionFailed,
                                               distance(b, iter)),
                                           iter
                                    );
                            }

                            if (take_while_res == false)
                            {
                                return ok(move(ret), iter);
                            }

                            iter = res_ok.pos;

                            ret.emplace_back(move(res_ok.res));
                            taken++;
                        }
                        else if (is_err(res))
                        {
                            auto res_err = unwrap_err(move(res));

                            if (_at_least.has_value() && taken < *_at_least)
                            {
                                return err(ManyErr<typename P::Err>(
                                               move(res_err.err),
                                               *_at_least,
                                               taken,
                                               ManyErrCause::ParserFailed,
                                               distance(b, iter)),
                                           res_err.pos
                                    );
                            }

                            return ok(move(ret), iter);
                        }
                        else
                        {
                            if (_at_least.has_value() && taken < *_at_least)
                            {
                                auto res_eoi = unwrap_eoi(move(res));

                                res_eoi.trace.push_back("Many position "s + to_string(taken+1));

                                return res_eoi;
                            }

                            return ok(move(ret), iter);
                        }
                    } while (!(_at_most.has_value() && taken >= *_at_most));

                    return ok(move(ret), iter);
                }
            };
        }

        template< typename P >
        auto many(P parser)
        {
            return many_ns::Many<P>(move(parser));
        }
    }
}
