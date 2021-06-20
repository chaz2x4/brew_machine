#!/usr/bin/env python

from pathlib import Path
import os

folder = Path('webpage').rglob('*.html')
files = [x for x in folder]

content = []
for file in files:
    f = open(file, 'r')
    content.append('const char* ')
    (name, ext) =  os.path.splitext(file.name)
    content.append(name + 'Html = ')
    for line in f.readlines():
        content.append('\"')
        for char in line:
            if char == '\n':
                content.append('\\n\"')
            if char == '\"':
                content.append('\\')
            if char == '\'':
                content.append('\\\'')
            else:
                content.append(char)
    content.append('";')
    f.close()

# print(content)
output=''.join(content)

f = open('include/websites.h', 'w+')
f.write(output)
f.close()
