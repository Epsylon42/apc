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

    for (string_view in : { "ababb", "aba", "acac", "a" }) {
        auto parser =
            sequence(
                hide(
                    many(sequence(unit('a'), unit('b')))
                    .at_least(2)
                    .at_most(4)
                    ),

                unit('b')
                );


        match(parser.parse(in.begin(), in.end()),
              [](auto res_ok)
              {
                  print(res_ok.res);
                  cout << endl << endl;
              },
              [](auto res_err)
              {
                  print_trace(res_err.err);
                  cout << endl;
              },
              [](auto res_eoi)
              {
                  print_trace(res_eoi);
                  cout << endl;
              });
    }
}
