## APC

APC is a header-only parser combinator library whose goals is to be extensible, be generic over every imaginable type of input and have good error handling (which is inspired by that of Rust programming language).

It's not done yet, obviously.

### parsers

Parser is a structure which has two associated types `Ok` and `Err` and a function `parse` possibly also `parse_with_sep` in the future. `parse` has to be generic over iterators (generic iterator type will be called `I`). `parse` has this signature: `apc::res::Result<Ok, Err, I> parse(I, I)`. To parse something you call `parse` with two iterators specifying the range.

Default parsers have names that start with and Uppercase letter and they with their helper types and functions are located in `res::parsers::name_ns` where `name` is the name of the parser all lowercase. Namespace `res::parsers` has functions that have the same name as their respective parsers but in all lowercase.

###### currently implemented parsers

* unit(T): returns T if the first element of input is equal to T. Otherwise returns error
* sequence(Ps...): accepts one or more parsers and executes them in sequence. Returns tuple of their results without `NilOk`s if they all succeed or error if at least one fails
* hide(P): accepts one parser and executes it but replaces its return type with `NilOk`

### error handling

Parser result is represented with `Result<T, E, I>` where `T` is success type, `E` is error type and `I` is iterator type. `Result` is just a `variant` with its arguments as `Ok<T, I>`, `Err<T, I>` and `EOI` (end of input). If a parser does not return a value, `T` is equal to `NilOk`. If a parser can't fail, `E` is equal to `NilErr`.

`Ok` has two fields: `res` is a result and `pos` is an iterator pointing after the end if consumed part.

`Err` has two fields: `err` is an error and `pos` is an iterator pointing at the position where the error occured.

`EOI` currently has just a "stack trace".

`Err` and `EOI` have a function `print_trace()` to print a human-readable error description.

`E` should inherit `apc::res::Error` type and override `std::string description(size_t offset)` and `Error& previous()`. `description` should return a human-readable error description and call `description(offset+n)` on the previous error where `n` is where the inner parser failed relative to the current one. `previous` has to return a reference to the previous error or to any `NilErr` if there isn't one.

`apc::res` has helper functions for dealing with results:
Check the contents:
* is_ok(res)
* is_err(res)
* is_eoi(res)

Get the contents:
* unwrap_ok(res)
* unwrap_err(res)
* unwrap_eoi(res)

Construct ok/err:
* ok(T, I)
* err(T, I)

## I'm tired and this readme is complex enough. I'll finish it when the library is actually ready
