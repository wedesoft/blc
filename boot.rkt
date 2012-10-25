(define #t (lambda (x y) x))
(define #f (lambda (x y) y))
(define not (lambda (b) (lambda (x y) (b y x))))
(define if (lambda (b x y) (b x y)))
