<?php
$query = isset($_GET['s']) ? $_GET['s'] : '';
//switch($query) {
	if($query == 'physics') 
	{
	if (!isset($_GET['k'])) {
		echo "Wikipedia\nphysics.org | Home\nThe Physics Classroom\n";
	} else {
		switch($_GET['k']) {
			case 1: echo "Physics is a natural science that involves the study of matter and its motion through spacetime, along with related ...\n"; break;
			case 2: echo "Your guide to physics on the web. physics.org is the place to be if you have a burning physics question, or if you just want to browse articles and interactive ...\n"; break;
			case 3: echo "The Physics Classroom is an online interactive tutorial of basic physics concepts. The lessons use an easy-to-understand language to present common physics ...\n"; break;
		}
	}
	}
	else if ($query == 'nike')
	{
	if (!isset($_GET['k'])) {
		echo "NIKE - JUST DO IT\nNikeStore\nNike,Inc-Wikipedia\n";
	} else {
		switch ($_GET['k']) {
			case 1: echo "Experience sports, training, shopping and everything else that's new at Nike."; break;
			case 2: echo "NikeStore. Shop the Official Nike Store for Shoes, Clothing & Gear"; break;
			case 3: echo "Origins and history,Products,Headquarters,Manufacturing.Nike uses web sites as a promotional tool to cover these events.\n"; break;
		}
	}
	}
	else // braille
	{
	if (!isset($_GET['k'])) {
		echo "Braille - Wikipedia\nBraille alphabet\nBraille History\n";
	} else {
		switch ($_GET['k']) {
			case 1: echo "The Braille system is a method that is widely used by blind people to read and write, and was the first digital form of writing."; break;
			case 2: echo "It consists of patterns of raised dots arranged in cells of up to six dots in a 3 x 2 configuration. Each cell represents a letter, numeral or punctuation mark."; break;
			case 3: echo "Braille was invented by a teenager, battled by the establishment, and went on to become a worldwide communications phenomenon in a tale too improbable to be fiction."; break;
		}
	}
	}
