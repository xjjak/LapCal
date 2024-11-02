#lang racket/gui

;; gui controller to find and connect to bluetooth devices

(provide
 ;; gui controller to find and connect to bluetooth devices
 ble-serial-controller%)

;; ==============
;; implementation

(define ble-serial-controller%
  (class vertical-panel%
    (init parent
          ;; callback for user feedback
          [message-callback displayln]
          ;; function that returns available bluetooth devices
          [scan-function ble-serial-scan])
    
    (super-new [parent parent]
               [stretchable-height #f])

    ;; ----------------
    ;; public interface

    ;; halt the current connection
    (define/public (disconnect)
      (when connection-thread
        (thread-send connection-thread 'exit)
        (thread-wait connection-thread)))


    ;; ------------
    ;; gui elements
    
    (define ble-device-selector
      (new choice% [parent this]
           [choices empty]
           [label "BLE Device: "]
           [stretchable-width #t]))

    (define scan-button
      (new button% [parent this]
           [label "Scan for BLE devices"]
           [stretchable-width #t]
           [callback
            (λ _ (thread scan-for-devices))]))

    (define connect-button
      (new button% [parent this]
           [label "Connect"]
           [stretchable-width #t]
           [callback
            (λ _
              (if connection-thread
                  (ble-disconnect)
                  (set! connection-thread (thread ble-connect))))]))

    
    ;; ---------------
    ;; class variables
    
    ;; function to report to the user
    (define msg message-callback)

    ;; class wide list of ble device mac addresses
    (define mac-ble-devices empty)

    ;; thread monitoring an active connection
    (define connection-thread #f)

    ;; all ui elements that should be inaccessible when a ble device
    ;; is connected
    (define ble-search-controls (list ble-device-selector
                                      scan-button))


    ;; --------------------------
    ;; connection thread handling

    (define (connection-thread-event-handler handle)
      (let loop ()
        (match (thread-try-receive)
          ['exit
           ;; send SIGINT to let process gracefully terminate
           (handle 'interrupt)
           (sleep 0.5)
           ;; NOTE: ble-serial requires two SIGINT signals to actually
           ;; terminate
           (handle 'interrupt)
           (msg "Connection is terminating...")
           (handle 'wait)
           (send connect-button enable #t)
           (msg "Connection is terminated.")]
          [_
           ;; periodically check if process is still running
           (if (eq? (handle 'status) 'running)
               (begin
                 (sleep 0.1)
                 (loop))
               (msg "Could not maintain the connection."))]))
      ;; cleanup connection after thread death
      (ble-disconnect))


    ;; ------------------
    ;; scanning utilities
    
    (define (scan-for-devices)
      (send scan-button enable #f)
      (send ble-device-selector enable #f)
      (send ble-device-selector clear)
      (set! mac-ble-devices
            (for/list ([device (scan-function)])
              (send ble-device-selector append (car device))
              (cdr device)))
      (send ble-device-selector enable #t)
      (send scan-button enable #t))

    
    ;; ------------------------
    ;; ble connection utilities
    
    (define (ble-disconnect)
      (when (thread? connection-thread)
        (send connect-button enable #f)
        (thread-send connection-thread 'exit))
      (for ([blectl ble-search-controls]) (send blectl enable #t))
      (send connect-button set-label "Connect")
      (set! connection-thread #f))

    (define (ble-connect)
      (for ([blectl ble-search-controls]) (send blectl enable #f))
      (send connect-button set-label "Disconnect")
      (define selected-device (send ble-device-selector get-selection))
      (if (null? selected-device)
          (msg "No device selected.")
          (match-let* ([stdout (open-output-file "/dev/stdout"
                                                 #:exists 'append)]
                       [mac (list-ref mac-ble-devices selected-device)]
                       [cmd (format "ble-serial -d \"~a\"" mac)]
                       [(list _ _ _ _ handle)
                        (process/ports stdout #f 'stdout cmd)])
            (connection-thread-event-handler handle))))))

;; --------------
;; scan functions

(define (ble-serial-scan)
  (match-let ([(list stdout _ _ _ handle) (process "ble-scan")])
    (sleep 0.1)
    (cond
      [(eq? (handle 'status) 'running)
         (for/list ([line (port->lines stdout)]
                    #:when (string-contains? line "rssi="))
            (let ([desc line]
                  [mac (first (string-split line))])
              (cons desc mac)))]
      [else #f])))

