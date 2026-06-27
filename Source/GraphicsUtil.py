# A small utility to convert a GIF to a set of frames blobbed together

from PIL import Image
import struct
import sys



if "gif" in sys.argv[1].lower():
   
    magic = int("ANIM".encode('utf-8').hex(), 16)

    gif = Image.open(sys.argv[1])

    frames = []

    while True:
        frame = gif.convert("RGBA")
        # Needed in BGRA format for UEFI
        r, g, b, a = frame.split()
        frame = Image.merge("RGBA", (b, g, r, a))
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



elif "png" in sys.argv[1]:

    img = Image.open(sys.argv[1]).convert("RGBA")

    width, height = img.size
    pixels = list(img.getdata())

    with open(f"{sys.argv[1].replace(".png", "")}.pic", "wb") as f:
        f.write(width.to_bytes(4, "little"))
        f.write(height.to_bytes(4, "little"))

        for r, g, b, a in pixels:
            # Exclude alpha-blended pixels as UEFI does not support that
            final_alpha = 255 if a >= 128 else 0
            if final_alpha == 0:
                f.write(bytes([0, 0, 0, 0]))
            else:
                f.write(bytes([b, g, r, final_alpha]))

    print(f"Successfully converted to {sys.argv[1].replace(".png", "")}.pic.")
