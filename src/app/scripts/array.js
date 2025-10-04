const array = (length) => {
	return Array(length)
		.fill(null)
		.map((__, ind) => ind);
};
