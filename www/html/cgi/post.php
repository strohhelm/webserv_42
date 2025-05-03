#!/usr/bin/php-cgi
<?php
// Read raw POST data
$raw_body = file_get_contents("php://stdin");

// Detect content type
$content_type = $_SERVER['CONTENT_TYPE'] ?? 'unknown';

// Decode parameters based on content type
$decoded = [];
if (strpos($content_type, 'application/x-www-form-urlencoded') !== false) {
    parse_str($raw_body, $decoded);
} elseif (strpos($content_type, 'application/json') !== false) {
    $decoded = json_decode($raw_body, true);
}

// Output HTML header
echo "Content-Type: text/html\r\n\r\n";
?>
<!DOCTYPE html>
<html>
<head>
    <title>POST CGI Test Output</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
        }
        h1 {
            color: #333;
        }
        pre {
            background: #f4f4f4;
            padding: 10px;
            border-left: 4px solid #ccc;
            white-space: pre-wrap;
        }
        .section {
            margin-bottom: 30px;
        }
        .key {
            font-weight: bold;
            color: #0066cc;
        }
        .value {
            color: #333;
        }
        .pair {
            margin: 5px 0;
        }
    </style>
</head>
<body>
    <h1>POST CGI Test Output</h1>

    <div class="section">
        <h2>Request Info</h2>
        <div class="pair"><span class="key">Method:</span> <span class="value"><?php echo htmlspecialchars($_SERVER['REQUEST_METHOD'] ?? 'UNKNOWN'); ?></span></div>
        <div class="pair"><span class="key">Content-Type:</span> <span class="value"><?php echo htmlspecialchars($content_type); ?></span></div>
        <div class="pair"><span class="key">Content-Length:</span> <span class="value"><?php echo htmlspecialchars($_SERVER['CONTENT_LENGTH'] ?? 'UNKNOWN'); ?></span></div>
    </div>

    <div class="section">
        <h2>Raw POST Body</h2>
        <pre><?php echo htmlspecialchars($raw_body); ?></pre>
    </div>

    <div class="section">
        <h2>Decoded Parameters</h2>
        <?php
        if (!empty($decoded) && is_array($decoded)) {
            foreach ($decoded as $key => $value) {
                echo "<div class='pair'><span class='key'>" . htmlspecialchars($key) . ":</span> <span class='value'>" . htmlspecialchars($value) . "</span></div>";
            }
        } else {
            echo "<p>(No decodable parameters found or unsupported format)</p>";
        }
        ?>
    </div>
</body>
</html>
