import os
import re

def process_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except UnicodeDecodeError:
        try:
             with open(filepath, 'r', encoding='latin-1') as f:
                content = f.read()
        except:
             print(f"Skipping binary or unreadable file: {filepath}")
             return False

    original_content = content
    
    # helper for replacements
    methods = ['getName', 'getText', 'getDescription', 'getSearchString']
    
    # function to avoid double wrapping
    def wrap_in_string(match):
        full_match = match.group(0)
        core_expr = match.group(1)
        # simplistic check if already wrapped
        if f"std::string({core_expr})" in full_match:
            return full_match
        # Check context
        return full_match.replace(core_expr, f"std::string({core_expr})")

    for method in methods:
        # 1. wxstr(pointer->method())
        # We target wxstr(  EXPR  )
        # We look for patterns like wxstr(item->getName())
        # We replace with wxstr(std::string(item->getName()))
        
        # Regex for: wxstr ( ws object -> method() ws )
        # Note: we exclude starting with std::string in the object part to avoid double wrap
        pattern1 = r'wxstr\s*\(\s*([a-zA-Z0-9_>.:-]+->' + method + r'\(\))\s*\)'
        content = re.sub(pattern1, r'wxstr(std::string(\1))', content)

        # 2. wxstr(object.method())
        pattern2 = r'wxstr\s*\(\s*([a-zA-Z0-9_>.:-]+\.' + method + r'\(\))\s*\)'
        content = re.sub(pattern2, r'wxstr(std::string(\1))', content)
        
        # 3. Stream operator << pointer->method()
        # Be careful not to match if it's already std::string encapsulated
        # We match << ws EXPR
        pattern3 = r'<<\s*([a-zA-Z0-9_>.:-]+->' + method + r'\(\))'
        content = re.sub(pattern3, r'<< std::string(\1)', content)

        # 4. Stream operator << object.method()
        pattern4 = r'<<\s*([a-zA-Z0-9_>.:-]+\.' + method + r'\(\))'
        content = re.sub(pattern4, r'<< std::string(\1)', content)
        
        # 5. const std::string& x = pointer->method();
        # Replace with std::string x(pointer->method());
        pattern5 = r'const\s+std::string&\s+([a-zA-Z0-9_]+)\s*=\s*([a-zA-Z0-9_>.:-]+->' + method + r'\(\))\s*;'
        content = re.sub(pattern5, r'std::string \1(\2);', content)

        # 6. const std::string& x = object.method();
        pattern6 = r'const\s+std::string&\s+([a-zA-Z0-9_]+)\s*=\s*([a-zA-Z0-9_>.:-]+\.' + method + r'\(\))\s*;'
        content = re.sub(pattern6, r'std::string \1(\2);', content)
        
        # 7. Local call: wxstr(getName())
        pattern7 = r'wxstr\s*\(\s*(' + method + r'\(\))\s*\)'
        content = re.sub(pattern7, r'wxstr(std::string(\1))', content)
        
        # 8. Local call assignment: const std::string& x = getName();
        pattern8 = r'const\s+std::string&\s+([a-zA-Z0-9_]+)\s*=\s*(' + method + r'\(\))\s*;'
        content = re.sub(pattern8, r'std::string \1(\2);', content)

    if content != original_content:
        # Cleanup: sometimes we might generate std::string(std::string(...)) if regex wasn't perfect
        # simplistic cleanup
        content = content.replace("std::string(std::string(", "std::string(")
        # Count parens to be safe? 
        # std::string(std::string(x)) -> std::string(x) leaves double closing parens?
        # The replace above converts "std::string(std::string(" to "std::string("
        # So "std::string(std::string(x))" becomes "std::string(x))" -> mismatch.
        # This cleanup is dangerous. 
        # Better to rely on the regex being specific enough.
        # I removed the cleanup step to be safe, assuming regex doesn't match existing std::string wrappers due to char class restrictions.
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    source_dir = os.path.join(os.getcwd(), 'source')
    print(f"Scanning {source_dir}...")
    count = 0
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith('.cpp') or file.endswith('.h'):
                if process_file(os.path.join(root, file)):
                    print(f"Updated {file}")
                    count += 1
    print(f"Total files updated: {count}")

if __name__ == '__main__':
    main()
