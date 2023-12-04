# SPDX-FileCopyrightText: 2023 C. J. Howard
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import csv

if __name__ == "__main__":
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='Generate a list of language tags in the header of a CSV file.')
    parser.add_argument('input_file', help='Input file')
    parser.add_argument('output_file', help='Output file')
    args = parser.parse_args()
    
    # Read header
    with open(args.input_file, 'r', encoding='utf-8') as file:
        header = next(csv.reader(file), None)
    
    # Collect language tags
    tags = [tag for tag in header if tag and tag not in ("key", "context")]
    
    # Generate manifest
    with open(args.output_file, 'w', encoding='utf-8') as file:
        file.write('\n'.join(tags))
