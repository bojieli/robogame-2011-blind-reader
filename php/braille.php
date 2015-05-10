<?php
// module/blind/braille.php 2011-08-27

if (!defined('IN_GEWU')) {
	exit(header('HTTP/1.1 403 Forbidden'));
}

if (!empty($_POST['content'])) {
	require_once modulefile('blind', 'lib_braille');
	$content = braille($_POST['content']);
	OBJ::update(CURRENT_PATH, array('content' => $content));
}
redirect(CURRENT_PATH);
?>
