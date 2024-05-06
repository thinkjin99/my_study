#!/usr/bin/python3
import cgi
import os

print("Content-type: text/html")
print()

form = cgi.FieldStorage()

a = form.getvalue("a")
b = form.getvalue("b")

result = int(a) * int(b)

print(f"Hello world {result}")
