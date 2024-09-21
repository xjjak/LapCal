#lang racket/gui

(provide gui-tests)

(require rackunit
         rackunit/text-ui
         "../src/gui/dataset-picker.rkt")

(define gui-tests
  (let ([frame #f])
    (test-suite
     "Test for gui classes"
     #:before (thunk (set! frame (new frame% [label "Test"])))
     #:after (thunk (set! frame #f))
     (test-case
         "Tests for frame"
       (check string=? (send frame get-label) "Test" "Check frame label"))
     
     (test-suite
         "Path parsing and callback"
       (let* ([path-input "/home/user/dataset.txt"]
              [path-output #f]
              [picker (new dataset-picker% [parent frame]
                           [callback (λ (path)
                                       (set! path-output path))])])
         (send picker set-dataset-path #f)
         (test-case "Input false" (check-false path-output))
         (send picker set-dataset-path (string->path path-input))
         (test-case "Input path" (check string=? path-input path-output))
         ; TODO: checking for same input content could lead to
         ; incorrect test results
         (send picker set-dataset-path path-input)
         (test-case "Input path-string" (check string=? path-input path-output))))

     )))

(run-tests gui-tests 'verbose)

;; (require rackunit/gui)
;; (test/gui gui-tests)
