<?php
include "db.php";
$rows = 15;
$cols = 20;
$bytes = 38;

$fp = fopen("/dev/blindfb", "rb");
if ($fp) {
	$data = fread($fp, $bytes);
} else {
	echo 'Error';
}
fclose($fp);

$table = array();
for ($i=0; $i<$rows; $i++) {
	for ($j=0; $j<$cols; $j++) {
		$offset = $i * $cols + $j;
		//if ($offset % 8 == 0)
		//	printf("%02x", ord($data[intval($offset/8)]));
		$table[$i][$j] = (intval(ord($data[intval($offset / 8)]) & (1<<($offset % 8))) != 0);
	}
}
?>
<head>
<meta http-equiv="refresh" content="1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<style>
.blind { border-collapse: collapse; }
.blind td { height: 18px; width: 18px; border: 2px solid #CDCDCD; }
.blind .on { background: #000; }
.blind .off { background: #FFF; }
</style>
</head>
<body>
<table class="blind">
<?php 
echo '<tr><th></th>';
for ($j=1; $j<=$cols; $j++) {
	echo "<th>$j</th>";
}
echo '</tr>';

for ($i=0; $i<$rows; $i++) {
	echo '<tr>';
	echo '<th>'.($i + 1).'</th>';
	for ($j=0; $j<$cols; $j++) {
		echo '<td class="';
		echo ($table[$i][$j] == 0) ? 'off' : 'on';
		echo '">&nbsp;</td>';
	}
	echo "</tr>\n";
} ?>
</table>
</body>
