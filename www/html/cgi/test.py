#!/usr/bin/env python3

import os
import cgi
import cgitb
cgitb.enable()  # Enable CGI debugging output (optional, super helpful)

print("Content-Type: text/html\r\n")

# Parse query parameters
form = cgi.FieldStorage()

# Get 'name' and 'language' parameters
name = form.getfirst('name', '').strip()
language = form.getfirst('language', '').strip()

# Start HTML output
print("\r\n")  # Very important: empty line after headers
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>CGI Input Test</title></head>")
print("<body>")

# Logic: check if inputs exist
if not name or not language:
    print("<h1>Error: Missing parameters!</h1>")
    print("<p>You must provide both <strong>name</strong> and <strong>language</strong> in the query string.</p>")
    print("<p>Example: <code>?name=Alice&language=Python</code></p>")
else:
    print(f"<h1>Hello, {name}!</h1>")
    print(f"<p>It is awesome that you like the \"{language}\" language!.</p>")

print("</body>")
print("</html>")