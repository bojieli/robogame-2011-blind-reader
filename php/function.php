<?php
// module/blind/function.php 2011-08-27

$function = isset($_POST['content']) ? $_POST['content'] : $_GET['content'];

$len = strlen($function);
for ($i=0; $i<$len; $i++) {
	$c = $function[$i];
	if (($c < 'a' || $c > 'z') && ($c < '0' || $c > '9') && !in_array($c, array('+', '-', '*', '/', '(', ')'))) {
		break;
	}
}
if ($i<$len) {
	$function = '0';
}

if (empty($function)) {
	$function = '0';
}

$rows = 15;
$cols = 20;
$fromx = -9;
$tox = 10;

$y = array();
$max = -INFINITY;
$min = INFINITY;
for ($x=$fromx; $x<=$tox; $x++) {
	$exp = 'return '.str_replace('x', '($x)', $function).';';
	$y[$x] = @eval($exp);
	if ($y[$x] < $min)
		$min = $y[$x];
	if ($y[$x] > $max)
		$max = $y[$x];
}

if ($min > 0)
	$min = 0;
if ($max < 0)
	$max = 0;

$height = $max - $min;
if ($height == 0)
	$height = 1;

$data = array();
$pos = array();
for ($x=$fromx; $x<=$tox; $x++) {
	$thispos = intval(($max - $y[$x]) * $rows / $height);
	$pos[$x] = $thispos;
	$data[$thispos][$x] = 1;
}
for ($x=$fromx+1; $x<=$tox; $x++) {
	if ($pos[$x] - $pos[$x-1] > 1) {
		$mid = intval(($pos[$x - 1] + $pos[$x]) / 2);
		for ($y=$pos[$x-1]+1; $y<=$mid; $y++) {
			$data[$y][$x-1] = 1;
		}
		for ($y=$mid+1; $y<$pos[$x]; $y++) {
			$data[$y][$x] = 1;
		}
	}
	elseif ($pos[$x-1] - $pos[$x] > 1) {
		$mid = intval(($pos[$x-1] + $pos[$x])/2);
		for ($y=$pos[$x]+1; $y<=$mid; $y++) {
			$data[$y][$x] = 1;
		}
		for ($y=$mid+1; $y<$pos[$x-1]; $y++) {
			$data[$y][$x-1] = 1;
		}
	}
}


$xaxis = intval($max * $rows / $height);
if ($xaxis >= $rows) {
	$xaxis = $rows - 1;
}
for ($x=$fromx; $x<=$tox; $x++) {
	$data[$xaxis][$x] = 1;
}
$data[$xaxis][0] = 0;

/*
for ($y=0; $y<$cols; $y++) {
	$data[$y][0] = 1;
}
*/

$str = '';
for ($x=0; $x<$rows; $x++)
	for ($y=$fromx; $y<=$tox; $y++)
		$str .= !empty($data[$x][$y]) ? '1' : '0';

$fp = fopen("/dev/blindbf", "w");
fwrite($fp, $str);
fclose($fp);
?>
