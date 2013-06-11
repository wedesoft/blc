TODO
====

* interpreter for output
  011 011 011 011 011 011 011 ->a.a           000000000010 1110 11110 111110 1111110 11111110 111111110
  111110
  011 011 011 011 011 011 011 ->a->b.a        000000000010 1110 11110 111110 1111110 11111110 111111110
  1111110
  011 011 011 011 011 011 011 ->a->b.b        000000000010 1110 11110 111110 1111110 11111110 111111110
  10
  011 011 011 011 011 011 011 ->a b c.(a b c) 000000000010 1110 11110 111110 1111110 11111110 111111110
  111110
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
