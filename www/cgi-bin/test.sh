#!/bin/bash

echo "Content-Type: text/html"
echo ""

cat << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Bash CGI Test</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #10b981 0%, #065f46 100%);
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
            background: linear-gradient(135deg, #059669, #047857);
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
            border-bottom: 3px solid #059669;
            padding-bottom: 10px;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        
        .info-card {
            background: #f0fdfa;
            border: 1px solid #99f6e4;
            border-radius: 12px;
            padding: 20px;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }
        
        .info-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(5, 150, 105, 0.1);
        }
        
        .info-card strong {
            color: #059669;
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
            background: #ecfdf5;
            border-radius: 12px;
            padding: 20px;
            max-height: 400px;
            overflow-y: auto;
        }
        
        .env-item {
            display: flex;
            padding: 12px 0;
            border-bottom: 1px solid #a7f3d0;
            align-items: flex-start;
        }
        
        .env-item:last-child {
            border-bottom: none;
        }
        
        .env-key {
            color: #059669;
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
            background: #7c3aed;
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
            <h1>⚡ Bash CGI Test</h1>
EOF

echo "            <div class=\"time\">$(date -u '+%Y-%m-%d %H:%M:%S UTC')</div>"

cat << 'EOF'
            <div class="status-badge">CGI Execution Successful</div>
        </div>
        
        <div class="content">
            <div class="section">
                <h2>📊 Request Information</h2>
                <div class="info-grid">
                    <div class="info-card">
                        <strong>Request Method</strong>
EOF

echo "                        <span>${REQUEST_METHOD:-Unknown}</span>"

cat << 'EOF'
                    </div>
EOF

if [ -n "$QUERY_STRING" ]; then
    echo "                    <div class=\"info-card\">"
    echo "                        <strong>Query String</strong>"
    echo "                        <span>$QUERY_STRING</span>"
    echo "                    </div>"
fi

cat << 'EOF'
                    <div class="info-card">
                        <strong>Server</strong>
EOF

echo "                        <span>${SERVER_NAME:-Unknown}:${SERVER_PORT:-Unknown}</span>"

cat << 'EOF'
                    </div>
                    
                    <div class="info-card">
                        <strong>Script Name</strong>
EOF

echo "                        <span>${SCRIPT_NAME:-Unknown}</span>"

cat << 'EOF'
                    </div>
                </div>
            </div>
EOF

if [ "$REQUEST_METHOD" = "POST" ] && [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
    echo "            <div class=\"section\">"
    echo "                <h2>📝 POST Data</h2>"
    echo "                <div class=\"post-data\">"
    head -c "$CONTENT_LENGTH"
    echo "                </div>"
    echo "            </div>"
fi

cat << 'EOF'
            <div class="section">
                <h2>🌐 Environment Variables</h2>
                <div class="env-list">
EOF

env | grep -E '^(REQUEST_|SERVER_|HTTP_|CONTENT_|QUERY_STRING|SCRIPT_NAME|PATH_INFO|REMOTE_)' | sort | while IFS='=' read -r key value; do
    echo "                    <div class=\"env-item\">"
    echo "                        <div class=\"env-key\">$key</div>"
    echo "                        <div class=\"env-value\">$value</div>"
    echo "                    </div>"
done

cat << 'EOF'
                </div>
            </div>
        </div>
    </div>
</body>
</html>
EOF
