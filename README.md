## APC

APC is a header-only parser combinator library whose goals is to be extensible, be generic over every imaginable type of input and have good error handling (which is inspired by that of Rust programming language).

It's not done yet, obviously.

### parsers

Parser is a structure which has two associated types `Ok` and `Err` and a function `parse` possibly also `parse_with_sep` in the future. `parse` has to be generic over iterators (generic iterator type will be called `I`). `parse` has this signature: `apc::res::Result<Ok, Err, I> parse(I, I)`. To parse something you call `parse` with two iterators specifying the range.

Default parsers have names that start with and Uppercase letter and they with their helper types and functions are located in `res::parsers::name_ns` where `name` is the name of the parser all lowercase. Namespace `res::parsers` has functions that have the same name as their respective parsers but in all lowercase.

###### currently implemented parsers

* unit(T): returns T if the first element of input is equal to T. Otherwise returns error
*lit(T): accepts something iterable and returns it if the input matches. Otherwise returns error
* sequence(Ps...): accepts one or more parsers and executes them in sequence. Returns tuple of their results without `NilOk`s if they all succeed or error if at least one fails
* alt(Ps...): accepts one or more parsers and executes them in sequence. Returns the result of the first successful parser. If they have the same return type, returns it, otherwise, returns a `variant`
* hide(P): accepts one parser and executes it but replaces its return type with `NilOk`
* many(P): accepts one parser and executes it until it fails. Returns vector of values
* any: accepts a type parameter and returns anything
* map(P, F) accepts a parser and a function. Returns the output of function applied to the parser's `Ok`.

### error handling

Parser result is represented with `Result<T, E, I>` where `T` is success type, `E` is error type and `I` is iterator type. `Result` is just a `variant` with its arguments as `Ok<T, I>`, `Err<T, I>` and `EOI` (end of input). If a parser does not return a value, `T` is equal to `NilOk`. If a parser can't fail, `E` is equal to `NilErr`.

`Ok` has two fields: `res` is a result and `pos` is an iterator pointing after the end if consumed part.

`Err` has two fields: `err` is an error and `pos` is an iterator pointing at the position where the error occured.

`EOI` currently has just a "stack trace".

There is a function print_trace() which accepts either `E` or `EOI` and prints a human-readable description

 `E` must have a field `prev` which can be a previous error, optional previous error, variant containing previous errors or `NilErr`. It also must have a function `tuple<string, size_t> description()` where string is human-readable description of the error, and size\_t is where the previous error happened relative to the current one.

`Result` has helper methods for doing things
* Check the contents: `res.is_*()`
* Get the contents: `res.unwrap_*()`
* Change the content:
  - `res.map_*(F)` accepts a function that accepts and returns ok/err, applies it to ok/err inside the result and returns a result with changed value
  - `res.fmap_*(F)` accepts a function that accepts ok/err and returns a result, applies it to ok/err/eoi inside the result and returns whatever it returned. If you pass a lambda you should explicitly set its return type i.e. `Result<something>`. If you don't, you are going to have a bad time. Really.
  - `res.visit_*(F)` accepts a function that accepts ok/err/eoi (possibly by referece) and returns void and applies it to ok/err/eoi


`apc::result` also has helper functions for constructing things
* ok(T, I)
* err(E, I)

## I'm tired and this readme is complex enough. I'll finish it when the library is actually done
