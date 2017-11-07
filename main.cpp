#include <string>
#include <tuple>
#include <optional>
#include <variant>
#include <utility>
#include <functional>
#include <type_traits>
#include <random>
#include <chrono>

#include "apc.hpp"

using namespace std;

template< typename T >
void print(const T& t)
{
    cout << t << ' ';
}

template< typename... Ts >
void print(const variant<Ts...>& t)
{
    visit([](auto& t)
          {
              print(t);
          }, t);
}

template< typename T >
void print(const vector<T>& t)
{
    for (const auto& e : t)
    {
        print(e);
    }
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

    string in = "ab{b}";

    auto parser =
        map(
            many_str<char>(),

            [](string s)
            {
                for (char& c : s)
                {
                    c++;
                }

                return s;
            }
            );

    auto res = parser.parse(in.begin(), in.end());
    if (is_ok(res))
    {
        print(unwrap_ok(move(res)).res);
    }
    else if (is_err(res))
    {
        print_trace(unwrap_err(move(res)).err);
    }
    else
    {
        print_trace(unwrap_eoi(move(res)));
    }
}
