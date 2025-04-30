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

// Output HTTP header
echo "Content-Type: text/plain\r\n\r\n";

// Output starts here
echo "========================================\n";
echo "           POST CGI Test Output         \n";
echo "========================================\n\n";

// Request Info
echo ">>> Request Info:\n";
echo "    Method         : " . ($_SERVER['REQUEST_METHOD'] ?? 'UNKNOWN') . "\n";
echo "    Content-Type   : " . $content_type . "\n";
echo "    Content-Length : " . ($_SERVER['CONTENT_LENGTH'] ?? 'UNKNOWN') . "\n\n";

// Raw Body
echo ">>> Raw POST Body:\n";
echo "    " . str_replace("\n", "\n    ", trim($raw_body)) . "\n\n";

// Decoded Body
echo ">>> Decoded Parameters:\n";
if (!empty($decoded) && is_array($decoded)) {
    foreach ($decoded as $key => $value) {
        echo "    " . str_pad($key, 15) . ": $value\n";
    }
} else {
    echo "    (No decodable parameters found or unsupported format)\n";
}

echo "\n========================================\n";
?>
