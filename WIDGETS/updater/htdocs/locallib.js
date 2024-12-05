'use strict';

var LocalLib = {};

/**
 * Create a pre-formatted hex dump of an input array (like a byte buffer)
 * Sample Usage:
 * var l = new Uint8Array([ 0xca, 0xfe, 0xba, 0xbe, '<'.charCodeAt(0), '>'.charCodeAt(0), '&'.charCodeAt(0) ]);
 * var res = LocalLib.hexdump(l);
 * // then insert res into a <pre> element (logging will show some escaped html
 * // codes if they are present in the input buffer).
 * console.log(res);
 */

LocalLib.hexdump = function(ary) {
	var pos = 0;
	var output = '';
	var printable = '';
	var column = 0;
	for (pos = 0; pos < ary.length; pos++) {
		column = pos % 16;
		if (column == 0) {
			output += '  ' + printable + "\r\n";
			printable = '';
		} else
		if (pos % 8 == 0) {
			output += '- ';
		}
		if (ary[pos] >= 0x20 && ary[pos] < 0x7f) {
			var c = String.fromCharCode(ary[pos]);
			if (c == '&') {
				printable += '&amp;';
			} else
			if (c == '<') {
				printable += '&lt;';
			} else
			if (c == '>') {
				printable += '&gt;';
			} else
			if (c == '"') {
				printable += '&quot;';
			} else
			if (c == "'") {
				printable += '&#039;';
			} else {
				printable += c;
			}
		} else { printable += '.'; }
		if (ary[pos] <= 0xf) { output += '0'; }
		output += ary[pos].toString(16) + " ";
	}
	if (printable) {
		for (var i = 1; i < (16 - column); i++) {
			output += '   ';
		}
		if (column <= 7) { output += '  '; }
		output += '  ' + printable + "\r\n";
	}
	return output;
}

