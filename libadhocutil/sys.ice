module AdHoc {
	["cpp:ice_print"]
	exception SystemException {
		string task;
		string message;
		int errNo;
	};

	["cpp:ice_print"]
	exception SystemExceptionOn extends SystemException {
		string objectName;
	};
};

