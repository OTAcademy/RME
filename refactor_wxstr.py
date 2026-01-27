import os
import re

def count_parens(s):
    count = 0
    for char in s:
        if char == '(':
            count += 1
        elif char == ')':
            count -= 1
    return count

def find_matching_paren(s, start_index):
    count = 0
    for i in range(start_index, len(s)):
        if s[i] == '(':
            count += 1
        elif s[i] == ')':
            count -= 1
            if count == 0:
                return i
    return -1

def process_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Pattern: wxstr(std::string(
    search_pattern = "wxstr(std::string("
    
    new_content = ""
    last_pos = 0
    
    modified = False
    
    first = True
    
    while True:
        start_pos = content.find(search_pattern, last_pos)
        if start_pos == -1:
            new_content += content[last_pos:]
            break
            
        # Found wxstr(std::string(
        # We need to find the closing paren for std::string(...)
        # The structure is wxstr(std::string( ARGS ))
        
        # position of first paren of std::string(
        inner_start = start_pos + len("wxstr(")
        # std::string starts at inner_start
        # inner_paren_start = inner_start + len("std::string")
        
        # verification
        check_str = content[start_pos:inner_start + len("std::string(")]
        # print(f"Found match: {check_str}")
        
        std_string_call_start = inner_start
        # Find where std::string(...) ends
        # content[std_string_call_start] starts with 'std::string('
        
        # Arg start is after 'std::string('
        arg_start = std_string_call_start + len("std::string(")
        
        # Find closing paren for std::string
        closing_paren_idx = find_matching_paren(content, arg_start - 1) # -1 to include the opening paren
        
        if closing_paren_idx == -1:
            print(f"Error parsing parens in {filepath} at {start_pos}")
            new_content += content[last_pos:start_pos+1]
            last_pos = start_pos + 1
            continue

        # Extract argument
        argument = content[arg_start:closing_paren_idx]
        
        # Now we replace wxstr(std::string(ARG)) with wxstr(ARG)
        # The original text was content[start_pos : closing_paren_idx + 1] -> This closes std::string
        # But wait, wxstr has a closing paren too!
        # wxstr( std::string( ... ) )
        #       ^ start             ^ closing_std  ^ closing_wxstr
        
        # Actually, simpler:
        # replace `std::string(` with nothing? No.
        # We want to keep `wxstr(` and then put `ARG` and then keep `)`.
        
        # We found `wxstr(std::string(` at `start_pos`.
        # Argument is `argument`.
        # `closing_paren_idx` is the `)` of `std::string`.
        # So we want to replace `std::string(ARG)` with `ARG`.
        
        # Reconstruct:
        # append everything before `std::string`
        new_content += content[last_pos:std_string_call_start]
        # append argument
        new_content += argument
        # skip to after closing paren of std::string
        last_pos = closing_paren_idx + 1
        modified = True

    if modified:
        print(f"Refactoring {filepath}")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)

def main():
    source_dir = "source"
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith(".cpp") or file.endswith(".h"):
                process_file(os.path.join(root, file))

if __name__ == "__main__":
    main()
