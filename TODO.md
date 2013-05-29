TODO
====

* just modify lambda_bison.y (i.e. don't introduce 011)?
* meta-circular interpreter
    * implement read_expression: input -> expression x input
    * problem with reading input in meta-circular evaluator
    * implement *define/let*
        * define *true*, *false*, *cons*, *empty*, ...
        * place *eval* reading from input into lambda expression?
* error handling
* named parameters, structures
* namespaces, contexts, objects and polymorphism
  (first list) -> list.first
* read from list
* interpreter for output
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
