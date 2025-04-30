#!/usr/bin/env python3

import cgi
import cgitb
cgitb.enable()  # Enable debugging output

print("Content-Type: text/html\n")

form = cgi.FieldStorage()

html_response = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>POST Request Test</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .container {{ max-width: 600px; margin: auto; }}
        .field {{ margin-bottom: 10px; }}
        .label {{ font-weight: bold; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>POST Request Received</h1>
        <p>Here are the submitted values:</p>
        <ul>
            {items}
        </ul>
        <a href="/cgi-bin/post_test.py">Submit another</a>
    </div>
</body>
</html>
"""

items_html = ""
if form:
    for key in form.keys():
        value = form.getvalue(key)
        items_html += f"<li><span class='label'>{key}:</span> {value}</li>\n"
else:
    items_html = "<li>No POST data received.</li>"

print(html_response.format(items=items_html))