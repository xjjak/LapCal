#lang racket/gui

(provide dataset-picker%)

(require framework
         "volatile-message.rkt")

(define dataset-picker%
  (class vertical-panel%
    (init parent
          callback)

    (define on-change callback)

    (super-new [parent parent]
               [stretchable-height #f])

    (define dataset-file #f)

    (define/public (set-dataset-path path)
      (cond
        [(path? path)
         (on-change (path->string path))]
        [(string? path)
         (on-change path)]
        [else
         ; TODO: logging?         
         (send feedback-msg show-message "Invalid path")
         #f]))

    (define load-button
      (new button% [parent this]
           [label "Load Dataset"]
           [callback (λ (b e)
                       (let ([ext (finder:default-extension)])
                         (finder:default-extension "txt")
                         (set-dataset-path (finder:get-file)) ;; TODO: issue warning
                         (finder:default-extension ext)))]))

    (define feedback-msg
      (new volatile-message% [parent this]
           [color "red"]))))
