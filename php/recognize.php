<?php
// module/blind/recognize.php 2011-08-27

if (!defined('IN_GEWU')) {
	exit(header('HTTP/1.1 403 Forbidden'));
}

require modulefile('common', 'header');
?>
<script>
function $(id) {
	return !id ? null : document.getElementById(id);
}

function goup() {
	if ($('position_top').value > 0) {
		$('position_top').value--;
	}
	position_onchange();
}
function godown() {
	$('position_top').value++;
	position_onchange();
}
function pageup() {
	if ($('position_top').value > 15) {
		$('position_top').value -= 15;
	} else {
		$('position_top').value = 0;
	}
	position_onchange();
}
function pagedown() {
	$('position_top').value = parseInt($('position_top').value) + 15;
	position_onchange();
}
function position_onchange() {
	$('screen').src = "<?=CURRENT_PATH ?>?action=screen&top=" + $('position_top').value;
}
</script>
<h1 align="center">格物者条形码识别系统演示</h1>
<div style="height:550px;width:680px;float:left">
<iframe id="screen" src="<?=CURRENT_PATH ?>?action=screen" height="90%" width="100%" /></iframe>
<div><button class="large" onclick="goup()">向上</button>&nbsp;&nbsp;<button class="large" onclick="godown()">向下</button>&nbsp;<button class="large" onclick="pageup()">PageUp</button>&nbsp;&nbsp;<button class="large" onclick="pagedown()">PageDown</button></div>
<input id="position_top" type="hidden" value="0" />
</div>
<div style="height:550px;width:280px;float:right">
<iframe id="aside" src="<?=CURRENT_PATH ?>?action=barcode" height="90%" width="100%" /></iframe>
<a href="<?=CURRENT_PATH ?>?action=read"><button class="large">盲读器演示</button></a>
</div>
<?php require modulefile('common', 'footer'); ?>
