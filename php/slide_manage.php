<html><head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<style>textarea {font-size: 16px; width:500px; height:200px;} div {border: 2px dashed #CCC; padding: 10px; }
* {font-size:24px;}
p {margin-top: 10px; margin-bottom: 0px;}
p.large {font-size:30px; font-weight:bold; margin-bottom: 0px;}</style>
</head><body>
<?php
include "db.php";
if (isset($_GET['id'])) {
	$id = addslashes($_GET['id']);
	$order = addslashes($_POST['order']);
	$content = addslashes($_POST['content']);
	if (!empty($content))
		mysql_query("UPDATE slide SET `order` = '$order', `content` = '$content' WHERE `id` = '$id'");
}
else if (isset($_GET['new'])) {
	$order = addslashes($_POST['order']);
	$content = addslashes($_POST['content']);
	mysql_query("INSERT INTO slide SET `order` = '$order', `content` = '$content'");
}
$rs = mysql_query("SELECT * FROM slide ORDER BY `order`");
while ($data = mysql_fetch_array($rs)) { ?>
<form action="slide_manage.php?id=<?=$data['id'] ?>" method="post">
<p>Order:  <input name="order" value="<?=$data['order']?>" />
<table>
<tr><td><textarea name="content"><?=$data['content']?></textarea>
<td width="600px"><div><?=$data['content'] ?></div></tr>
</table>
<p><button type="submit">提交</button>
</form>
<hr />
<?php } ?>
<h2>New Slide</h2>
<form action="slide_manage.php?new" method="post">
<p>Order:  <input name="order">
<p><textarea name="content"></textarea>
<p><button type="submit">提交</button>
