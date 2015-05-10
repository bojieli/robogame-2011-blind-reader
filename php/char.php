<?php
// module/blind/char.php 2011-08-27

if (!defined('IN_GEWU')) {
	exit(header('HTTP/1.1 403 Forbidden'));
}

$chars = array(
	'0' => 
"
1111
1  1
1  1
1  1
1111",
	'1' =>
"
  1 
 11 
  1 
  1 
 111",
 	'2' =>
"
1111
   1
1111
1   
1111",
	'3' =>
"
1111
   1
1111
   1
1111",
	'4' =>
"
  1 
 11 
1 1 
1111
  1 ",
	'5' =>
"
1111
1   
1111
   1
1111",
	'6' =>
"
1111
1   
1111
1  1
1111",
	'7' =>
"
1111
   1
   1
   1
   1",
   	'8' =>
"
1111
1  1
1111
1  1
1111",
	'9' =>
"
1111
1  1
1111
   1
1111"
	);

while (list($index, $char) = each($chars)) {
	$chars[$index] = str_replace("\n", '', $char);
}

if (!empty($_POST['content'])) {
	$len = strlen($_POST['content']);
	$word = $_POST['content'];
	$data = array();
	for ($i=0; $i<$len; $i++) {
		$str = $chars[$word[$i]];
		$xt = (intval($i / 4)) * 6;
		$yt = ($i % 4) * 5;
		for ($x=0; $x<5; $x++) {
			for ($y=0; $y<4; $y++) {
				$data[$xt + $x][$yt + $y] = $str[$x*4+$y];
			}
		}
	}
	$content = '';
	$height = (intval($len / 4) + 1) * 6;
	for ($x=0; $x<$height; $x++) {
		for ($y=0; $y<20; $y++) {
			$content .= ((isset($data[$x][$y]) && $data[$x][$y] == '1') ? '1' : '0');
		}
	}
	OBJ::update(CURRENT_PATH, array('content' => $content));
}
redirect(CURRENT_PATH);
?>
