#pragma once

namespace apc
{
    namespace parsers
    {
        namespace map_ns
        {
            using namespace res;

            template< typename P, typename F >
            struct Map
            {
                P parser;
                F func;

                using Ok = invoke_result_t<F, typename P::Ok&&>;
                using Err = typename P::Err;

                Map(P parser, F func)
                    : parser(move(parser))
                    , func(move(func)) {}

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    auto parser_res = parser.parse(b, e);
                    if (is_ok(parser_res))
                    {
                        auto res_ok = unwrap_ok(move(parser_res));

                        return ok(func(move(res_ok.res)), res_ok.pos);
                    }
                    else if (is_err(parser_res))
                    {
                        auto res_err = unwrap_err(move(parser_res));

                        return err(move(res_err.err), res_err.pos);
                    }
                    else
                    {
                        return unwrap_eoi(move(parser_res));
                    }
                }
            };
        }

        template< typename P, typename F >
        auto map(P parser, F func)
        {
            return map_ns::Map<P, F>(move(parser), move(func));
        }
    }
}
