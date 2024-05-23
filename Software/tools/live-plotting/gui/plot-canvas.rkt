#lang racket/gui

(provide plot-canvas%)

(require plot)

(define plot-canvas%
  (class canvas%
    (init parent
          [(internal-size size) 50]
          [value-label #f]
          [(internal-min-value min-value) #f]
          [(internal-max-value max-value) #f]
          [(internal-name name) #f])

    (define size internal-size)
    (define buffer (make-vector size 0))
    (define index 0)

    (define/public (set-size new-size)
      (set! size new-size)
      (set! buffer (make-vector size 0))
      (set! index 0)
      (send this on-paint))
    
    (define min-value internal-min-value)
    (define max-value internal-max-value)
    (define/public (set-min-value value)
      (set! min-value value)
      (send this on-paint))
    (define/public (set-max-value value)
      (set! max-value value)
      (send this on-paint))
    (define/public (set-bounds bounds)
      (match bounds
        [(list a b) (set-min-value a) (set-min-value b)]
        [(cons a b) (set-min-value a) (set-min-value b)]))
    
    (define name internal-name)
    (define/public (set-name text)
      (set! name text)
      (send this on-paint))

    (define default-pen #f)
    
    (super-new
     [parent parent]
     [paint-callback
      (λ (canvas dc)
        (send dc clear)
        (send dc set-smoothing 'aligned)
        
        (define cwidth (send canvas get-width))
        (define cheight (send canvas get-height))
        (define text-width
          (λ (text)
            (let-values ([(w h c d) (send dc get-text-extent text)])
              w)))
        (define text-height
          (λ (text)
            (let-values ([(w h c d) (send dc get-text-extent text)])
              h)))
        (define padding 5)

        (unless default-pen
          (set! default-pen (send dc get-pen)))
        
        (let ([size-text (format "~s (~~~s s)" size (* 0.02 size))]
              [window-width
               (max 0
                (- cwidth
                   (apply max (map text-width (list (number->string min-value)
                                                    (number->string max-value))))
                   padding))]
              [min-text (number->string min-value)]
              [max-text (number->string max-value)]
              [y-zero (- cheight (* (/ (- min-value) (- max-value min-value))
                                    cheight))]
              [zero-text "0"])
          (send dc set-pen (send default-pen get-color)
                           (* 3 (send default-pen get-width))
                           (send default-pen get-style))
          
          (send dc draw-text size-text 0 (- cheight (text-height size-text)))

          (send dc draw-line window-width 0 window-width cheight)
          
          (send dc draw-text min-text
                (+ window-width padding)
                (- cheight (text-height min-text)))
          (send dc draw-text max-text (+ window-width padding) 0)

          (send dc set-pen (send default-pen get-color)
                           (* 2 (send default-pen get-width))
                           'long-dash)
          
          (send dc draw-line 0 y-zero window-width y-zero)
          (send dc draw-text zero-text
                (+ window-width padding)
                (- y-zero (/ (text-height zero-text) 2)))

          ; draw the plot
          (send dc set-clipping-rect 0 0 window-width cheight)
          (send dc set-pen "red"
                           (* 2 (send default-pen get-width))
                           (send default-pen get-style))
          (let* ([width window-width]
                 [height cheight]
                 [value-range (- max-value min-value)]
                 [remap-y (λ (y) (- height (* (/ (- y min-value) value-range)
                                              height)))]
                 [remap-x (λ (x) (if (size . > . 1)
                                     (* width (/ (+ x (sub1 size)) (sub1 size)))
                                     (/ width 2)))]
                 [y-values (let-values ([(left right) (vector-split-at buffer index)])
                             (vector-append right left))]
                 [points (for/list ([x (inclusive-range (- 1 size) 0)]
                                    [y y-values])
                           (cons (remap-x x) (remap-y y)))])
            (send dc draw-lines points))
          (send dc set-clipping-region #f)

          (send dc set-pen default-pen)))])

    (define/public (push value)
      (vector-set! buffer index value)
      (set! index (modulo (add1 index) size))
      (send this on-paint))

    (define/public (clear)
      (vector-fill! buffer 0)
      (set! index 0)
      (send this on-paint))))
