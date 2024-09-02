#lang racket/base

(provide (struct-out tab)
         get-tab-builder)

(struct tab (id name children))

(define (get-tab-builder)
  (let ([id 0]
        [tabs '()])
    (hash
     'create (λ (#:name [name #f] #:children [children null])
               (let ([new-tab (tab id (or name (number->string id)) children)])
                 (set! tabs (append tabs (list new-tab)))
                 (set! id (add1 id))
                 new-tab))
     'get-tabs (λ ()
                 tabs))))
