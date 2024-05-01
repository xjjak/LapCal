;;
;; -*- eval: (setenv "LD_LIBRARY_PATH" "/home/palisn/.nix-profile/lib") -*-
#lang racket/gui

;; TODO: add to config when changing text fields

(require plot)
(require "plot-canvas.rkt")
(require "ble-serial.rkt")
(require "parse.rkt")
(require "volatile-message.rkt")
(require "configuration.rkt")

(define default-selection '(0 0))
(define default-size 100)

(define config-file "config.json")
(define config
  (if (file-exists? config-file)
      (load-config config-file)
      (let ([cfg (generate-default-config)])
        (write-config cfg config-file)
        cfg)))
(print config)

(define sensor-index 0)
(define value-index 0)

;; TODO: update text-fields
(define (update-plotter-values)
  (let ([size (get-attribute config sensor-index value-index 'size)]
        [min (get-attribute config sensor-index value-index 'min)]
        [max (get-attribute config sensor-index value-index 'max)])
    (send plotter set-size      size)
    (send size-text-field set-value (number->string size))
    (send plotter set-min-value min)
    (send min-text-field set-value (number->string min))
    (send plotter set-max-value max)
    (send max-text-field set-value (number->string max))))


(define frame
  (new
   (class frame%
     (init)
     (super-new [label "float"]
                [width 900]
                [height 600])

     (define/augment (on-close)
       (send ble-controller disconnect)))))

(define container
  (new horizontal-panel% [parent frame]))

(define plotter
  (new plot-canvas% [parent container]
       [size
        (get-attribute config sensor-index value-index 'size)]
       [min-value
        (get-attribute config sensor-index value-index 'min)]
       [max-value
        (get-attribute config sensor-index value-index 'max)]))

(define sidebar
  (new vertical-panel% [parent container]
       [stretchable-width #f]
       [horiz-margin 5]
       [min-width 200]
       [alignment '(left top)]))


(define ble-serial-port "/tmp/ttyBLE")

(define ble-controller
  (new ble-serial-controller% [parent sidebar]
       [message-callback
        (λ (msg)
          (send feedback-msg show-message msg))]))


;; maintain plotting in different thread
(define plotting-thread #f)

(define plot-button
  (new button% [parent sidebar]
       [label "Start plotting"]
       [stretchable-width #t]
       [vert-margin 25]
       [callback
        (λ (b c)
          (if plotting-thread
              (begin
                (thread-send plotting-thread 'stop)
                (send b set-label "Start plotting")
                (send ble-controller enable #t)
                (set! plotting-thread #f))
              (if (file-exists? ble-serial-port)
                  (begin
                    (send ble-controller enable #f)
                    (send b set-label "Stop plotting")
                    (set! plotting-thread
                          (thread
                           (thunk
                             (define in (open-input-file "/tmp/ttyBLE"))
                             (define testing-interval 50)

                             (let loop ([i testing-interval])
                               (let* ([str (read-line in)]
                                      [data (parse str)])
                                 (let* ([row (if (vector? data) (vector-ref data sensor-index) #f)]
                                        [value (if (vector? row) (vector-ref row value-index) #f)])
                                   (if value
                                       (send plotter push value)
                                       (send feedback-msg show-message
                                             "Parsing error: Check sensor connection"))))
                               (unless (and (= i 0) (eq? (thread-try-receive) 'stop))
                                 (loop (modulo (sub1 i) testing-interval))))
                             (close-input-port in)))))
                  (send feedback-msg show-message (format "Serial port (~a) not found." ble-serial-port)))))]))


;; control plotting behaviour
; sensor
(define sensor-selection
  (new choice% [parent sidebar]
       [label "Sensor: "]
       [choices '("[0] Thumb"
                  "[1] Index"
                  "[2] Middle"
                  "[3] Ring"
                  "[4] Pinky"
                  "[5] Palm")]
       [stretchable-width #t]
       [callback
        (λ (b c)
          (set! sensor-index (send b get-selection))
          (update-plotter-values))]))

; value
(define value-selection
  (new choice% [parent sidebar]
       [label "Value: "]
       [choices '("[0] Acceleration X"
                  "[1] Acceleration Y"
                  "[2] Acceleration Z"
                  "[3] Rotation X"
                  "[4] Rotation Y"
                  "[5] Rotation Z")]
       [stretchable-width #t]
       [callback
        (λ (b c)
          (set! value-index (send b get-selection))
          (update-plotter-values))]))

;; control plotter behaviour
(define max-text-field
  (new text-field% [parent sidebar]
       [label "Max: "]
       [init-value
        (number->string
         (get-attribute config sensor-index value-index 'max))]
       [callback
        (λ (b c)
          (define max-value (string->number (send b get-value)))
          (when max-value
            (send plotter set-max-value max-value)))]))

(define min-text-field
  (new text-field% [parent sidebar]
       [label "Min: "]
       [init-value 
        (number->string
         (get-attribute config sensor-index value-index 'min))]
       [callback
        (λ (b c)
          (define min-value (string->number (send b get-value)))
          (when min-value
            (send plotter set-min-value min-value)))]))

(define size-text-field
  (new text-field% [parent sidebar]
       [label "Plot size: "]
       [init-value (number->string default-size)]
       [callback
        (λ (b c)
          (define new-size (string->number (send b get-value)))
          (when new-size
            (send plotter set-size new-size)))]))



(define feedback-msg
  (new volatile-message% [parent sidebar]
       [color "red"]))

(cond
  [(file-exists? ble-serial-port)
   (send ble-controller enable #f)
   (send feedback-msg set-label
         "Port busy.\n Connection probably already established.")]
  [(not (and (system "command -v ble-scan")
             (system "command -v ble-serial")))
   (send ble-controller enable #f)
   (send feedback-msg set-label
         "Can't find ble-scan and ble-serial.\nPlease setup the port yourself.")]
  [else
   (void)])

(application-quit-handler (thunk (display "test")))

(send frame show #t)
