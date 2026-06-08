# A small utility to convert a GIF to a set of frames blobbed together

from PIL import Image
import struct
import sys

magic = int("ANIM".encode('utf-8').hex(), 16)

gif = Image.open(sys.argv[1])

frames = []

while True:
    frame = gif.convert("RGBA")
    frames.append(frame)
    try:
        gif.seek(gif.tell() + 1)
    except EOFError:
        break

width, height = frames[0].size

with open(f"{sys.argv[1].replace(".gif", "")}.anim", "wb") as f:

    header = struct.pack("<IIIII", magic, width, height, len(frames), 15)
    f.write(header)

    for frame in frames:
        f.write(frame.tobytes())
    
print(f"Successfully animated {sys.argv[1].replace(".gif", "")}.anim.")
