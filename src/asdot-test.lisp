;(load "asdot.lisp")

;(princ "----------------------------")(terpri)

;(load "../cases/case18.lisp")
;(princ "----------------------------")

;(print 
;    (basic-binary "AND" (basic-boolean "TRUE") (basic-boolean "FALSE")))




(defparameter *node-index* 0)
(defmacro new-index ()
  `(incf *node-index*))

(defmacro ast-node (num label)
  `(format t "  astnode_~a[label=\"~a\"];~%" ,num ,label))
(defmacro connect-nodes (no ni)
  `(format t "  astnode_~a -> astnode_~a;~%" ,no ,ni))




(defmacro define-converter (name &rest params)
    `(defun ,name (,@params)
        (print (list ,@params))
    ))

;(define-converter basic-boolean vali)
;
;(basic-boolean "TRUE")
;(basic-boolean "FALSE")
;
;(define-converter basic-unary op expr)
;
;(basic-unary "+" (basic-boolean "TRUE"))

(defmacro define-converter-2 (name params &rest body)
    `(defun ,name ,params
        (let ((index (new-index)))
            ,@body
            index)))

(define-converter-2 my-converter (pa pb pc)
    (print pa)
    (print pb)
    (print pc))

(print (my-converter 1 22 33))
(print (my-converter 4 55 66))




(terpri)(quit)
