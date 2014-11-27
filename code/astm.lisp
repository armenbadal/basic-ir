;;;;
;;;; Structures for Abstract Syntax Tree (AST).
;;;;
;;;;

(defstruct ast-module name subs)
(defstruct ast-function name args type body)
(defstruct ast-variable name type)
(defstruct ast-constant value type)
(defstruct ast-unary oper expr)
(defstruct ast-type-cast expr from to)
(defstruct ast-binary oper type expro expri)
(defstruct ast-func-call name args)
(defstruct ast-sequence sto sti)
(defstruct ast-declare name type)
(defstruct ast-result exp)
(defstruct ast-assign name expr)
(defstruct ast-branch cond thenp elsep)
(defstruct ast-for-loop param start stop step body)
(defstruct ast-while-loop cond body)
(defstruct ast-input vars)
(defstruct ast-print vals)

