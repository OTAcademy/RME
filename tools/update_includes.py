
import os
import re

# Configuration
SOURCE_DIR = 'source'
MOVED_FILES = {
    'graphics.h': 'rendering/graphics.h',
    'map_drawer.h': 'rendering/map_drawer.h',
    'map_display.h': 'rendering/map_display.h',
    'light_drawer.h': 'rendering/light_drawer.h',
    'gl_batch.h': 'rendering/gl_batch.h',
    'minimap_window.h': 'rendering/minimap_window.h',
}

def update_includes(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except UnicodeDecodeError:
        print(f"Skipping binary or invalid file: {file_path}")
        return

    original_content = content
    
    for old_header, new_header in MOVED_FILES.items():
        # Regex to match #include "old_header" or #include <old_header> 
        # We focus on quotes as these are local headers.
        # Use simple string replacement for safety if regex gets too complex, 
        # but regex ensures we only match full header names.
        
        # Pattern: #include "header" with optional whitespace
        # We want to replace "header" with "new_header"
        
        pattern = r'(#\s*include\s*")' + re.escape(old_header) + r'(")'
        replacement = r'\1' + new_header + r'\2'
        
        content = re.sub(pattern, replacement, content)

    if content != original_content:
        print(f"Updating {file_path}")
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)

def main():
    print("Starting include update...")
    for root, dirs, files in os.walk(SOURCE_DIR):
        for file in files:
            if file.endswith('.cpp') or file.endswith('.h'):
                file_path = os.path.join(root, file)
                update_includes(file_path)
    print("Finished.")

if __name__ == '__main__':
    main()
