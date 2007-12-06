;; Neutrino mode.  Plenty of inspiration taken from scala-mode.

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

(defconst neutrino-keywords-regexp
  (regexp-opt
   '("def" "class" "new")
   'words))

(defconst neutrino-constants-regexp
  (regexp-opt
   '("this" "true" "false" "null")
   'words))

(defvar neutrino-font-lock-keywords
  `((,neutrino-keywords-regexp 0 font-lock-keyword-face nil)
    (,neutrino-constants-regexp 0 font-lock-constant-face nil)))

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

(provide 'neutrino-mode)
