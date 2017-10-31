#pragma once

#include <string>
#include <tuple>
#include <optional>
#include <variant>
#include <functional>
#include <string_view>
#include <vector>
#include <ostream>
#include <iostream>

namespace apc
{
    using namespace std;

    namespace res
    {
        struct NilOk {};

        struct Error
        {
            virtual std::string description(size_t offset) = 0;
            virtual Error& previous() = 0;
            virtual bool is_nil()
            {
                return false;
            }

            void print_trace(std::ostream& out = std::cerr)
            {
                out << description(0) << std::endl;
            }
        };

        struct NilErr : Error
        {
            virtual string description(size_t offset = 0) override
            {
                return "NilErr";
            }

            virtual Error& previous() override
            {
                return *this;
            }

            virtual bool is_nil() override
            {
                return true;
            }
        };

        struct EOI
        {
            vector<string> trace;

            EOI() : trace() {}
            EOI(string&& s) : trace{s} {}
            EOI(string& s) : trace{s} {}

            void print_trace(ostream& out = cerr) const
            {
                out << "End of input" << endl;

                for (auto iter = trace.crbegin(); iter != trace.crend(); iter++)
                {
                    out << "\tIn " << *iter << endl;
                }
            }
        };

        template< typename T, typename I >
        struct Ok
        {
            T res;
            I pos;

            Ok(T res, I pos) : res(move(res)), pos(move(pos)) {}
        };

        template< typename E, typename I >
        struct Err
        {
            E err;
            I pos;

            Err(E err, I pos) : err(move(err)), pos(move(pos)) {}
        };

        template< typename T, typename I >
        Ok<T, I> ok(T t, I i)
        {
            return Ok<T, I>(move(t), move(i));
        }

        template< typename E, typename I >
        Err<E, I> err(E e, I i)
        {
            return Err<E, I>(move(e), move(i));
        }


        template< typename T, typename E, typename I >
        using Result = variant<Ok<T, I>, Err<E, I>, EOI>;


        template< typename T, typename E, typename I >
        bool is_ok(const Result<T, E, I>& res)
        {
            return holds_alternative<Ok<T, I>>(res);
        }

        template< typename T, typename E, typename I >
        bool is_err(const Result<T, E, I>& res)
        {
            return holds_alternative<Err<E, I>>(res);
        }

        template< typename T, typename E, typename I >
        bool is_eoi(const Result<T, E, I>& res)
        {
            return holds_alternative<EOI>(res);
        }


        template< typename T, typename E, typename I >
        Ok<T, I>& unwrap_ok(Result<T, E, I>& res)
        {
            return get<Ok<T, I>>(res);
        }

        template< typename T, typename E, typename I >
        Ok<T, I> unwrap_ok(Result<T, E, I>&& res)
        {
            return get<Ok<T, I>>(move(res));
        }

        template< typename T, typename E, typename I >
        const Ok<T, I>& unwrap_ok(const Result<T, E, I>& res)
        {
            return get<Ok<T, I>>(res);
        }


        template< typename T, typename E, typename I >
        Err<E, I>& unwrap_err(Result<T, E, I>& res)
        {
            return get<Err<E, I>>(res);
        }

        template< typename T, typename E, typename I >
        Err<E, I> unwrap_err(Result<T, E, I>&& res)
        {
            return get<Err<E, I>>(move(res));
        }

        template< typename T, typename E, typename I >
        const Err<E, I>& unwrap_err(const Result<T, E, I>& res)
        {
            return get<Err<E, I>>(res);
        }


        template< typename T, typename E, typename I >
        EOI& unwrap_eoi(Result<T, E, I>& res)
        {
            return get<EOI>(res);
        }

        template< typename T, typename E, typename I >
        EOI unwrap_eoi(Result<T, E, I>&& res)
        {
            return get<EOI>(move(res));
        }

        template< typename T, typename E, typename I >
        const EOI& unwrap_eoi(const Result<T, E, I>& res)
        {
            return get<EOI>(res);
        }


        template< typename T, typename E, typename I,
                  typename MOK, typename MERR, typename MEOI >
        void match (Result<T, E, I> res,
                    MOK match_ok,
                    MERR match_err,
                    MEOI match_eoi
            )
        {

            if (is_ok(res))
            {
                match_ok(unwrap_ok(move(res)));
            }
            else if (is_err(res))
            {
                match_err(unwrap_err(move(res)));
            }
            else
            {
                match_eoi(unwrap_eoi(move(res)));
            }
        }

        template< typename T, typename T1, typename E, typename E1, typename I,
                  typename MOK, typename MERR, typename MEOI >
        Result<T1, E1, I> match (Result<T, E, I> res,
                                 MOK match_ok,
                                 MERR match_err,
                                 MEOI match_eoi
            )
        {

            if (is_ok(res))
            {
                return match_ok(unwrap_ok(move(res)));
            }
            else if (is_err(res))
            {
                return match_err(unwrap_err(move(res)));
            }
            else
            {
                return match_eoi(unwrap_eoi(move(res)));
            }
        }
    }
}
