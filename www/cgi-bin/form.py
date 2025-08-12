#!/usr/bin/env python3
import os
import urllib.parse
import sys

content_length = int(os.environ.get('CONTENT_LENGTH', 0))
post_data = ""
if content_length > 0:
    post_data = sys.stdin.read(content_length)

print("Content-Type: text/html")
print() 

print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Form Processing Result</title>
    <style>
        body {{ font-family: Arial, sans-serif; padding: 20px; background: #e74c3c; color: white; }}
        .result {{ background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; margin: 20px 0; }}
        pre {{ background: rgba(0,0,0,0.3); padding: 15px; border-radius: 5px; overflow-x: auto; }}
    </style>
</head>
<body>
    <h1>Form Processing Result</h1>
    <div class="result">
        <h3>Method:</h3>
        <p>{os.environ.get('REQUEST_METHOD', 'Unknown')}</p>
    </div>
    <div class="result">
        <h3>Raw POST Data:</h3>
        <pre>{post_data if post_data else 'No POST data received'}</pre>
    </div>
""")

if post_data:
    try:
        parsed_data = urllib.parse.parse_qs(post_data)
        print("<div class='result'><h3>Parsed Form Data:</h3><ul>")
        for key, values in parsed_data.items():
            for value in values:
                print(f"<li><strong>{key}:</strong> {value}</li>")
        print("</ul></div>")
    except Exception as e:
        print(f"<div class='result'><p>Could not parse form data: {e}</p></div>")

print("""
    <a href="/" style="color: #fff; text-decoration: underline;">← Back to Test Suite</a>
</body>
</html>
""")
