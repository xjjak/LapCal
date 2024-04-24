#lang racket/gui

(require plot)
(require "plot-canvas.rkt")

(define frame
  (new frame% [label "live-plotting"]))

(define plotter
  (new plot-canvas%
       [parent frame]
       [size 200]
       [min-value -1]
       [max-value 6]))

(send frame show #t)
