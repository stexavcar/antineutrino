;; Neutrino mode.  Inspiration taken from scala-, sml- and java-mode.

;; To enable this mode for all .n files add this line to your .emacs
;; file (or local equivalent):
;; 
;; (setq auto-mode-alist (cons '("\\.n$" . neutrino-mode) auto-mode-alist))

(require 'regexp-opt)

;; Customization

(defgroup neutrino
  nil
  "Mode to edit Neutrino code."
  :group 'languages)

(defcustom neutrino-indent-step 2
  "Indentation step."
  :type 'integer
  :group 'neutrino)

;; Indentation

(defun neutrino-indentation ()
  "Returns the suggested indentation for the current line"
  (save-excursion
    (beginning-of-line)
    (or (neutrino-indentation-from-block)
        0)))

(defun neutrino-parse-partial-sexp ()
  (parse-partial-sexp (point-min) (point)))

(defun neutrino-indentation-from-block ()
  (save-excursion
    (let* ((state (neutrino-parse-partial-sexp))
           (block-start (nth 1 state)))
      (if (not block-start)
          0
        (goto-char (1+ block-start))
        (neutrino-block-indentation)))))

(defmacro neutrino-point-after(&rest body)
  `(save-excursion
     ,@body
     (point)))

(defun neutrino-forward-spaces (&optional limit)
  (if limit
      (save-restriction
        (narrow-to-region (point) limit)
        (forward-comment 100000))
    (forward-comment 100000)))

(defun neutrino-block-indentation ()
  (let ((block-start-eol (neutrino-point-after (end-of-line)))
        (block-after-spc (neutrino-point-after (neutrino-forward-spaces))))
    (if (> block-after-spc block-start-eol)
        (+ (current-indentation) neutrino-indent-step)
      (current-column))))

(defun neutrino-indent-line ()
  (interactive)
  (let ((indentation (neutrino-indentation)))
    (indent-line-to indentation)))

;; Syntax coloring

;; All the language's keywords have to occur between the begin and
;; end markers or a build check will fail.

;; -- begin keywords --

(defconst neutrino-keywords-regexp
  (regexp-opt
   '("def" "new" "and" "or" "not" "if" "else" "return" "fn"
     "in" "while" "do" "operator" "on" "raise" "protocol" "native"
     "task" "yield" "var" "while" "assert" "super" "for"
     "rec")
   'words))

(defconst neutrino-modifiers-regexp
  (regexp-opt
   '("internal" "channel" "static" "factory")
   'words))

(defconst neutrino-constants-regexp
  (regexp-opt
   '("this" "true" "false" "null")
   'words))

;; -- end keywords --

(defvar neutrino-font-lock-keywords
  `(("\\<def\\>\\ +\\<\\(\\w+\\)\\>\\ *(" 1 font-lock-function-name-face nil)
    ("\\<\\(def\\|var\\|rec\\)\\>\\ +\\<\\(\\w+\\)\\>" 2 font-lock-variable-name-face nil)
    (,neutrino-keywords-regexp 0 font-lock-keyword-face nil)
    (,neutrino-constants-regexp 0 font-lock-constant-face nil)
    ("[A-Z]\\w+" 0 font-lock-type-face nil)
    ("@\\w+" 0 font-lock-constant-face nil)
    (,neutrino-modifiers-regexp 0 font-lock-constant-face nil)))


(define-derived-mode neutrino-mode fundamental-mode "Neutrino"
  "Major mode for editing Neutrino code"
  
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults
        '(neutrino-font-lock-keywords
          nil
          nil
          ((?\_ . "w"))
          nil))

  (make-local-variable 'indent-line-function)
  (setq indent-line-function #'neutrino-indent-line))

;; End-of-line comments.  Pretty convoluted eh.  Here's how it works.
;; the first entry specifies that '/' is the first (1) character and
;; also the second (2) character that starts b-style comments.  The
;; second line specifies that \n ends b-style comments.
(modify-syntax-entry ?\/ ". 12b" neutrino-mode-syntax-table)
(modify-syntax-entry ?\n "> b"   neutrino-mode-syntax-table)

;; Multi-line comments.  The first line specifies that '(' is the
;; first character of a-style comments.  The second line says that
;; '*' is the second character (2) of a-style comments and also the
;; first character that ends them (3).  Finally, the last line
;; says that ')' is the second character that ends a-style comments.
(modify-syntax-entry ?\( "()1"   neutrino-mode-syntax-table)
(modify-syntax-entry ?*  ". 23"  neutrino-mode-syntax-table)
(modify-syntax-entry ?\) ")(4"   neutrino-mode-syntax-table)

(provide 'neutrino-mode)
