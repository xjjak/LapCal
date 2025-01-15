#lang racket

(provide generate-default-config
         get-attribute
         load-config
         write-config)

(require json)

(define (generate-default-config)
  ;; (list/c (hash/c (or/c 'sensors 'values 'min 'max 'size)
  ;;                 (or/c (or/c (integer-in 0 5) string?
  ;;                             (list/c (or/c (integer-in 0 5) string?)))
  ;;                       integer?)))
  '(#hasheq((sensor . "all")
            (value  . ("acc-x" "acc-y" "acc-z"))
            (min    . -10000)
            (max    . 10000))
    #hasheq((sensor . "all")
            (value  . ("rot-x" "rot-y" "rot-z"))
            (min    . -360)
            (max    . 360))
    #hasheq((sensor . "all")
            (value  . "all")
            (size   . 100))))

(define (get-attribute config sensor value attr)
  (cond
    [(null? config) (error "No default value configured in the configuration." sensor value attr)]
    [(and (config-match? (car config) sensor value)
          (hash-has-key? (car config) attr))
     (hash-ref (car config) attr)]
    [else
     (get-attribute (cdr config) sensor value attr)]))

(define (config-match? hm sensor value)
  (define in-sensor
    (cond
      [(hash-has-key? hm 'sensor)
       (let ([hm-sensor (hash-ref hm 'sensor)])
         (cond
           [(list? hm-sensor)
            (member sensor (map translate-sensor-term hm-sensor))]
           [(string? hm-sensor)
            (or (equal? hm-sensor "all")
                (equal? sensor (translate-sensor-term hm-sensor)))]
           [((integer-in 0 5) hm-sensor)
            (= sensor hm-sensor)]
           [else (error "Unknown value for sensor" hm-sensor)]))]
      [else #t]))
  (define in-value
    (cond
      [(hash-has-key? hm 'value)
       (let ([hm-value (hash-ref hm 'value)])
         (cond
           [(list? hm-value)
            (member value (map translate-value-term hm-value))]
           [(string? hm-value)
            (or (equal? hm-value "all")
                (equal? value (translate-value-term hm-value)))]
           [((integer-in 0 5) hm-value)
            (= value hm-value)]
           [else (error "Unknown value for value" hm-value)]))]
      [else #t]))
  (and in-sensor in-value))

(define sensor-terms (list "thumb" "index" "middle"
                           "ring" "pinky" "palm"))
(define (translate-sensor-term term)
  (index-where sensor-terms (λ (st) (string-prefix? st term))))

(define value-terms (list "acc-x" "acc-y" "acc-z"
                           "rot-x" "rot-y" "rot-z"))
(define (translate-value-term term)
  (index-where value-terms (λ (st) (string-prefix? st term))))


(define (load-config path)
  (let* ([in (open-input-file path)]
         [cfg (read-json in)])
    (close-input-port in)
    cfg))

(define (write-config cfg path)
  (let ([out (open-output-file path
                               #:exists 'replace
                               #:replace-permissions? #t)])
    (write-json cfg out)
    (close-output-port out)))
