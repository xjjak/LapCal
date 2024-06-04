;; -*- eval: (setenv "LD_LIBRARY_PATH" "/home/palisn/.nix-profile/lib") -*-
#lang racket/gui

;; TODO: add to config when changing text fields (at least at runtime)

(require "gui/plot-canvas.rkt")
(require "gui/live-interface.rkt")
(require "gui/canvas-configurator.rkt")

(require "configuration/configuration.rkt")

(define config-file "config.json")
(define config
  (if (file-exists? config-file)
      (load-config config-file)
      (let ([cfg (generate-default-config)])
        (write-config cfg config-file)
        cfg)))

(define sensor-index 0)
(define value-index 0)

(define frame
  (new (class frame%
         (init)
         (super-new [label "float"]
                    [width 900]
                    [height 600])

         (define/augment (on-close)
           (send live-interface disconnect)))))

(define container
  (new horizontal-panel% [parent frame]))

(define plotter
  (new plot-canvas% [parent container]
       [size (get-attribute config sensor-index value-index 'size)]
       [min-value (get-attribute config sensor-index value-index 'min)]
       [max-value (get-attribute config sensor-index value-index 'max)]))

(define sidebar
  (new tab-panel% [parent container]
       [choices '("Tab 1" "Tab 2")]
       [stretchable-width #f]))

(define live-interface
  (new live-interface% [parent sidebar]
       [on-input
        (λ (value)
          (send plotter push value))]
       [default-sensor-index sensor-index]
       [default-value-index value-index]))


(define canvas-configurator
  (new canvas-configurator% [parent sidebar]
       [config config]
       [default-sensor-index sensor-index]
       [default-value-index value-index]
       [on-update
        (λ (sensor-idx value-idx size min-value max-value)
          (send live-interface set-sensor sensor-idx)
          (send live-interface set-value value-idx)
          (send plotter set-size size)
          (send plotter set-min-value min-value)
          (send plotter set-max-value max-value))]))


(send frame show #t)
