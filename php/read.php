<?php
// module/blind/read.php 2011-08-27

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
<h1 align="center">格物者盲读器演示</h1>
<div style="height:550px;width:680px;float:left">
<iframe id="screen" src="<?=CURRENT_PATH ?>?action=screen" height="90%" width="100%" /></iframe>
<div><button class="large" onclick="goup()">向上</button>&nbsp;&nbsp;<button class="large" onclick="godown()">向下</button>&nbsp;<button class="large" onclick="pageup()">PageUp</button>&nbsp;&nbsp;<button class="large" onclick="pagedown()">PageDown</button></div>
<input id="position_top" type="hidden" value="0" />
</div>
<div style="height:550px;width:280px;float:right">
<br />
<h2>显示数字</h2>
<form action="<?=CURRENT_PATH ?>?action=char" method="post">
<textarea name="content" style="width:280px;height:100px;"></textarea>
<button type="submit">提交</button>
</form>
<br />
<h2>显示盲文</h2>
<form action="<?=CURRENT_PATH ?>?action=braille" method="post">
<textarea name="content" style="width:280px;height:100px;"></textarea>
<button type="submit">提交</button>
</form>
<br />
<h2>显示函数图像</h2>
<form action="<?=CURRENT_PATH ?>?action=function" method="post">
<input name="content" />
<button type="submit">提交</button>
</form>
<p>绘图区间：[-9, 10]</p>
<p>&nbsp;</p>
<a href="<?=CURRENT_PATH ?>?action=recognize"><button class="large">条形码识别系统</button></a>
</div>
<?php require modulefile('common', 'footer'); ?>
