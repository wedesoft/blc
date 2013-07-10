TODO
====

* quote expr -> list of booleans
* remove normalise
* eval list -> expression
* print to list; read_bit <-> first, rest (split)
  (output true)
  1#<output>
  ((output true) false)
  10#<output>
  (pair true null)
  00011011100000110000010
  (pair false (pair true null))
  00011011100000100110110000000110111011101100000110000010
  (pair false (pair true #<output>))
* primitives (implement using quote and eval?)
  * eq?
  * list, null?, pair?, xor
  * member?, append, assoc, cond, equal?
  * eval, quasiquote (quote with environment), quote
  * sublis (substitution)
* implement object system 
  namespaces, contexts, objects and polymorphism, inspect method
  (first list) -> list.first
  (1 . ())
* better garbage collector? reflective?
* direct integration of BLC (no need for definitions)?
* continuation passing style, make environment visible?
* error handling
* named parameters, structures
* Earley parser for bits?
* add *quote* and *eval* to environment (<-> *read\_bit*?)
* add input, eval, and quote without introducing new types?
* ffi
* binary tree tokenizer, compose grammars
* overload representation of pairs?
* macros
