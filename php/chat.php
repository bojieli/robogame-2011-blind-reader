<?php
include "db.php";
if (isset($_GET['s'])) {
	$_GET['s'] = addslashes($_GET['s']);
	if (isset($_GET['from']))
		mysql_query("INSERT INTO chat SET `from` = 1, `content` = '".$_GET['s']."'");
	else
		mysql_query("INSERT INTO chat SET `from` = 0, `content` = '".$_GET['s']."'");
}
?>
<html>
<meta http-equiv="refresh" content="10" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<body>
<?php
$rs = mysql_query("SELECT * FROM chat");
while ($data = mysql_fetch_array($rs)) {
	if ($data['from']) {
		echo 'Friend: ';
	}
	else {
		echo 'Blind : ';
	}
	echo $data['content'];
	echo '<br />';
}
?>
<form action="chat.php" method="get">
<p><textarea name="s"></textarea>
<input type="hidden" name="from" value="1" />
<p><button type="submit">Submit</button>
</form>
</body>
</html>
