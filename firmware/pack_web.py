import os
import gzip
import sys

# Paths
INPUT_HTML = "web/index.html"
OUTPUT_HEADER = "AllSeeingEye/src/WebStatic.h"

def pack():
    if not os.path.exists(INPUT_HTML):
        print(f"[ERROR] {INPUT_HTML} not found!")
        sys.exit(1)
        
    print(f"[PACK] Compressing {INPUT_HTML} -> {OUTPUT_HEADER}...")
    
    with open(INPUT_HTML, 'rb') as f:
        content = f.read()
        
    # Gzip
    compressed = gzip.compress(content)
    
    # Write Header
    with open(OUTPUT_HEADER, 'w') as f:
        f.write("#ifndef WEBSTATIC_H\n")
        f.write("#define WEBSTATIC_H\n\n")
        f.write("#include <Arduino.h>\n\n")
        
        f.write(f"const uint32_t index_html_gz_len = {len(compressed)};\n")
        f.write("const uint8_t index_html_gz[] PROGMEM = {\n")
        
        # Hex Dump
        for i, byte in enumerate(compressed):
            f.write(f"0x{byte:02X}")
            if i < len(compressed) - 1:
                f.write(", ")
            if (i + 1) % 16 == 0:
                f.write("\n")
                
        f.write("\n};\n\n")
        f.write("#endif\n")
        
    print(f"[PACK] Done. Size: {len(content)} -> {len(compressed)} bytes.")

if __name__ == "__main__":
    pack()
