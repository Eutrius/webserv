<?php
header("Content-Type: text/html");

$method = $_SERVER['REQUEST_METHOD'] ?? 'Unknown';
$current_time = date('Y-m-d H:i:s \U\T\C');
$server_name = $_SERVER['SERVER_NAME'] ?? 'Unknown';
$server_port = $_SERVER['SERVER_PORT'] ?? 'Unknown';
$script_name = $_SERVER['SCRIPT_NAME'] ?? 'Unknown';
$query_string = $_SERVER['QUERY_STRING'] ?? '';

echo "<!DOCTYPE html>
<html lang=\"en\">
<head>
    <meta charset=\"UTF-8\">
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
    <title>PHP CGI Test</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #ff6b6b 0%, #ee5a52 100%);
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
            background: linear-gradient(135deg, #e11d48, #be123c);
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
            border-bottom: 3px solid #e11d48;
            padding-bottom: 10px;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        
        .info-card {
            background: #fef7f7;
            border: 1px solid #fecaca;
            border-radius: 12px;
            padding: 20px;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }
        
        .info-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(225, 29, 72, 0.1);
        }
        
        .info-card strong {
            color: #e11d48;
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
            background: #fef2f2;
            border-radius: 12px;
            padding: 20px;
            max-height: 400px;
            overflow-y: auto;
        }
        
        .env-item {
            display: flex;
            padding: 12px 0;
            border-bottom: 1px solid #fecaca;
            align-items: flex-start;
        }
        
        .env-item:last-child {
            border-bottom: none;
        }
        
        .env-key {
            color: #e11d48;
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
            background: #059669;
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
    <div class=\"container\">
        <div class=\"header\">
            <h1>🐘 PHP CGI Test</h1>
            <div class=\"time\">$current_time</div>
            <div class=\"status-badge\">CGI Execution Successful</div>
        </div>
        
        <div class=\"content\">
            <div class=\"section\">
                <h2>📊 Request Information</h2>
                <div class=\"info-grid\">
                    <div class=\"info-card\">
                        <strong>Request Method</strong>
                        <span>$method</span>
                    </div>";

if (!empty($query_string)) {
    echo "                    <div class=\"info-card\">
                        <strong>Query String</strong>
                        <span>" . htmlspecialchars($query_string) . "</span>
                    </div>";
}

echo "                    <div class=\"info-card\">
                        <strong>Server</strong>
                        <span>$server_name:$server_port</span>
                    </div>
                    
                    <div class=\"info-card\">
                        <strong>Script Name</strong>
                        <span>$script_name</span>
                    </div>
                </div>
            </div>";

if ($method === 'POST') {
    $post_data = file_get_contents('php://input');
    if (!empty($post_data)) {
        echo "            <div class=\"section\">
                <h2>📝 POST Data</h2>
                <div class=\"post-data\">" . htmlspecialchars($post_data) . "</div>
            </div>";
    }
}

echo "            <div class=\"section\">
                <h2>🌐 Environment Variables</h2>
                <div class=\"env-list\">";

foreach ($_SERVER as $key => $value) {
    if (strpos($key, 'REQUEST_') === 0 || 
        strpos($key, 'SERVER_') === 0 || 
        strpos($key, 'HTTP_') === 0 || 
        strpos($key, 'CONTENT_') === 0 || 
        $key === 'QUERY_STRING' || 
        $key === 'SCRIPT_NAME' || 
        $key === 'PATH_INFO' || 
        strpos($key, 'REMOTE_') === 0) {
        echo "                    <div class=\"env-item\">
                        <div class=\"env-key\">$key</div>
                        <div class=\"env-value\">" . htmlspecialchars($value) . "</div>
                    </div>";
    }
}

echo "                </div>
            </div>
        </div>
    </div>
</body>
</html>";
?>
