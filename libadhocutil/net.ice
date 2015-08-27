module AdHoc {
	module Net {
		exception CurlException {
			int resultcode;
			string message;
			optional(1) short httpcode;
		};
	};
};

