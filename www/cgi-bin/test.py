#!/usr/bin/env python3

import os
import sys
from datetime import datetime

print("Content-Type: text/html")
print()

print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Test</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #3b82f6, #1d4ed8);
            color: white;
            padding: 40px;
            text-align: center;
        }
        
        .header h1 {
            font-size: 2.5rem;
            font-weight: 700;
            margin-bottom: 10px;
        }
        
        .header .time {
            font-size: 1.1rem;
            opacity: 0.9;
        }
        
        .content {
            padding: 40px;
        }
        
        .section {
            margin-bottom: 40px;
        }
        
        .section h2 {
            color: #1e293b;
            font-size: 1.8rem;
            margin-bottom: 20px;
            border-bottom: 3px solid #3b82f6;
            padding-bottom: 10px;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        
        .info-card {
            background: #f8fafc;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            padding: 20px;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }
        
        .info-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(0, 0, 0, 0.1);
        }
        
        .info-card strong {
            color: #3b82f6;
            display: block;
            margin-bottom: 8px;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        .info-card span {
            color: #475569;
            font-size: 1rem;
            word-break: break-all;
        }
        
        .env-list {
            background: #f1f5f9;
            border-radius: 12px;
            padding: 20px;
            max-height: 400px;
            overflow-y: auto;
        }
        
        .env-item {
            display: flex;
            padding: 12px 0;
            border-bottom: 1px solid #e2e8f0;
            align-items: flex-start;
        }
        
        .env-item:last-child {
            border-bottom: none;
        }
        
        .env-key {
            color: #3b82f6;
            font-weight: 600;
            min-width: 200px;
            margin-right: 20px;
            font-size: 0.9rem;
        }
        
        .env-value {
            color: #475569;
            flex: 1;
            word-break: break-all;
            font-family: 'Courier New', monospace;
            font-size: 0.85rem;
        }
        
        .post-data {
            background: #fef3c7;
            border: 1px solid #f59e0b;
            border-radius: 8px;
            padding: 20px;
            font-family: 'Courier New', monospace;
            white-space: pre-wrap;
            word-break: break-all;
        }
        
        .status-badge {
            background: #10b981;
            color: white;
            padding: 8px 16px;
            border-radius: 20px;
            font-size: 0.9rem;
            font-weight: 600;
            display: inline-block;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🐍 Python CGI Test</h1>
            <div class="time">""" + datetime.now().strftime("%Y-%m-%d %H:%M:%S UTC") + """</div>
            <div class="status-badge">CGI Execution Successful</div>
        </div>
        
        <div class="content">
            <div class="section">
                <h2>📊 Request Information</h2>
                <div class="info-grid">""")

method = os.environ.get('REQUEST_METHOD', 'Unknown')
print(f"""                    <div class="info-card">
                        <strong>Request Method</strong>
                        <span>{method}</span>
                    </div>""")

query = os.environ.get('QUERY_STRING', '')
if query:
    print(f"""                    <div class="info-card">
                        <strong>Query String</strong>
                        <span>{query}</span>
                    </div>""")

server_name = os.environ.get('SERVER_NAME', 'Unknown')
server_port = os.environ.get('SERVER_PORT', 'Unknown')
print(f"""                    <div class="info-card">
                        <strong>Server</strong>
                        <span>{server_name}:{server_port}</span>
                    </div>
                    
                    <div class="info-card">
                        <strong>Script Name</strong>
                        <span>{os.environ.get('SCRIPT_NAME', 'Unknown')}</span>
                    </div>
                </div>
            </div>""")

if method == 'POST':
    content_length = os.environ.get('CONTENT_LENGTH', '0')
    if content_length.isdigit() and int(content_length) > 0:
        post_data = sys.stdin.read(int(content_length))
        print(f"""            <div class="section">
                <h2>📝 POST Data</h2>
                <div class="post-data">{post_data}</div>
            </div>""")

print("""            <div class="section">
                <h2>🌐 Environment Variables</h2>
                <div class="env-list">""")

for key, value in sorted(os.environ.items()):
    if key.startswith(('REQUEST_', 'SERVER_', 'HTTP_', 'CONTENT_', 'QUERY_', 'SCRIPT_', 'PATH_INFO', 'REMOTE_')):
        print(f"""                    <div class="env-item">
                        <div class="env-key">{key}</div>
                        <div class="env-value">{value}</div>
                    </div>""")

print("""                </div>
            </div>
        </div>
    </div>
</body>
</html>""")
