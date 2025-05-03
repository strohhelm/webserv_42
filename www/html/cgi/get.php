<?php
header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html>
<html>
<head>
    <title>GET Query Viewer</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
        }
        h1 {
            color: #333;
        }
        .pair {
            margin: 5px 0;
        }
        .key {
            font-weight: bold;
            color: #0066cc;
        }
        .value {
            color: #333;
        }
        .raw {
            margin-bottom: 20px;
            font-family: monospace;
            background: #f9f9f9;
            padding: 10px;
            border-left: 4px solid #ccc;
        }
    </style>
</head>
<body>
    <h1>GET Request Parameters</h1>

    <?php
    $query = $_SERVER['QUERY_STRING'];

    if (!empty($query)) {
        echo "<div class='raw'><strong>Raw Query String:</strong> " . htmlspecialchars(urldecode($query)) . "</div>";

        // Parse the query string manually
        parse_str($query, $params);

        foreach ($params as $key => $value) {
            $decodedKey = htmlspecialchars(urldecode($key));
            $decodedValue = htmlspecialchars(urldecode($value));
            echo "<div class='pair'><span class='key'>$decodedKey:</span> <span class='value'>$decodedValue</span></div>";
        }
    } else {
        echo "<p>No query string received.</p>";
    }
    ?>
</body>
</html>
