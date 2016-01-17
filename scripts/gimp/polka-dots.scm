(define (script-fu-polka-dots image drawable cell-size top-dot-radius bottom-dot-radius)
  (let* ((image-width (car (gimp-image-width image)))
	 (image-height (car (gimp-image-height image))))

    (gimp-selection-none image)

    (let loop ((y (- (/ cell-size 2))) (s #f))
      (if (< y image-height)
	(let ((cell-radius (+ bottom-dot-radius
			      (* (/ y image-height)
				 (- top-dot-radius bottom-dot-radius)))))
	  (let loop ((x (+ (- (/ cell-size 2)) (if s (/ cell-size 2) 0))))
	    (if (< x image-width)
	      (begin
		(gimp-image-select-ellipse image
					   CHANNEL-OP-ADD
					   x
					   y
					   cell-radius
					   cell-radius)
		(loop (+ x cell-size)))))
	  (loop (+ y cell-size) (not s)))))

    (gimp-edit-fill drawable FOREGROUND-FILL)
    (gimp-selection-none image)
    (gimp-displays-flush)))

(script-fu-register
  "script-fu-polka-dots"
  "Polka Dots"
  ""
  ""
  ""
  ""
  ""
  SF-IMAGE "Image" 0
  SF-DRAWABLE "Drawable" 0
  SF-ADJUSTMENT "Cell size" '(32 1 128 1 10 0 1)
  SF-ADJUSTMENT "Top dot radius" '(12 1 128 1 10 0 1)
  SF-ADJUSTMENT "Bottom dot radius" '(12 1 128 1 10 0 1))

(script-fu-menu-register "script-fu-polka-dots" "<Image>/Filters/Render")
