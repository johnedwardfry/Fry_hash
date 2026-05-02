import sys
import re
import argparse
from pathlib import Path


def extract_code_parts(cpp_code):
    """
    Finds the main function and splits the code into two parts:
    1. The internal body of main().
    2. Everything else (includes, global variables, helper functions).
    """
    # Regex allows for potential comments/newlines before the opening brace
    match = re.search(r'\bint\s+main\s*\([^)]*\)[\s\S]*?\{', cpp_code)

    if not match:
        return None, None, "Error: No main() function found in the provided script."

    brace_start = match.end() - 1
    brace_count = 0
    end_index = -1

    in_string = False
    in_comment = False

    for i in range(brace_start, len(cpp_code)):
        char = cpp_code[i]
        prev_char = cpp_code[i - 1] if i > 0 else ''
        next_char = cpp_code[i + 1] if i < len(cpp_code) - 1 else ''

        # Handle Strings
        if char == '"' and prev_char != '\\' and not in_comment:
            in_string = not in_string

        # Handle Single-line Comments
        if not in_string:
            if char == '/' and next_char == '/':
                in_comment = True
            elif char == '\n':
                in_comment = False

        # Count Braces
        if not in_string and not in_comment:
            if char == '{':
                brace_count += 1
            elif char == '}':
                brace_count -= 1

            if brace_count == 0:
                end_index = i
                break

    if end_index != -1:
        # 1. Extract the main body
        raw_body = cpp_code[match.end():end_index].strip() + "\n"

        # 2. Extract definitions (everything before 'int main' and after the closing '}')
        before_main = cpp_code[:match.start()].strip()
        after_main = cpp_code[end_index + 1:].strip()

        # Combine them cleanly
        definitions = f"{before_main}\n\n{after_main}".strip() + "\n"

        return raw_body, definitions, None
    else:
        return None, None, "Error: Mismatched braces. Missing a closing '}'."


def process_files(source_path, target_path, output_path,
                  body_marker="// INSERT_CODE_HERE",
                  def_marker="// INSERT_DEFINITIONS_HERE"):
    """
    Extracts the main() body and definitions from the source,
    and inserts them into the target at their respective markers.
    """
    try:
        source_code = source_path.read_text(encoding='utf-8')
    except FileNotFoundError:
        sys.exit(f"Error: Could not find the source file '{source_path}'")

    extracted_body, extracted_defs, error = extract_code_parts(source_code)
    if error:
        sys.exit(error)

    try:
        target_code = target_path.read_text(encoding='utf-8')
    except FileNotFoundError:
        sys.exit(f"Error: Could not find the target file '{target_path}'")

    # Inject Definitions
    if def_marker in target_code:
        target_code = target_code.replace(def_marker, extracted_defs)
    else:
        print(f"Note: Definition marker '{def_marker}' not found in target. Skipping definitions insertion.")

    # Inject Main Body
    if body_marker in target_code:
        target_code = target_code.replace(body_marker, extracted_body)
    else:
        sys.exit(f"Error: The body marker '{body_marker}' was not found in '{target_path}'.")

    # Write output
    output_path.write_text(target_code, encoding='utf-8')

    print(f"Success! Extracted definitions and main() body from '{source_path.name}'")
    print(f"Merged code saved to: '{output_path}'")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Extract C++ definitions and main() body, injecting them into a template.")
    parser.add_argument("source", nargs="?", default=r"E:\projects\Fry_hash\reconstructor_alpha.cpp",
                        help="Path to the source C++ file")
    parser.add_argument("target", nargs="?", default=r"E:\projects\Fry_hash\loader.cpp",
                        help="Path to the target template file")
    parser.add_argument("output", nargs="?", default=r"E:\projects\Fry_hash\final_output.cpp",
                        help="Path for the resulting output file")

    args = parser.parse_args()

    if len(sys.argv) == 1:
        print("No CLI arguments provided. Falling back to default paths...\n")

    process_files(Path(args.source), Path(args.target), Path(args.output))