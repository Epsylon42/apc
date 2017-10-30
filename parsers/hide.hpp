#pragma once

#include <tuple>

namespace apc
{
    namespace parsers
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
                    auto res = parser.parse(b, e);

                    if (is_ok(res))
                    {
                        auto res_ok = unwrap_ok(move(res));

                        return ok(NilOk{}, move(res_ok.pos));
                    }
                    else if (is_err(res))
                    {
                        auto res_err = unwrap_err(move(res));

                        return err(res_err.err, res_err.pos);
                    }
                    else
                    {
                        return unwrap_eoi(move(res));
                    }
                }
            };
        }

        template< typename P >
        auto hide(P parser)
        {
            return hide_ns::Hide<P>(move(parser));
        }

    }
}
