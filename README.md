blc
===

**Author**:       Jan Wedekind
**Copyright**:    2013
**License**:      GPL

Synopsis
--------

Binary Lambda Calculus virtual machine

Installation
------------

This is how to build the project under GNU/Linux.

    make -f Makefile.dist
    ./configure
    make

Usage
-----

The [REPL](http://en.wikipedia.org/wiki/Read-eval-print\_loop) can be started as follows.

    ./run.sh

Example
-------

Here's an example program consuming zeros until it encounters '1' or the end of input.

    I      = ->self.self
    true   = ->first second.first
    false  = ->first second.second
    not    = ->value.(value false true)
    or     = ->x y.(x true y)
    and    = ->x y.(x y false)
    if     = ->condition consequent alternative.(condition consequent alternative)
    null   = false
    pair   = ->first rest.->select.(select first rest)
    first  = ->pair.(pair true)
    rest   = ->pair.(pair false)
    empty? = ->list.(list ->first rest bool.false true)
    Y      = ->self.(->arg.(self (arg arg)) ->arg.(self (arg arg)))

    ((Y ->f input.((first input) true (f (rest input)))) input)

    001

Testing
-------

The tests can be run using the following command.

    make test

See Also
--------

* [TODO](TODO.html)

External Links
--------------

* [Binary lambda calculus interpreter by John Tromp](http://homepages.cwi.nl/~tromp/cl/cl.html)
* [De Bruijn index](http://en.wikipedia.org/wiki/De\_Bruijn\_index)
* [Maru: a metacircular s-expression evaluator and compiler](http://piumarta.com/software/maru/) (also see [here](https://github.com/kstephens/maru))
* [Nile Programming Language: Declarative Stream Processing for Media Applications](https://github.com/damelang/nile)
* [Jitblt: Efficient Run-time Code Generation for Digital Compositing](http://www.vpri.org/pdf/tr2008002\_jitblt.pdf)
* [Church encoding](http://en.wikipedia.org/wiki/Church\_encoding)
* [Nokolisp](http://koti.welho.com/tnoko/Nokolisp.htm)
* [Racket Programming Language](http://www.racket-lang.org/)
* [Revised⁶ Report on the Algorithmic Language Scheme](http://www.r6rs.org/)
* [Scheme implemented in Python](https://github.com/codebox/scheme-interpreter/blob/master/scheme.py)
* [Lisp as the Maxwell’s equations of software](http://www.michaelnielsen.org/ddi/lisp-as-the-maxwells-equations-of-software/) (also see [here](http://gliese1337.blogspot.co.uk/2012/04/schrodingers-equation-of-software.html))
* [STEPS Toward Espressive Programming Systems, 2011 Progress Report](http://www.vpri.org/pdf/tr2011004\_steps11.pdf)
* [Kivy cross platform UI](http://kivy.org/)
* [Qemu open source processor emulator](http://qemu.org/Manual)
* [Fisher: Control Structures for Programming Languages](http://reports-archive.adm.cs.cmu.edu/anon/anon/usr/ftp/scan/CMU-CS-70-fisher.pdf)
* [Peck: Theory Time - A pure lambda-calculus foundation for prototype-based OOP](http://blog.suspended-chord.info/2012/10/19/theory-time-a-pure-lambda-calculus-foundation-for-prototype-based-oop/)
* [Sitaram: Teach Yourself Scheme in Fixnum Days](http://download.plt-scheme.org/doc/300/pdf/t-y-scheme.pdf)
* [Might: How to compile with continuations](http://matt.might.net/articles/cps-conversion/)
* [Filinski: Declarative Continuations and Categorical Duality](http://www.diku.dk/hjemmesider/ansatte/andrzej/papers/DCaCD.ps.gz)
* [Earley parser](en.wikipedia.org/wiki/Earley\_parser)
* [Binary numbers](http://en.wikipedia.org/wiki/Binary\_number)
