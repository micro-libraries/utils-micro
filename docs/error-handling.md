
The C++ standard has gradually defined various features for handling errors or unexpected situations.
(We shall use the terms *errors*, *unexpected/exceptional situations/outcomes/returns* interchangeably.)

The most obnoxious among them is throwing and catching exceptions, which is disabled in many C++ projects
(a summary of reasons is provided [here](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0709r4.pdf)).
This collection of micro libraries recommends disabling exceptions and does not `throw` any by itself, but it may
cooperate with libraries that do. All places where exceptions could propagate through are marked
with `noexcept(conditions)`; otherwise, `noexcept` is implicit. If an external call from a `noexcept` function isn't
wrapped in `try/catch`, it's expected to also never throw.

We recognize the following types of *errors* and ways to handle them:

- **Fatal errors** such as failure to allocate or hardware errors. These result in immediate termination
  of the running program and are handled by system libraries.
- **Precondition violations.** The recommended practice is to not validate preconditions on the callee side, but state
  them clearly in documentation and leave it to the caller. Nonetheless, for debugging purposes, they may be validated
  using assertions such as `utils_micro::AssertionHandler`.
- **Sanity checks.** Unlike preconditions, these don't validate inputs but any intermediate results and serve both as minor
  debugging assistance and statements of non-obvious properties that should hold at some points. These checks should
  also be performed as assertions using `utils_micro::AssertionHandler`.
- **Complex validation** of specific properties of data. Sometimes, preconditions may be too complex for the rule
  "leave it to the caller". In that case, we should provide a validation function as a normal part of public API 
  if possible; separating validation from further processing is the best approach.
- **Intermediate errors.** Sometimes the separation above is infeasible, especially if invalid properties of input
  are revealed during a complex algorithm. In that case, an unexpected result describing the error is passed to
  the caller and the required properties of input aren't documented as preconditions, but possible error returns.
- **External errors.** Typical examples are access to an external resource or an unknown failure from an external library.
  These must be passed to the caller as we can't decide what to do.
- **Any other recoverable errors.** It's possible to do nothing wrong and still fail. Probabilistic algorithms are one 
  example. We pass these to the caller as well.

Opinions on how error handling should be approached widely vary. We declare that the best API design is to ensure
each function call has a single, obvious outcome (while assuming the inputs satisfy given preconditions), or at least
to limit cases where this is infeasible to a minimum.

Within this collection of micro libraries, `std::expected` is returned from functions that could have an unexpected
outcome. Since constructors cannot return values, any constructor that does non-trivial work and could fail for reasons
known to us falls under the case of complex validation. It should be implemented as a factory function that attempts
to parse/validate input data and constructs a desired object in case of success, returning `std::expected`.

Errors should thus never silently pass through. This rule is only feasible if combined with the previous
ones - no input validation and as few unexpected outcomes as possible. We wrap all *known* external errors
(including thrown exceptions) into `std::expected`, but when possible outcomes of calls depend on template parameters,
we allow silent propagation of exceptions, only marking our functions as `noexcept(conditions)`. Such cases should
also only be allowed to the minimum possible extent.

In validation functions and similar, for which success is just as expected as any specific failure, `std::outcome`
may be used. Otherwise, we avoid the error code approach where success is syntactically indistinguishable from failure.
Errors should be rare, so their handling can and should be visible.

Sometimes a failure can be naturally described as a type of expected result. For example, object detection can fail.
If we return a sequence of detected objects, it naturally takes care of that situation and also situations where
multiple objects were detected.
