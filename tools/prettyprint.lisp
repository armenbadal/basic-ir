(let ((filename (second sb-ext:*posix-argv*)))
  (unless filename
    (error "Usage: sbcl --script prettyprint.lisp <file.lisp>"))

  (with-open-file (in filename)
    (loop for form = (read in nil nil)
          while form
          do (pprint form)
             (terpri))))
             