module AdHoc {
	module Net {
		["cpp:ice_print"]
		exception CurlException {
			int resultcode;
			string message;
			optional(1) short httpcode;
		};
	};
};

