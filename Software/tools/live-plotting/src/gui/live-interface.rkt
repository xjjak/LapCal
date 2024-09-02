#lang racket/gui

(provide live-interface%)

(require "../processing/parse.rkt")

(require "ble-serial.rkt")
(require "volatile-message.rkt")

;; TODO: extract "view size editor" to separate component

(define live-interface%
  (class vertical-panel%
    (init parent
          [(callback on-input)]
          [default-sensor-index 0]
          [default-value-index 0])
    (define on-input callback)
    (define sensor-index default-sensor-index)
    (define value-index default-value-index)

    (define/public (set-sensor sensor)
      (set! sensor-index sensor))

    (define/public (set-value value)
      (set! value-index value))

    (super-new [parent parent]
               [horiz-margin 5]
               [min-width 200]
               [alignment '(left top)])
    
    (define ble-controller
      (new ble-serial-controller% [parent this]
           [message-callback
            (λ (msg)
              (send feedback-msg show-message msg))]))
    
    (define/public (disconnect)
      (send ble-controller disconnect))

    (define ble-serial-port "/tmp/ttyBLE")
    
    ;; maintain plotting in different thread
    (define plotting-thread #f)
    
    (define plot-button
      (new button% [parent this]
           [label "Start plotting"]
           [stretchable-width #t]
           [vert-margin 25]
           [callback
            (λ (b c)
              (if plotting-thread
                  (begin
                    (when (thread-running? plotting-thread)
                      (thread-send plotting-thread 'stop))
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
                                         [data (parse str)]
                                         [row (if (vector? data) (vector-ref data sensor-index) #f)]
                                         [value (if (vector? row) (vector-ref row value-index) #f)])
                                    (if value
                                        (on-input value)
                                        ;; (send plotter push value)
                                        (send feedback-msg show-message
                                              "Parsing error: Check sensor connection")))
                                  (unless (and (= i 0) (eq? (thread-try-receive) 'stop))
                                    (loop (modulo (sub1 i) testing-interval))))
                                (close-input-port in)))))
                      (send feedback-msg show-message (format "Serial port (~a) not found." ble-serial-port)))))]))
    
    
    ;; (define (update-plotter-values)
    ;;   (let ([size (get-attribute config sensor-index value-index 'size)]
    ;;         [min (get-attribute config sensor-index value-index 'min)]
    ;;         [max (get-attribute config sensor-index value-index 'max)])
    ;;     (send plotter set-size size)
    ;;     (send plotter set-min-value min)
    ;;     (send plotter set-max-value max)
    ;;     (send size-text-field set-value (number->string size))
    ;;     (send min-text-field set-value (number->string min))
    ;;     (send max-text-field set-value (number->string max))))
    
    
    (define feedback-msg
      (new volatile-message% [parent this]
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
       (void)])))
