<?php
include "db.php";
$id = isset($_GET['id']) ? $_GET['id'] : 0;
$rs = mysql_query("SELECT * FROM slide ORDER BY `order` LIMIT $id,1");
$data = mysql_fetch_array($rs);
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<style>* {font-size:24px;}
p { margin-top: 10px; margin-bottom: 0px;}
.large {font-size:30px; font-weight:bold; margin-bottom: 0px;}</style>
</head>
<body style="overflow:hidden">
<div style="height: 95%; overflow:hidden;"><?=$data['content'] ?></div>
<?php if ($id>0) {  ?>
<a style="font-size:16px" href="slide.php?id=<?=$id-1 ?>">上一页</a>
<?php } else { ?>
<a style="font-size:16px">上一页</a>
<?php } ?>
&nbsp;&nbsp;<a style="font-size:16px" href="slide.php?id=<?=$id+1 ?>">下一页</a></p>
</body>
