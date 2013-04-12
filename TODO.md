TODO
====

* test I/O with unlimited length
* add *quote* and *eval* to environment (<-> *read\_bit*?)
* add input, eval, and quote without introducing new types?
* implement *define*: place *eval* reading from input into lambda expression?
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
* use BLC list as stack for garbage collector?
* read from list
