#pragma once

#include <tuple>

namespace apc::parsers
{
    namespace hide_ns
    {
        using namespace res;

        template< typename P >
        struct Hide
        {
            P parser;

            using Ok = NilOk;
            using Err = typename P::Err;

            Hide(P parser) : parser(parser) {}

            template< typename I >
            Result<Ok, Err, I> parse(I b, I e)
            {
                return parser.parse(b, e)
                    .fmap_ok([](auto& res_ok) -> Result<Ok, Err, I>
                    {
                        return ok(NilOk{}, move(res_ok.pos));
                    });
            }
        };
    }

    template< typename P >
    auto hide(P parser)
    {
        return hide_ns::Hide<P>(move(parser));
    }


}
