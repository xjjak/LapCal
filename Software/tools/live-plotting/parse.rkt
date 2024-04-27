#lang racket

(provide parse)

(require racket/string)

(define (parse str)
  (cond
    [(eof-object? str)
     #f]
    [else
     (let* ([str-trimmed (string-trim str)]
            [sensors (string-split str-trimmed ":" #:trim? #f)])
       (if (= (length sensors) 7)
           (for/vector ([sensor-str (rest sensors)])
             (let ([values (string-split sensor-str ";" #:trim? #f)])
               (vector-map string->number (list->vector values))))
           #f))]))
