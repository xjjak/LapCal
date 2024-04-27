;; -*- eval: (setenv "LD_LIBRARY_PATH" "/home/palisn/.nix-profile/lib") -*-
#lang racket/gui

(require plot)
(require "plot-canvas.rkt")
(require "ble-serial.rkt")
(require "parse.rkt")
(require "volatile-message.rkt")

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

(define default-min -10000)
(define default-max 10000)
(define default-size 100)
(define plotter
  (new plot-canvas% [parent container]
       [size 100]
       [min-value default-min]
       [max-value default-max]))

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
                                 (let* ([row (if (vector? data) (vector-ref data finger-index) #f)]
                                        [value (if (vector? row) (vector-ref row sensor-index) #f)])
                                   (if value
                                       (send plotter push value)
                                       (send feedback-msg show-message
                                             "Parsing error: Check sensor connection"))))
                               (unless (and (= i 0) (eq? (thread-try-receive) 'stop))
                                 (loop (modulo (sub1 i) testing-interval))))
                             (close-input-port in)))))
                  (send feedback-msg show-message (format "Serial port (~a) not found." ble-serial-port)))))]))


;; control plotting behaviour
(define finger-index 0)
(define sensor-index 0)
; finger
(new choice% [parent sidebar]
     [label "Finger: "]
     [choices '("[0] Thumb"
                "[1] Index"
                "[2] Middle"
                "[3] Ring"
                "[4] Pinky"
                "[5] Palm")]
     [stretchable-width #t]
     [callback
      (λ (b c)
        (set! finger-index (send b get-selection)))])

; sensor
(new choice% [parent sidebar]
     [label "Sensor: "]
     [choices '("[0] Acceleration X"
                "[1] Acceleration Y"
                "[2] Acceleration Z"
                "[3] Rotation X"
                "[4] Rotation Y"
                "[5] Rotation Z")]
     [stretchable-width #t]
     [callback
      (λ (b c)
        (set! sensor-index (send b get-selection)))])

;; control plotter behaviour
(new text-field% [parent sidebar]
     [label "Max: "]
     [init-value (number->string default-max)]
     [callback
      (λ (b c)
        (define max-value (string->number (send b get-value)))
        (when max-value
          (send plotter set-max-value max-value)))])

(new text-field% [parent sidebar]
     [label "Min: "]
     [init-value (number->string default-min)]
     [callback
      (λ (b c)
        (define min-value (string->number (send b get-value)))
        (when min-value
          (send plotter set-min-value min-value)))])

(new text-field% [parent sidebar]
     [label "Plot size: "]
     [init-value (number->string default-size)]
     [callback
      (λ (b c)
        (define new-size (string->number (send b get-value)))
        (when new-size
          (send plotter set-size new-size)))])



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
