#pragma once

namespace apc::parsers
{
    namespace nop_ns
    {
        using namespace res;

        struct Nop
        {
            using Ok = NilOk;
            using Err = NilErr;

            Nop() {}

            template< typename I >
            Result<Ok, Err, I> parse(I b, I e)
            {
                return ok(NilOk{}, b);
            }
        };
    }

    nop_ns::Nop nop()
    {
        return nop_ns::Nop{};
    }
}
