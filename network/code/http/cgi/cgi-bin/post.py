#!/usr/bin/python3
import sys
import os

length = os.getenv("CONTENT_LENGTH")
print("Content-type: text/html")
print()
body = sys.stdin.read(int(length))
print(f"This is {body} len: {length}")
