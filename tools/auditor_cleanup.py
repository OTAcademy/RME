import os
import re

SOURCE_DIR = "source"
EXCLUDE_DIRS = [os.path.join("source", "ext")]
OTML_HEADER = os.path.join("source", "map", "otml.h")

def process_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except UnicodeDecodeError:
        print(f"Skipping binary/non-utf8 file: {filepath}")
        return

    new_lines = []
    modified = False

    for line in lines:
        original_line = line

        # 1. Remove commented debug code
        if re.match(r'^\s*//\s*(printf|sprintf|std::cout|std::cerr).*', line):
            modified = True
            continue # Skip adding this line

        # 2. NULL -> nullptr
        if "NULL" in line:
             new_line = re.sub(r'\bNULL\b', 'nullptr', line)
             if new_line != line:
                 line = new_line
                 modified = True

        # 3. typedef -> using
        if filepath != OTML_HEADER:
            if "typedef" in line:
                # Regex for: typedef [Type] [Alias];
                match = re.match(r'^(\s*)typedef\s+(.+?)\s+(\w+);\s*$', line)
                if match:
                    indent = match.group(1)
                    type_def = match.group(2)
                    alias_name = match.group(3)

                    if "struct {" in type_def or "enum {" in type_def:
                        pass
                    elif "(" in type_def and ")" in type_def:
                        pass # Skip function pointers
                    else:
                        line = f"{indent}using {alias_name} = {type_def};\n"
                        modified = True

        new_lines.append(line)

    if modified:
        print(f"Modifying {filepath}")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)

def main():
    for root, dirs, files in os.walk(SOURCE_DIR):
        # Exclude 'ext' directory
        if "ext" in dirs:
            if os.path.join(root, "ext") == os.path.join("source", "ext"):
                dirs.remove("ext")

        for file in files:
            if file.endswith(".h") or file.endswith(".cpp"):
                filepath = os.path.join(root, file)
                process_file(filepath)

if __name__ == "__main__":
    main()
