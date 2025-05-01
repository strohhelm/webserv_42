#!/usr/bin/php-cgi
<?php
// Read raw POST data
$raw_body = file_get_contents("php://stdin");

// Detect content type
$content_type = $_SERVER['CONTENT_TYPE'] ?? 'unknown';
$decoded = [];

// Decode based on content type
if (strpos($content_type, 'application/x-www-form-urlencoded') !== false) {
    parse_str($raw_body, $decoded);
} elseif (strpos($content_type, 'application/json') !== false) {
    $decoded = json_decode($raw_body, true);
}

// Required CGI header
echo "Content-Type: text/html\n\n";

// Start HTML output
echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>🛰 POST CGI Console</title>
    <link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@500&display=swap" rel="stylesheet">
    <style>
        body {
            background: url('https://images.unsplash.com/photo-1477201389074-1863f668fac6?auto=format&fit=crop&w=1920&q=80') no-repeat center center fixed;
            background-size: cover;
            color: #00ffe7;
            font-family: 'Orbitron', sans-serif;
            padding: 40px;
        }

        h1, h2 {
            text-shadow: 0 0 12px #00ffe7, 0 0 24px #00bfbf;
        }

        h1 {
            font-size: 2.2rem;
            text-align: center;
            margin-bottom: 40px;
        }

        .section {
            background: rgba(0, 0, 0, 0.75);
            border: 2px solid #00ffe7;
            box-shadow: 0 0 15px #00ffe7;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 30px;
        }

        ul {
            list-style: none;
            padding-left: 0;
        }

        li {
            margin: 8px 0;
        }

        strong {
            color: #0ff;
        }

        pre {
            background: #111;
            padding: 10px;
            border-radius: 8px;
            border: 1px solid #00bfbf;
            color: #0ff;
            overflow-x: auto;
        }

        .footer {
            text-align: center;
            margin-top: 50px;
            color: #aad;
            text-shadow: 0 0 10px #00ffe7;
        }
    </style>
</head>
<body>

    <h1>🛰 POST CGI Console (PHP)</h1>

    <div class="section">
        <h2>Request Info</h2>
        <ul>
            <li><strong>Request Method:</strong> ${_SERVER['REQUEST_METHOD']}</li>
            <li><strong>Content Type:</strong> ${content_type}</li>
            <li><strong>Content Length:</strong> ${_SERVER['CONTENT_LENGTH']}</li>
        </ul>
    </div>

    <div class="section">
        <h2>Raw POST Body</h2>
        <pre>${raw_body}</pre>
    </div>

    <div class="section">
        <h2>Decoded Parameters</h2>
HTML;

// Decoded parameters output
if (!empty($decoded) && is_array($decoded)) {
    echo "<ul>";
    foreach ($decoded as $key => $value) {
        echo "<li><strong>" . htmlspecialchars($key) . ":</strong> " . htmlspecialchars($value) . "</li>";
    }
    echo "</ul>";
} else {
    echo "<p>No decodable parameters found or unsupported format.</p>";
}

// Close HTML
echo <<<HTML
    </div>

    <div class="footer">
        🌌 Signal Received — CGI Transmission Complete
    </div>

</body>
</html>
HTML;
?>
