TODO
====

* better garbage collector? reflective?
* tail-call optimisation
* id = ->x.x
  id(true)
* obj.fun
  (send obj fun)
  (1 . ())
* go meta-circular
  binary representation for proc, wrap, input, output
  forward declaration?
* IO (monad) with FFI
  stdin, stdout, self?: constants
* symbols/strings?
* inspect methods
* implement object system 
  namespaces, contexts, objects and polymorphism, inspect method
* quote expr -> list of booleans
* eval list -> expression
* write expression to list
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
