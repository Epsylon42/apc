#pragma once

namespace apc
{
    namespace parsers
    {
        namespace any_ns
        {
            using namespace res;

            template< typename T >
            struct Any
            {
                using Ok = T;
                using Err = NilErr;

                Any() {}

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    if (b >= e)
                    {
                        return EOI("Any");
                    }

                    return ok(*b, next(b));
                }
            };
        }

        template< typename T >
        any_ns::Any<T> any()
        {
            return any_ns::Any<T>();
        }
    }
}
