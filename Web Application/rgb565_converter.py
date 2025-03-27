from glob import glob
import numpy as np
from PIL import Image

# Obtain all RGB565 buffer arrays transferred by Arduino Nicla Vision as text (.txt) files.
path = "C:\\Users\\kutlu\\New E\\xampp\\htdocs\\pipeline_diagnostics_interface\\detections"
images = glob(path + "/*.txt")

# Convert each RGB565 buffer (TXT file) to a JPG image file and save the generated image files to the images folder.
for img in images:
    loc = path + "/images/" + img.split("\\")[8].split(".")[0] + ".jpg"
    size = (320,320)
    # RGB565 (uint16_t) to RGB (3x8-bit pixels, true color)
    raw = np.fromfile(img).byteswap(True)
    file = Image.frombytes('RGB', size, raw, 'raw', 'BGR;16', 0, 1)
    file.save(loc)
    #print("Converted: " + loc)