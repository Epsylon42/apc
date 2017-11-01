#pragma once

#include <string>
#include <sstream>

#include "misc.hpp"
#include "res.hpp"

namespace apc
{
    namespace parsers
    {
        namespace unit_ns
        {
            using namespace res;

            template< typename T >
            struct UnitErr
            {
                NilErr prev;

                T expected;
                T got;

                UnitErr(T expected, T got)
                    : prev()
                    , expected(expected)
                    , got(got) {}

                tuple<string, size_t> description()
                {
                    if constexpr (misc::is_printable_v<T>)
                                 {
                                     stringstream sstream;
                                     sstream << "Unit error because expected " << '"' << expected << '"'
                                             << " but got " << '"' << got << '"';

                                     return { sstream.str(), 0 };
                                 }
                    else
                    {
                        return { "Unit error", 0 };
                    }
                }
            };

            template< typename T >
            struct Unit
            {
                T unit;

                using Ok = T;
                using Err = UnitErr<T>;

                Unit(T unit) : unit(unit) {}

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    if (b >= e)
                    {
                        if constexpr (misc::is_printable_v<T>)
                                     {
                                         stringstream sstream;
                                         sstream << "Unit expecting "
                                                 << '"' << unit << '"';
                                         return EOI(sstream.str());
                                     }
                        else
                        {
                            return EOI("Unit");
                        }
                    }

                    if (*b == unit)
                    {
                        return ok(*b, next(b));
                    }

                    return err(UnitErr<T>(unit, *b), b);
                }
            };
        }

        template< typename T >
        auto unit(T unit)
        {
            return unit_ns::Unit<T>(move(unit));
        }

    }
}
