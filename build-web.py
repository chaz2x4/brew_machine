#!/usr/bin/env python

from pathlib import Path
import os

def process_file(file):
    content = []
    with open(file, 'r') as f:
        name, _ =  os.path.splitext(file.name)
        content.append(f'const char* {name}Html = ')
        for line in f:
            content.append('\"')
            content.append(
                line.translate(
                    str.maketrans(
                        {'\n' : '\\n\"\n',
                         '\"' : '\\"',
                         '\'' : '\\\''
                         }
                    )
                )
            )
        content.append('";')
    return content

files_content = process_file(Path('webpage') / 'index.html')
output_path = Path('include') / 'websites.h'
with output_path.open('w+') as f:
    f.write(''.join(files_content))
