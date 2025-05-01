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


// while(1)
// {
//     sleep(1);
// }
// Required CGI header
echo "Content-Type: text/html\n\n";
// Start HTML output
echo "<!DOCTYPE html>";
echo "<html><head><title>POST CGI Test</title></head><body>";
echo "<h1>POST CGI Test (PHP)</h1>";

// Request Info
echo "<h2>Request Info</h2>";
echo "<ul>";
echo "<li><strong>Request Method:</strong> " . htmlspecialchars($_SERVER['REQUEST_METHOD'] ?? 'UNKNOWN') . "</li>";
echo "<li><strong>Content Type:</strong> " . htmlspecialchars($content_type) . "</li>";
echo "<li><strong>Content Length:</strong> " . htmlspecialchars($_SERVER['CONTENT_LENGTH'] ?? 'UNKNOWN') . "</li>";
echo "</ul>";

// Raw POST Body
echo "<h2>Raw POST Body</h2>";
echo "<pre>" . htmlspecialchars($raw_body) . "</pre>";

// Decoded Parameters
echo "<h2>Decoded Parameters</h2>";
if (!empty($decoded) && is_array($decoded)) {
    echo "<ul>";
    foreach ($decoded as $key => $value) {
        echo "<li><strong>" . htmlspecialchars($key) . ":</strong> " . htmlspecialchars($value) . "</li>";
    }
    echo "</ul>";
} else {
    echo "<p>No decodable parameters found or unsupported format.</p>";
}

// End HTML
echo "</body></html>";
?>
