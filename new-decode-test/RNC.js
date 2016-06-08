'use strict';

/************************************************************
	GF16 variables
************************************************************/

var GF16memL = new Uint16Array(

var GF16memIdx = new Uint32Array(


/************************************************************
	GF16 functions
************************************************************/

// Multiplication
function
GF16mul(a, b)
{
	return GF16memL[GF16memIdx[a] + GF16memIdx[b]];
}

// Division
function
GF16div(a, b)
{
	return GF16memL[65535 + GF16memIdx[a] - GF16memIdx[b]];
}

/* Debug
a = 1;
b = 2;
console.log(GF16mul(a, b));
*/

/************************************************************
	Download and Decode
************************************************************/

// Decode
function
Decode(coef, fsize, esize, buf_in)
{
	var i, l, n;
	var decode_size, buf, buf_out;
	var c0, c1, c2, c3, c4, t0, t1, d;

	// Calculate decode size
	n = fsize % 6;
	decode_size = (n == 0) ? fsize : fsize + (6 - n);

	// Allocate buf_out
	buf = new ArrayBuffer(decode_size);
	buf_out = new Uint16Array(buf);

	// Prepare for decoding
        c0 = GF16mul(coef[1][0], coef[0][1]) ^ GF16mul(coef[0][0], coef[1][1]);
        c1 = GF16mul(coef[1][0], coef[0][2]) ^ GF16mul(coef[0][0], coef[1][2]);
	if (c0 == 0 && c1 == 0) {
		// Equations are not independent
		console.log("Error: Duplication in equations occurred. Can't decode.");
		return;
	}
	c2 = GF16mul(coef[2][0], coef[0][1]) ^ GF16mul(coef[0][0], coef[2][1]);
	c3 = GF16mul(coef[2][0], coef[0][2]) ^ GF16mul(coef[0][0], coef[2][2]);
	if (c2 == 0 && c3 == 0) {
		// Equations are not independent
		console.log("Error: Duplication in equations occurred. Can't decode.");
		return;
	}
	c4 = GF16mul(c1, c2) ^ GF16mul(c0, c3);
	if (c4 == 0) {
		// Equations are not independent
		console.log("Error: Duplication in equations occurred. Can't decode.");
		return;
	}

	// Decode
	n = esize / 2; // esize / sizeof(uint16_t)
	l = 0;
	for (i = 0; i < n; i++) {
		t0 = GF16mul(coef[1][0], buf_in[0][i]) ^
		     GF16mul(coef[0][0], buf_in[1][i]);
		t1 = GF16mul(coef[2][0], buf_in[0][i]) ^
		     GF16mul(coef[0][0], buf_in[2][i]);

		// Get X2
		d = GF16mul(c2, t0) ^ GF16mul(c0, t1);
		d = GF16div(d, c4);
		buf_out[l + 2] = d;

		// Get X1
		if (c0 == 0) {
			d = t1 ^ GF16mul(c3, d);
			d = GF16div(d, c2);
		} else {
			d = t0 ^ GF16mul(c1, d);
			d = GF16div(d, c0);
		}
		buf_out[l + 1] = d;

		// Get X0
		d = buf_in[0][i] ^ GF16mul(coef[0][1], d);
		d ^= GF16mul(coef[0][2], buf_out[l + 2]);
		buf_out[l] = GF16div(d, coef[0][0]);

		// Debug
		//console.log(buf_out[l] + " " + buf_out[l + 1] + " " + buf_out[l + 2]);

		l += 3;
	}

	console.log("Decoding done");

	return buf;
}

/*
// Use CORS to workaround XMLHttpRequest + Same Orgin Policy
function
createCORSRequest(method, url){
	var xhr = new XMLHttpRequest();

	if ("withCredentials" in xhr) {
		xhr.open(method, url, true);
	} else if (typeof XDomainRequest != "undefined") {
		xhr = new XDomainRequest({mozSystem: true});
		xhr.open(method, url);
	} else {
		xhr = null;
	}

	return xhr;
}
*/

// Download and decode
function
DownloadAndDecode(complete)
{
	var i, j, idx;
	var server = ["rnc02.asusa.net/encoded-files02", "rnc03.asusa.net/encoded-files03", "rnc04.asusa.net/encoded-files04"];
	//var file = "VfE_html5.mp4";
	var file = "chunk";

	var dh_coef, dh_fsize;
	var fsize, esize;
	var coef = new Array(3);
	var data = new Array(3);
	var data_received = 0;
	var decoded_data = null;

	// Allocate coef
	for (i = 0; i < 3; i++) {
		coef[i] = new Uint16Array(3);
	}

	// Set origin
	document.domain = 'asusa.net';

	// Get header and data from servers (asynchronous is complicated....)
	var xhr;
	for (i = 0; i < 3; i++) {
		// Get header first
		xhr = new XMLHttpRequest();
		xhr.open("GET", "http://" + server[i] + "/" + file, true);
		//xhr = createCORSRequest("GET", "http://" + server[i] + "/" + file);
		xhr.id = i; // Server ID
		xhr.setRequestHeader('Range', 'bytes=0-11');
		xhr.responseType = "arraybuffer";
		xhr.onload = function() {
			idx = this.id; // Server ID
			console.log("Got header from " + idx);
			dh_coef = new Uint16Array(this.response, 0, 3);
			dh_fsize = new Uint32Array(this.response, 8, 1);
			fsize = dh_fsize[0];
			for (j = 0; j < 3; j++) {
				coef[idx][j] = dh_coef[j];
				console.log("  Coef[" + j + "]: " + coef[idx][j]);
			}
			console.log("  File size: " + fsize);

			// OK, now get data
			xhr = new XMLHttpRequest();
			xhr.id = idx; // Server ID
			xhr.open("GET", "http://" + server[idx] + "/" + file, true);
			xhr.setRequestHeader('Range', 'bytes=12-');
			xhr.responseType = "arraybuffer";
			xhr.onload = function() {
				idx = this.id; // Server ID
				esize = this.response.byteLength; // Enc data size
				console.log("Got data from " + idx);
				console.log("  Data size:  " + esize);

				// Map data
				data[idx] = new Uint16Array(this.response);

				// Check if all data were received
				data_received++;
				if (data_received == 3) {
					// Decode
					console.log("Data all received, start decoding");
					decoded_data = Decode(coef, fsize, esize, data);

					// Playback decoded data
					complete(decoded_data);

					return decoded_data;
				}
			}
			xhr.send(null);
		}
		xhr.send(null);
	}
}

// This is a replacement of the one in stream.js
var Stream = (function stream() {
	function constructor(url) {
	}
  
	constructor.prototype = {
		readAll: function(progress, complete) {
			// Download and decode
			DownloadAndDecode(complete);
		}
	};

	return constructor;
})();
