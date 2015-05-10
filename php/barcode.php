<?php
// module/blind/barcode.php 2011-08-27
include "db.php";
$rs = mysql_query("SELECT content FROM barcode ORDER BY id DESC LIMIT 1");
$data = mysql_fetch_array($rs);
$content = $data['content'];
exec("curl localhost/blind/lib_braille.php?str=".$content);

/*
if (!defined('IN_GEWU')) {
	exit(header('HTTP/1.1 403 Forbidden'));
}

$barcode = OBJ::select(CURRENT_PATH, array('content'), array('class' => 'barcode'), 'create_time DESC', '1');
$barcode = $barcode[0]['content'];

$data = OBJ::select(CURRENT_PATH, array('abstract', 'content'), array('title' => $barcode, 'class' => 'barcodedata'));
if (empty($data)) {
	$data['abstract'] = $data['content'] = '';
	$not_found = true;
} else {
	$data = $data[0];
}

$content = $data['abstract'];

// add 0s to meet 20 bits boundary
$len = strlen($content);
if ($len % 20 != 0) {
	$num = 20 - $len % 20;
	for ($i=0; $i<$num; $i++) {
		$content .= '0';
	}
}

require_once modulefile('blind', 'lib_braille');
$content .= braille($data['content']);
OBJ::update(CURRENT_PATH, array('content' => $content));
?>
<html><head>
<meta http-equiv="refresh" content="1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>
<body>
<h2>条码<br /><?=$barcode ?></h2>
<?php if (isset($not_found)) echo '<h2>数据库中无此商品信息</h2>'; ?>
<p style="font-size:20px;">数据：<?=$data['content'] ?></p>
</body></html>
*/
