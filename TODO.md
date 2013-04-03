TODO
====

* guard input monad
* add *quote* and *eval* to environment
* add input, eval, and quote without introducing new types?
* implement *define*: place *eval* reading from input into lambda expression?
* *readBit* function (monadic IO) <-> quote, eval?
* implement meta-circular interpreter
* primitives (implement using quote and eval?)
    * quote, null, true, false, cons, (1 . ()), first, rest
    * define, list, eq?, not, null?, pair?, and, or, xor, if
    * member?, append, assoc, cond, equal?
    * eval, quasiquote, quote
    * sublis (substitution)
* ffi
* Earley parser for bits?
* binary tree tokenizer, compose grammars
* overload representation of pairs?
* macros
