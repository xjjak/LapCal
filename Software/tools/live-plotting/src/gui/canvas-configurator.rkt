#lang racket/gui

(provide canvas-configurator%)

(require "../configuration/configuration.rkt")

(define canvas-configurator%
  (class vertical-panel%
    (init parent
          config
          [(callback on-update)]
          [default-sensor-index 0]
          [default-value-index 0])
    (define on-update callback)
    (define sensor-index default-sensor-index)
    (define value-index default-value-index)
    (define int-size #f)
    (define int-min #f)
    (define int-max #f)
    
    (super-new [parent parent])


    (define (update-plotter-values-config)
      (let ([size (get-attribute config sensor-index value-index 'size)]
            [min (get-attribute config sensor-index value-index 'min)]
            [max (get-attribute config sensor-index value-index 'max)])
        (set! int-size size)
        (set! int-min min)
        (set! int-max max)
        (send size-text-field set-value (number->string size))
        (send min-text-field set-value (number->string min))
        (send max-text-field set-value (number->string max))
        (update-plotter-values #:min min #:max max #:size size)))

    (define (update-plotter-values #:min [min #f]
                                   #:max [max #f]
                                   #:size [size #f])
      (on-update sensor-index
                 value-index
                 (or size int-size)
                 (or min int-min)
                 (or max int-max)))

    ;; control plotting behaviour
    ; sensor
    (define sensor-selection
      (new choice% [parent this]
           [label "Sensor: "]
           [choices '("[0] Thumb"
                      "[1] Index"
                      "[2] Middle"
                      "[3] Ring"
                      "[4] Pinky"
                      "[5] Palm")]
           [selection sensor-index]
           [stretchable-width #t]
           [callback
            (λ (b c)
              (set! sensor-index (send b get-selection))
              (update-plotter-values-config))]))
    
    ; value
    (define value-selection
      (new choice% [parent this]
           [label "Value: "]
           [choices '("[0] Acceleration X"
                      "[1] Acceleration Y"
                      "[2] Acceleration Z"
                      "[3] Rotation X"
                      "[4] Rotation Y"
                      "[5] Rotation Z")]
           [selection value-index]
           [stretchable-width #t]
           [callback
            (λ (b c)
              (set! value-index (send b get-selection))
              (update-plotter-values-config))]))
    
    ;; control plotter behaviour
    (define max-text-field
      (new text-field% [parent this]
           [label "Max: "]
           [init-value
            (number->string
             (get-attribute config sensor-index value-index 'max))]
           [callback
            (λ (b c)
              (define max-value (string->number (send b get-value)))
              (when max-value
                (update-plotter-values #:max max-value)
                ;; (send plotter set-max-value max-value)
                ))]))
    
    (define min-text-field
      (new text-field% [parent this]
           [label "Min: "]
           [init-value
            (number->string
             (get-attribute config sensor-index value-index 'min))]
           [callback
            (λ (b c)
              (define min-value (string->number (send b get-value)))
              (when min-value
                (update-plotter-values #:min min-value)
                ;; (send plotter set-min-value min-value)
                ))]))
    
    (define size-text-field
      (new text-field% [parent this]
           [label "Plot size: "]
           [init-value (number->string
                        (get-attribute config sensor-index value-index 'size))]
           [callback
            (λ (b c)
              (define new-size (string->number (send b get-value)))
              (when new-size
                (update-plotter-values #:size new-size)
                ;; (send plotter set-size new-size)
                ))]))
    
    (update-plotter-values-config)))
