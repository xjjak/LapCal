#lang racket/gui

;; coordinate plotting canvas and live data source

(provide
 ;; coordinate plotting canvas and live data source
 live-interface%)

;; ==============
;; implementation

(require "../processing/parse.rkt"
         "ble-serial.rkt"
         "volatile-message.rkt")

;; TODO: extract "view size editor" to separate component

(define live-interface%
  (class vertical-panel%
    (init parent
          ;; class that implements the ble-controller<%> interface (see ble-serial.rkt)
          ble-controller%
          ;; callback on new input from glove
          [(callback on-input)]
          ;; default indices for plot source
          [default-sensor-index 0]
          [default-value-index 0])
    
    (super-new [parent parent]
               [horiz-margin 5]
               [min-width 200]
               [alignment '(left top)])
    
    ;; ----------------
    ;; public interface

    ;; set current sensor index to given index
    (define/public (set-sensor sensor)
      (set! sensor-index sensor))

    ;; set current value index to given index
    (define/public (set-value value)
      (set! value-index value))

    ;; disconnect the bluetooth controller
    (define/public (disconnect)
      (send ble-controller disconnect))


    ;; ---------------
    ;; class variables

    ;; bind callback
    (define on-input callback)

    ;; currently active sensor and value index
    (define sensor-index default-sensor-index)
    (define value-index default-value-index)

    ;; thread for plotting incoming data
    (define plotting-thread #f)


    ;; ----------------
    ;; helper functions
    
    (define (msg str #:ttl [ttl #f] #:color [color #f])
      (send feedback-msg show-message str ttl color))

    
    ;; ------------
    ;; gui elements
    
    ;; controls bluetooth connection
    (define ble-controller
      (new ble-controller% [parent this]
           [message-callback msg]))

    ;; button to toggle plotting
    (define plot-button
      (new button% [parent this]
           [label "Start plotting"]
           [stretchable-width #t]
           [vert-margin 25]
           [callback
            (λ _
              (if plotting-thread
                  (stop-plotting)
                  (start-plotting)))]))

    ;; text box for user feedback
    (define feedback-msg
      (new volatile-message% [parent this]
           [color "red"]))


    ;; -------------------------
    ;; plotting thread utilities

    ;; start plotting thread and adjust gui
    (define (start-plotting)
      (send ble-controller enable #f)
      (send plot-button set-label "Stop plotting")
      (set! plotting-thread (thread thread-fn)))

    ;; terminate plotting thread and adjust gui
    (define (stop-plotting)
      (when (and (thread? plotting-thread)
                 (thread-running? plotting-thread))
        (thread-send plotting-thread 'stop))
      (send plot-button set-label "Start plotting")
      (send ble-controller enable #t)
      (set! plotting-thread #f))

    ;; repeatedly get relevant value from data source and pass it to
    ;; callback until thread receives `'stop` signal
    (define (plotting-loop get-input)
      (define testing-interval 50)
      (let loop ([i testing-interval])
        (let* ([str (get-input)]
               [data (parse str)]
               [row (if (vector? data) (vector-ref data sensor-index) #f)]
               [value (if (vector? row) (vector-ref row value-index) #f)])
          (if value
              (on-input value)
              (msg "Parsing error: Check sensor connection")))
        (unless (and (= i 0) (eq? (thread-try-receive) 'stop))
          (loop (modulo (sub1 i) testing-interval)))))

    
    ;; --------------
    ;; initialization

    ;; setup thread function
    (define thread-fn
      (ble-serial-input #:processing-loop plotting-loop
                        #:ble-controller ble-controller
                        #:error (λ (str #:lock lock)
                                  (when lock
                                    (send ble-controller enable #f))
                                  (msg str #:ttl #f))))))

;; --------------------------
;; thread function generators

;; a collection of functions that generate thread functions required
;; for handling different data sources

;; generate thread function that uses the ble-serial data source
(define (ble-serial-input #:processing-loop processing-loop
                          #:ble-controller ble-controller
                          #:error [error void])
  ;; return thread function
  (λ ()
    (define input-port (send ble-controller get-ble-input-port))
    (cond
      [(and (input-port? input-port)
            (not (port-closed? input-port)))
       (processing-loop (port-reader input-port))
       (close-input-port input-port)]
      [else
       (error (format "Serial port (~a) not found." input-port)
              #:lock #f)])))

;; ----------------
;; helper functions

;; generate function that reads a line from given port when called
(define (port-reader port)
  (let ([in port])
    (thunk (read-line in))))


