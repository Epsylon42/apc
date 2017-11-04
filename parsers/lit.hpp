#pragma once

#include <string>
#include <sstream>

namespace apc
{
    namespace parsers
    {
        namespace lit_ns
        {
            using namespace res;

            template< typename T >
            struct LitErr
            {
                NilErr prev;

                T expected;

                size_t inner_offset;

                LitErr(T expected, size_t inner_offset)
                    : prev()
                    , expected(move(expected))
                    , inner_offset(inner_offset) {}

                tuple<string, size_t> description()
                {
                    if constexpr (misc::is_printable_v<T>)
                    {
                        stringstream sstream;
                        sstream << "Lit error: expected " << '"' << expected << '"';

                        return { sstream.str(), 0 };
                    }
                }
            };

            template< typename T >
            struct Lit
            {
                T lit;

                using Ok = T;
                using Err = LitErr<T>;

                Lit(T lit) : lit(move(lit)) {}

                template< typename I >
                Result<Ok, Err, I> parse(I b, I e)
                {
                    I iter = b;
                    auto lit_iter = begin(lit);
                    for (; lit_iter != end(lit) && iter != e; advance(iter, 1), advance(lit_iter, 1))
                    {
                        if (*iter != *lit_iter)
                        {
                            return err(LitErr<T>(lit, distance(b, iter)), iter);
                        }
                    }

                    auto lit_len = distance(begin(lit), end(lit));

                    if (distance(b, iter) < lit_len)
                    {
                        return EOI("Lit");
                    }

                    return ok(lit, next(b, lit_len));
                }
            };
        }

        template< typename T >
        enable_if_t<misc::is_iterable_v<T>, lit_ns::Lit<T>> lit(T t)
        {
            return lit_ns::Lit<T>(move(t));
        }

        lit_ns::Lit<string_view> lit(const char* t)
        {
            return lit(string_view(t));
        }
    }
}
