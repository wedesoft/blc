TODO
====

* recursive *print\_expression* using local environments
    * interpreter for output
* direct integration of BLC (no need for definitions)?
* continuation passing style, make environment visible?
* error handling
* named parameters, structures
* namespaces, contexts, objects and polymorphism
  (first list) -> list.first
* read from list
* Earley parser for bits?
* add *quote* and *eval* to environment (<-> *read\_bit*?)
* add input, eval, and quote without introducing new types?
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
