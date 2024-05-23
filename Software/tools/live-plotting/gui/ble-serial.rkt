#lang racket/gui

(provide ble-serial-controller%)

(define ble-serial-controller%
  (class vertical-panel%
    (init parent
          message-callback)
    (super-new [parent parent]
               [stretchable-height #f])

    (define msg message-callback)
    
    (define ble-devices empty)
    (define connection #f)

    (define/public (disconnect)
      (when connection
        (thread-send connection 'exit)
        (thread-wait connection)))

    (define ble-device-selector
      (new choice% [parent this]
           [choices ble-devices]
           [label "BLE Device: "]
           [stretchable-width #t]))

    (define scan-button
      (new button% [parent this]
           [label "Scan for BLE devices"]
           [stretchable-width #t]
           [callback
            (λ (b c)
              (thread
               (λ ()
                 (send b enable #f)
                 (send ble-device-selector enable #f)
                 (send ble-device-selector clear)
                 (set! ble-devices empty)
                 
                 (let ([lines (filter
                               (λ (line) (string-contains? line "rssi="))
                               (port->lines (first (process "ble-scan"))))])
                   (for-each (λ (line) (send ble-device-selector append line))
                             lines)
                   (set! ble-devices
                         (map (λ (line) (first (string-split line)))
                              lines)))
                 
                 (send b enable #t)
                 (send ble-device-selector enable #t))))]))

    (define connect-button
      (new button% [parent this]
           [label "Connect"]
           [stretchable-width #t]
           [callback
            (λ (b c)
              (define ble-controls (list ble-device-selector
                                         scan-button))
              (define (reset-connection)
                (map (λ (control) (send control enable #t)) ble-controls)
                (send b set-label "Connect")
                (set! connection #f))
              
              (if connection
                  (begin
                    (send b enable #f)
                    (thread-send connection 'exit)
                    (reset-connection))
                  (begin
                    (map (λ (control) (send control enable #f)) ble-controls)
                    (send b set-label "Disconnect")

                    (set!
                     connection
                     (thread
                      (λ ()
                        (define selected-device (send ble-device-selector get-selection))
                        (match-define
                          (list in out pid err handle)
                          (process/ports
                           (open-output-file "/dev/stdout" #:exists 'append) #f 'stdout
                           (format "ble-serial -d \"~a\"" ; "ble-serial -d 08:3A:8D:0F:35:72"
                                   (list-ref ble-devices selected-device))))
                        (let loop ()
                          (match (thread-try-receive)
                            ['exit
                             (handle 'interrupt)
                             (sleep 0.5)
                             (handle 'interrupt)
                             (msg "Connection is terminating...")
                             (let still-running ()
                               (when (eq? (handle 'status) 'running)
                                 (sleep 0.1)
                                 (still-running)))
                             (send b enable #t)
                             (msg "Connection is terminated.")]
                            [_
                             (if (eq? (handle 'status) 'running)
                                 (begin
                                   (sleep 0.1)
                                   (loop))
                                 (msg "Could not maintain the connection."))]))
                        (reset-connection)))))))]))))
