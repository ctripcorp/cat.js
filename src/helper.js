
function extract_type(err){
	var type = "Error";
	if(err.code){
		type = err.code; // system error
	}
	/* above each is javascript error */
	else if(err instanceof RangeError){
		type = "RangeError"; 
	}else if(err instanceof TypeError){
		type = "TypeError";
	}else if(err instanceof ReferenceError){
		type = "ReferenceError";
	}else if(err instanceof SyntaxError){
		type = "SyntaxError";
	}

	return type;
}

module.exports.extract_type=extract_type;