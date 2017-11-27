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
#include <type_traits>

namespace apc::res
{
    using namespace std;

    struct NilOk {};

    struct NilErr {};

    //TODO: make it work with consts
    template< typename E >
    void print_trace(E& err, ostream& out = cerr, size_t offset = 0, bool first = true)
    {
        if constexpr (is_same_v<E, NilErr>)
        {
            return;
        }
        else
        {
            if constexpr (misc::is_optional(err))
            {
                if (err.has_value())
                {
                    print_trace(*err, out, offset, first);
                }
            }
            else if constexpr (misc::is_variant(err))
            {
                visit([&out, offset, first](auto& e)
                        {
                            print_trace(e, out, offset, first);
                        }, err);
            }
            else
            {
                auto [ desc, e_offset ] = err.description();

                if (first)
                {
                    out << "Error trace" << endl;
                }

                out << "\t" << "At " << offset+1 << ' ' << desc << endl;

                print_trace(err.prev, out, offset + e_offset, false);
            }
        }
    }


    struct EOI
    {
        vector<string> trace;

        EOI() : trace() {}
        EOI(string&& s) : trace{s} {}
        EOI(string& s) : trace{s} {}

    };

    void print_trace(EOI& eoi, ostream& out = cerr)
    {
        out << "End of input" << endl;

        for (auto iter = eoi.trace.crbegin(); iter != eoi.trace.crend(); iter++)
        {
            out << "\tIn " << *iter << endl;
        }
    }

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
    struct Result
    {
        variant<Ok<T, I>, Err<E, I>, EOI> inner;

        Result(Result<T, E, I>& res) = default;
        Result(Result<T, E, I>&& res) = default;
        Result(const Result<T, E, I>& res) = default;

        template< typename... Ts >
        Result(Ts&&... ts) : inner(forward<Ts>(ts)...) {}

        bool is_ok() const
        {
            return holds_alternative<Ok<T, I>>(inner);
        }

        bool is_err() const
        {
            return holds_alternative<Err<E, I>>(inner);
        }

        bool is_eoi() const
        {
            return holds_alternative<EOI>(inner);
        }

        Ok<T, I>& unwrap_ok()
        {
            return get<Ok<T, I>>(inner);
        }

        Err<E, I>& unwrap_err()
        {
            return get<Err<E, I>>(inner);
        }

        EOI& unwrap_eoi()
        {
            return get<EOI>(inner);
        }

        //TODO: use more moves
        template< typename F >
        Result<decltype(invoke_result_t<F, Ok<T, I>&>::res), E, I> map_ok(F pred)
        {
            if (is_ok())
            {
                return pred(unwrap_ok());
            }
            else if (is_err())
            {
                return unwrap_err();
            }
            else
            {
                return unwrap_eoi();
            }
        }

        template< typename F >
        Result<T, decltype(invoke_result_t<F, Err<E, I>&>::err), I> map_err(F pred)
        {
            if (is_err())
            {
                return pred(unwrap_err());
            }
            else if (is_ok())
            {
                return unwrap_ok();
            }
            else
            {
                return unwrap_eoi();
            }
        }

        template< typename F >
        Result<T, E, I> map_eoi(F pred)
        {
            if (is_eoi())
            {
                return pred(unwrap_eoi());
            }
            else if (is_ok())
            {
                return unwrap_ok();
            }
            else
            {
                return unwrap_err();
            }
        }

        template< typename F >
        Result<T, E, I>& visit_ok(F pred)
        {
            if (is_ok())
            {
                pred(unwrap_ok());
            }

            return *this;
        }

        template< typename F >
        Result<T, E, I>& visit_err(F pred)
        {
            if (is_err())
            {
                pred(unwrap_err());
            }

            return *this;
        }

        template< typename F >
        Result<T, E, I>& visit_eoi(F pred)
        {
            if (is_eoi())
            {
                pred(unwrap_eoi());
            }

            return *this;
        }

        template< typename F >
        invoke_result_t<F, Ok<T, I>&> fmap_ok(F pred)
        {
            if (is_ok())
            {
                return pred(unwrap_ok());
            }
            else if (is_err())
            {
                return unwrap_err();
            }
            else
            {
                return unwrap_eoi();
            }
        }

        template< typename F >
        invoke_result_t<F, Err<E, I>&> fmap_err(F pred)
        {
            if (is_err())
            {
                return pred(unwrap_err());
            }
            else if (is_ok())
            {
                return unwrap_ok();
            }
            else
            {
                return unwrap_eoi();
            }
        }

        template< typename F >
        invoke_result_t<F, EOI&> fmap_eoi(F pred)
        {
            if (is_eoi())
            {
                return pred(unwrap_eoi());
            }
            else if (is_ok())
            {
                return unwrap_ok();
            }
            else
            {
                return unwrap_err();
            }
        }
    };
}
