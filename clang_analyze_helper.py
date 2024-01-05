#!/bin/python3

# This code is licenced under the MIT license:
#
# Copyright (c) 2024 Hottinger Brüel & Kjær
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import json
import sys
import os

# Call cmake to build compile_commands.json
os.system("cmake -S . -B build_clang -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DJETPROXY_POST_BUILD_UNITTEST=OFF -DJETPROXY_EXAMPLES=OFF")

# This script removes all _deps from an input compile_commands.json file
cc_json_file_path = "build_clang/compile_commands.json"
f = open(cc_json_file_path,)
compile_commands = json.load(f)

output_compile_commands = [x for x in compile_commands if x['directory'].find("_deps") == -1]

with open(cc_json_file_path, 'w') as outfile:
    json.dump(output_compile_commands, outfile)

os.system("run-clang-tidy -p build_clang")
os.system("rm -rf build_clang")
