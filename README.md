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

    input  = 10
    output = 1110
    I      = λself.self
    true   = λfirst second.first
    false  = λfirst second.second
    if     = λcondition consequent alternative.(condition consequent alternative)
    null   = false
    pair   = λfirst rest. λselect.(select first rest)
    first  = λpair.(pair true)
    rest   = λpair.(pair false)
    empty  = λlist.(list λfirst rest bool.false true)
    Y      = λself.(λarg.(self (arg arg)) λarg.(self (arg arg)))

    ((Y λf input.((first input) true (f (rest input)))) input)

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
* [LYSP: a tiny lightweight Lisp interpreter](http://piumarta.com/software/lysp/)
* [Racket Programming Language](http://www.racket-lang.org/)
* [Revised⁶ Report on the Algorithmic Language Scheme](http://www.r6rs.org/)
* [Scheme implemented in Python](https://github.com/codebox/scheme-interpreter/blob/master/scheme.py)
* [Repl.It: Online REPL for several programming languages](http://repl.it/)
* [Lisp as the Maxwell’s equations of software](http://www.michaelnielsen.org/ddi/lisp-as-the-maxwells-equations-of-software/) (also see [here](http://gliese1337.blogspot.co.uk/2012/04/schrodingers-equation-of-software.html))
* [STEPS Toward Espressive Programming Systems, 2011 Progress Report](http://www.vpri.org/pdf/tr2011004\_steps11.pdf)
* [Kivy cross platform UI](http://kivy.org/)
* [Qemu open source processor emulator](http://qemu.org/Manual)
* [Archon: Directly Reflective Meta-Programming](http://homepage.divms.uiowa.edu/~astump/papers/archon.pdf)
* [Wand: The Theory of Fexprs is Trivial](ftp://ftp.ccs.neu.edu/pub/people/wand/papers/fexprs.ps)
* [Mogensen: Efficient Self-Interpretation in Lambda Calculus](http://repository.readscheme.org/ftp/papers/topps/D-128.pdf)
* [Peck: Theory Time - A pure lambda-calculus foundation for prototype-based OOP](http://blog.suspended-chord.info/2012/10/19/theory-time-a-pure-lambda-calculus-foundation-for-prototype-based-oop/)
