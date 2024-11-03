#lang racket/gui

;; gui controller to find and connect to bluetooth devices

(provide
 ;; gui controller to find and connect to bluetooth devices
 ble-serial-controller%
 ble-receiver-controller%)

;; ==============
;; implementation

;; ble controller classes

(define (make-ble-controller-class ble-connection%)
  (class vertical-panel%
    (init parent
          ;; function that returns available bluetooth devices
          [scan-function ble-serial-scan]
          ;; callback for user feedback
          [message-callback (λ (str . _) (displayln str))])
    
    (super-new [parent parent]
               [stretchable-height #f])

    ;; ----------------
    ;; public interface

    ;; halt the current connection
    (define/public (disconnect)
      (when (and (thread? connection-thread)
                 (thread-running? connection-thread))
        (thread-send connection-thread 'exit)
        (thread-wait connection-thread)))

    ;; return input-port to input coming from bluetooth device
    (define/public (get-ble-input-port)
      (send ble-connection get-input-port))

    
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
              (if (and (thread? connection-thread)
                       (thread-running? connection-thread))
                  (ble-disconnect)
                  (set! connection-thread (thread ble-connect))))]))

    
    ;; ---------------
    ;; class variables
    
    ;; function to report to the user
    (define msg message-callback)

    ;; class wide list of ble device mac addresses
    (define mac-ble-devices empty)

    ;; active bluetooth connection
    (define ble-connection #f)

    ;; thread monitoring an active connection
    (define connection-thread #f)

    ;; all ui elements that should be inaccessible when a ble device
    ;; is connected
    (define ble-search-controls (list ble-device-selector
                                      scan-button))


    ;; --------------------------
    ;; connection thread handling

    ;; contiguously check if program is still runnning, handle
    ;; incoming signals, and cleanup on termination
    (define (connection-thread-event-handler blecon)
      (let loop ()
        (match (thread-try-receive)
          ['exit
           (msg "Terminating connection..."
                #:color "white")
           (send blecon disconnect)
           (send connect-button enable #t)
           (msg "Connection terminated."
                #:color "green")]
          [_
           ;; periodically check if process is still running
           (if (send blecon active?)
               (begin
                 (sleep 0.1)
                 (loop))
               (msg "Could not maintain the connection."))]))
      ;; cleanup connection after thread death
      (ble-disconnect))


    ;; ------------------
    ;; scanning utilities
    
    ;; use scan-function to update the known bluetooth devices
    (define (scan-for-devices)
      (for ([blectl ble-search-controls]) (send blectl enable #f))
      (send ble-device-selector clear)
      (set! mac-ble-devices
            (for/list ([device (scan-function)])
              (send ble-device-selector append (car device))
              (cdr device)))
      (for ([blectl ble-search-controls]) (send blectl enable #t)))

    
    ;; ------------------------
    ;; ble connection utilities
    
    ;; terminate active bluetooth thread and cleanup
    (define (ble-disconnect)
      (when (thread? connection-thread)
        (send connect-button enable #f)
        (thread-send connection-thread 'exit))
      (for ([blectl ble-search-controls]) (send blectl enable #t))
      (send connect-button set-label "Connect")
      (send connect-button enable #t)
      (set! connection-thread #f))

    ;; establish and maintain bluetooth connection with selected
    ;; device
    (define (ble-connect)
      (define selected-device (send ble-device-selector get-selection))
      (cond
        [selected-device
         (for ([blectl ble-search-controls]) (send blectl enable #f))
         (send connect-button set-label "Disconnect")
         (set! ble-connection
               (new ble-connection%
                    [mac (list-ref mac-ble-devices selected-device)]))
         (connection-thread-event-handler ble-connection)]
        [else
         (msg "No device selected.")]))))


;; --------------
;; scan functions

;; perform bluetooth scan using `ble-scan` program
(define (ble-serial-scan)
  (cond
    [(system "command -v ble-scan") ;; check if ble-scan is available
     (match-let ([(list stdout _ _ _ _) (process "ble-scan")])
       (sleep 0.1)
       (for/list ([line (port->lines stdout)]
                  #:when (string-contains? line "rssi="))
         (let ([desc line]
               [mac (first (string-split line))])
           (cons desc mac))))]
    [else #f]))


;; -----------------------
;; ble connection handling

;; TODO: desc
(define ble-connection<%>
  (interface () get-input-port disconnect active?))

;; TODO: desc
(define ble-process-connection%
  (class* object% (ble-connection<%>) (super-new)
    (init mac
          ;; command that starts the process; must include `~a` where
          ;; the mac address goes
          cmd
          [stdout #f])

    ;; ----------------
    ;; public interface

    ;; return input-port of bluetooth device transmissions
    (define/public (get-input-port)
      (dup-input-port input-port))

    ;; disconnect bluetooth device and clean up
    (define/pubment (disconnect)
      ;; send SIGINT to let process gracefully terminate
      (process-handle 'interrupt)
      (sleep 0.5)
      ;; NOTE: some programs, like ble-serial, requires two SIGINT
      ;; signals to actually terminate
      (process-handle 'interrupt)
      (process-handle 'wait)
      (when (input-port? input-port)
        (close-input-port input-port))
      (inner (void) disconnect))

    ;; return whether the connection is still active
    (define/pubment (active?)
      (and (eq? (process-handle 'status) 'running)
           (inner #t active?)))

    
    ;; ---------------
    ;; class variables

    ;; the input-port from the connected bluetooth device
    (define input-port #f)

    ;; process handle
    (define process-handle #f)
    

    ;; --------------
    ;; initialization
    
    (match-let* ([cmd (format cmd mac)]
                 [(list port _ _ _ handle)
                  (process/ports stdout #f 'stdout cmd)])
      (set! input-port port)
      (set! process-handle handle))))

;; TODO: come up with a way to actually use these arguments without the need for inheritance?

;; TODO: error handling: is ble-serial available? etc.

;; TODO: desc
(define ble-serial-connection%
  (class ble-process-connection%
    (init mac
          [(f port-file) "/tmp/ttyBLE"]
          [stdout (open-output-file "/dev/stdout" #:exists 'append)]
          [cmd-args ""])

    (super-new [mac mac]
               [cmd (format "ble-serial -d \"~~a\" -p ~a ~a" f cmd-args)]
               [stdout stdout])

    ;; ----------------
    ;; public interface

    (define/augment (disconnect)
      (when (input-port? input-port)
        (close-input-port input-port)))

    (define/override (get-input-port)
      (unless input-port
        (set! input-port (open-input-file port-file)))
      (dup-input-port input-port))
    
    ;; ---------------
    ;; class variables

    ;; rename init argument
    (define port-file f)
    
    ;; input-port to port file used by ble-serial
    (define input-port #f)))

;; TODO: desc
(define ble-receiver-connection%
  (class ble-process-connection%
    (init mac
          [cmd-args ""])

    (super-new [mac mac]
               ;; TODO: generalize command
               [cmd (format "python ble-receiver.py -d \"~~a\" ~a" cmd-args)])))


;; / ble controller classes
;; TODO: figure out how to bring them to the top again

(define ble-serial-controller%
  (make-ble-controller-class ble-serial-connection%))

(define ble-receiver-controller%
  (make-ble-controller-class ble-receiver-connection%))


