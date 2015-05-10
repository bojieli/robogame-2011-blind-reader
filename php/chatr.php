<?php
include "db.php";
$rs = mysql_query("SELECT * FROM chat ORDER BY id DESC LIMIT 1");
$data = mysql_fetch_array($rs);
echo $data['content'];
?>
