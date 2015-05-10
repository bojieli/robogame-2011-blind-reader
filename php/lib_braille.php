<?php
// module/blind/lib_braille.php 2011-08-27
$string = $_GET['str'];
$fp = fopen("/dev/blindbf", "w");
fprintf($fp, braille($string));
fclose($fp);

function braille($string) {
	static $number_prefix = '001111';
	static $numbers = array(
	'1' => '100000',
	'2' => '110000',
	'3' => '100100',
	'4' => '100110',
	'5' => '100010',
	'6' => '110100',
	'7' => '110110',
	'8' => '110010',
	'9' => '010100',
	'0' => '010110'
	);

	static $chars = array(
	'a' => '100000',
	'b' => '110000',
	'c' => '100100',
	'd' => '100110',
	'e' => '100010',
	'f' => '110100',
	'g' => '110110',
	'h' => '110010',
	'i' => '010100',
	'j' => '010110',
	'k' => '101000',
	'l' => '111000',
	'm' => '101100',
	'n' => '101110',
	'o' => '101010',
	'p' => '111100',
	'q' => '111110',
	'r' => '111010',
	's' => '011100',
	't' => '011110',
	'u' => '101001',
	'v' => '111001',
	'w' => '010111',
	'x' => '101101',
	'y' => '101111',
	'z' => '101011'
	);

	$len = strlen($string);
	$word = strtolower($string);
	$str = '';
	for ($i=0; $i<$len; $i++) {
		$c = $word[$i];
		if ($c >= '0' && $c <= '9') {
			$str .= $number_prefix.$numbers[$c];
		} elseif ($c >= 'a' && $c <= 'z') {
			$str .= $chars[$c];
		} else {
			$str .= '000000';
		}
	}
	
	$data = array();
	$xp = $yp = 0;
	$newlen = strlen($str);
	for ($i=0; $i<$newlen; $i+=6) {
		$data[$xp+0][$yp+0] = $str[$i+0];
		$data[$xp+1][$yp+0] = $str[$i+1];
		$data[$xp+2][$yp+0] = $str[$i+2];
		$data[$xp+0][$yp+1] = $str[$i+3];
		$data[$xp+1][$yp+1] = $str[$i+4];
		$data[$xp+2][$yp+1] = $str[$i+5];

		if ($yp + 2 < 20) {
			$yp += 2;
		} else {
			$xp += 4;
			$yp = 0;
		}
	}

	$content = '';
	$height = $xp + 3;
	for ($x=0; $x<$height; $x++) {
		for ($y=0; $y<20; $y++) {
			$content .= ((isset($data[$x][$y]) && $data[$x][$y] == '1') ? '1' : '0');
		}
	}

	return $content;
}
?>
