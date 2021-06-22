import cv2 as cv
import os

# _tdir = os.getcwd() + '/PY/vexCV/template.jpg'
_tdir = os.path.abspath(__file__).rpartition('\\')[0] + "/template.jpg"
print(f"Template target is '{_tdir}'")
cv_template_raw = cv.imread(_tdir, 0)
cv_template_blur = cv.GaussianBlur(cv_template_raw, (5,5), 0)
cv_template = cv.Canny(cv_template_blur, 5, 50)

cv_template_h, cv_template_w = cv_template.shape

print("CV init done")