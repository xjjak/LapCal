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
       [choices null]
       [stretchable-width #f]
       [callback
        (λ (b e)
          (when (eq? (send e get-event-type) 'tab-panel)
            (set-tab-panel (send b get-selection))))]))


(define live-interface (new live-interface% [parent sidebar]
                            [on-input
                             (λ (value) (send plotter push value))]
                            [default-sensor-index sensor-index]
                            [default-value-index value-index]))

;; define tabs
(require "tab.rkt")
(define tab-builder (get-tab-builder))

;; FIXME: multiple canvas configurator seem to interfere with one
;; another
(define (make-canvas-configurator fn)
  (new canvas-configurator% [parent sidebar]
       [config config]
       [default-sensor-index sensor-index]
       [default-value-index value-index]
       [on-update
        (λ (sensor-idx value-idx size min-value max-value)
          (fn sensor-idx value-idx size min-value max-value)
          (send plotter set-size size)
          (send plotter set-min-value min-value)
          (send plotter set-max-value max-value))]))

(define live-tab
  ((hash-ref tab-builder 'create)
   #:name "Live"
   #:children
   (let* ([canvas-configurator
           (make-canvas-configurator
            (λ (sensor-idx value-idx size min-value max-value)
              (send live-interface set-sensor sensor-idx)
              (send live-interface set-value value-idx)))])
     (list live-interface
           canvas-configurator))))

(define analyze-tab
  ((hash-ref tab-builder 'create)
   #:name "Review"
   #:children
   (let* ([canvas-configurator (make-canvas-configurator void)])
     (list canvas-configurator))))


;; make all tab widgets invisible
(define (clear-tab-panel)
  (let ([tabs ((hash-ref tab-builder 'get-tabs))]
        [make-invisible (λ (t)
                          (for ([widget (tab-children t)])
                            (send widget show #f)))])
    (for-each make-invisible tabs)))
(clear-tab-panel)


(define (set-tab-panel sel)
  (clear-tab-panel)
  (let* ([tabs ((hash-ref tab-builder 'get-tabs))]
         [current-tab (findf (λ (t) (= (tab-id t) sel)) tabs)])
    (if current-tab
        (begin
          (printf "Switched tab to ~s~n" (tab-name current-tab))
          (for ([widget (tab-children current-tab)])
            (send widget show #t)))
        (printf "WARNING: no tab with the id ~s~n" sel))))



;; add tabs as choices in tab-panel
(let ([tabs ((hash-ref tab-builder 'get-tabs))])
  (send sidebar set (map tab-name tabs)))

;; set default tab-panel
(let ([tabs ((hash-ref tab-builder 'get-tabs))])
  (unless (null? tabs) (set-tab-panel 0)))

(send frame show #t)
