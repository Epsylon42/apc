#include <string>
#include <tuple>
#include <optional>
#include <variant>
#include <utility>
#include <functional>
#include <type_traits>

#include "apc.hpp"

using namespace std;

template< typename T >
void print(const T& t)
{
    cout << t << ' ';
}

template< typename T, typename... Ts >
void print(const tuple<T, Ts...>& t)
{
    print(get<0>(t));

    if constexpr (sizeof...(Ts) != 0)
    {
        apply([](auto head, auto... tail)
              {
                  print(make_tuple(tail...));
              }, t);
    }
}

int main()
{
    using namespace apc::res;
    using namespace apc::parsers;

    auto parser
        = sequence(
            unit('a'),
            hide(unit('b')),
            unit('c')
            );

    for (string_view s : { "abc", "aec", "ab" })
    {
        auto res = parser.parse(s.begin(), s.end());

        if (is_ok(res))
        {
            print(unwrap_ok(move(res)).res);
            cout << endl;
        }
        else if (is_err(res))
        {
            unwrap_err(move(res)).err.print_trace();
        }
        else
        {
            unwrap_eoi(move(res)).print_trace();
        }

        cout << endl << endl;
    }
}
