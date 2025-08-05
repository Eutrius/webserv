#!/usr/bin/env python3
import os
import datetime

print("Content-Type: text/html\r")
print("\r")
print("""<!DOCTYPE html>
<html>
<head>
    <title>Python CGI Info</title>
    <style>
        body { font-family: monospace; padding: 20px; background: #1a1a1a; color: #00ff00; }
        .info { background: #333; padding: 15px; margin: 10px 0; border-radius: 5px; }
        h1 { color: #00ffff; }
    </style>
</head>
<body>
    <h1>Python CGI Information</h1>
    <div class="info">
        <h3>Current Time:</h3>
        <p>{}</p>
    </div>
    <div class="info">
        <h3>Python Version:</h3>
        <p>{}</p>
    </div>
    <div class="info">
        <h3>Environment Variables:</h3>
        <pre>""".format(
            datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            os.sys.version
        ))

for key, value in sorted(os.environ.items()):
    print(f"{key} = {value}")

print("""</pre>
    </div>
    <a href="/" style="color: #00ffff;">← Back to Test Suite</a>
</body>
</html>""")
