#!/usr/bin/python3
import cgi
import time

print("Content-type: text/html")
print()

form = cgi.FieldStorage()
time.sleep(30)
a = form.getvalue("a")
b = form.getvalue("b")

result = int(a) * int(b)

print(f"Hello world {result}")
