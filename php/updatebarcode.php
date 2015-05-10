<?php
include "db.php";

// EAN-13:00000000000000
$input = explode(':', $_GET['barcode']);
$barcode = $input[1];

if (is_numeric($barcode)) {
	mysql_query("INSERT INTO barcode SET `content` = '$barcode'");
}

// echo back to screen
echo $barcode."\n";
?>
