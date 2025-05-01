#!/usr/bin/env python3

import os
import sys
import urllib.parse
import html

# Read environment variables
request_method = os.environ.get('REQUEST_METHOD', '')
content_length = int(os.environ.get('CONTENT_LENGTH', '0'))

# Read raw POST data from stdin
post_data = sys.stdin.read(content_length)

# Parse the POST data manually
parsed_data = urllib.parse.parse_qs(post_data)

# Build dynamic HTML response
print("Content-Type: text/html\n")
print("""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Raw POST Handler</title>
    <style>
        body { font-family: sans-serif; margin: 2em; }
        h1 { color: #333; }
        ul { padding-left: 1em; }
        li { margin: 0.5em 0; }
        code { background: #eee; padding: 2px 4px; border-radius: 3px; }
    </style>
</head>
<body>
    <h1>POST Data Received</h1>
    <p><strong>Request Method:</strong> {method}</p>
    <p><strong>Raw POST Body:</strong> <code>{raw}</code></p>
    <h2>Parsed Form Data:</h2>
    <ul>
"""
)

# Output each parsed key-value
for key, values in parsed_data.items():
    safe_key = html.escape(key)
    safe_values = ', '.join(html.escape(v) for v in values)
    print(f"<li><strong>{safe_key}:</strong> {safe_values}</li>")

print("""
    </ul>
    <p><a href="/">Back to form</a></p>
</body>
</html>
""")
