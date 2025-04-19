<?php
echo "Content-Type: text/html\n\n";
echo "<h1>Received POST</h1>";

echo "<h2>POST Data</h2>";
foreach ($_POST as $key => $value) {
    echo "<p><strong>$key:</strong> $value</p>";
}
?>
