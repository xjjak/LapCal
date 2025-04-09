#lang racket/gui

(provide volatile-message%)

(define volatile-message%
  (class message%
    (init parent
          [color #f]
          [message ""]
          [message-time 5])
    (super-new [parent parent]
               [label ""]
               [color color])

    (inherit get-color
             set-color
             set-label)
    
    (define default-message message)
    (define default-color (get-color))
    (define time-to-live message-time)
    (define message-thread #f)

    (define/public (show-message msg [ttl #f] [color #f])
      (when message-thread
        (kill-thread message-thread))
      
      (set! message-thread
            (thread
             (thunk
              (when color (set-color color))
              (set-label msg)

              (sleep (or ttl time-to-live))

              (set-color default-color)
              (set-label default-message)))))))
