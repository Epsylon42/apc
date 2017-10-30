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
                struct UnitErr : Error
            {
                NilErr prev;

                T expected;
                T got;

                UnitErr(T expected, T got)
                    : prev()
                    , expected(expected)
                    , got(got) {}

                virtual string description(size_t offset = 0) override
                {
                    if constexpr (misc::is_printable<T>())
                                 {
                                     stringstream sstream;
                                     sstream << "Unit error at " << offset
                                             << " because expected " << '"' << expected << '"'
                                             << " but got " << '"' << got << '"';

                                     return sstream.str();
                                 }
                    else
                    {
                        return "Unit error at " + std::to_string(offset);
                    }
                }

                virtual Error& previous() override
                {
                    return prev;
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
                        if constexpr (misc::is_printable<T>())
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
