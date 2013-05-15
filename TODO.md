TODO
====

* interpreter for output
* Earley parser for bits?
* implement *define*
    * define *true*, *false*, *cons*, *empty*, ...
    * place *eval* reading from input into lambda expression?
* add *quote* and *eval* to environment (<-> *read\_bit*?)
* add input, eval, and quote without introducing new types?
* implement meta-circular interpreter
* primitives (implement using quote and eval?)
    * quote, null, true, false, cons, (1 . ()), first, rest
    * define, list, eq?, not, null?, pair?, and, or, xor, if
    * member?, append, assoc, cond, equal?
    * eval, quasiquote, quote
    * sublis (substitution)
* ffi
* binary tree tokenizer, compose grammars
* overload representation of pairs?
* macros
* read from list
