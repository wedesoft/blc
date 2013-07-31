TODO
====

* go meta-circular
  binary representation for proc, wrap, input, output
  forward declaration?
* IO (monad) with FFI
  stdin, stdout, self?: constants
  read_bit, write_bit, read_char, write_char: methods
* symbols/strings?
* inspect methods
* implement object system 
  namespaces, contexts, objects and polymorphism, inspect method
  (first list) -> list.first
  (1 . ())
* continuation passing style, tail-call optimisation
* better garbage collector? reflective?
* have write_expr produce lists,
  quote creates list or error
* (eval (quote input))
* quote expr -> list of booleans
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
* error handling
* named parameters, structures
* Earley parser for bits?
* add *quote* and *eval* to environment (<-> *read\_bit*?)
* ffi
* binary tree tokenizer, compose grammars
* overload representation of pairs?
* macros
