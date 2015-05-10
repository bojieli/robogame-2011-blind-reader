<?php
$fp = fopen("chat_blind.txt", "r");
fscanf($fp, "%s", $str);
exec("curl localhost/blind/lib_braille.php?str=".$str);
include "db.php";
mysql_query("INSERT INTO chat SET `from` = 0, `content` = '$str'");
?>
