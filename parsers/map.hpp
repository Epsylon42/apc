#pragma once

namespace apc::parsers
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
                return parser.parse(b, e)
                    .map_ok([this](auto& res_ok)
                    {
                        return ok(func(move(res_ok.res)), res_ok.pos);
                    });
            }
        };
    }

    template< typename P, typename F >
    auto map(P parser, F func)
    {
        return map_ns::Map<P, F>(move(parser), move(func));
    }

}
