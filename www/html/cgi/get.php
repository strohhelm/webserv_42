<?php
// Required CGI header
echo "Content-Type: text/html\n\n";

// Start HTML
echo "<!DOCTYPE html>";
echo "<html><head><title>PHP CGI Test</title></head><body>";
echo "<h1>Hello from PHP-CGI!</h1>";

// Show current server time
echo "<p><strong>Server Time:</strong> " . date("Y-m-d H:i:s") . "</p>";

// Show GET parameters (if any)
echo "<h2>GET Parameters</h2>";
if (!empty($_GET)) {
    echo "<ul>";
    foreach ($_GET as $key => $value) {
        echo "<li><strong>$key:</strong> $value</li>";
    }
    echo "</ul>";
} else {
    echo "<p>No GET parameters.</p>";
}

// Show some environment details
echo "<h2>Server Info (from \$_SERVER)</h2>";
echo "<ul>";
echo "<li><strong>REQUEST_METHOD:</strong> " . $_SERVER['REQUEST_METHOD'] . "</li>";
echo "<li><strong>SCRIPT_FILENAME:</strong> " . $_SERVER['SCRIPT_FILENAME'] . "</li>";
echo "<li><strong>SERVER_PROTOCOL:</strong> " . $_SERVER['SERVER_PROTOCOL'] . "</li>";
echo "<li><strong>GATEWAY_INTERFACE:</strong> " . $_SERVER['GATEWAY_INTERFACE'] . "</li>";
echo "</ul>";

// Dump all env variables (for debugging)
echo "<h2>Environment Dump</h2>";
echo "<pre>";
print_r($_SERVER);
echo "</pre>";

while (true) {
    # code...
}

// End HTML
echo "</body></html>";
?>
