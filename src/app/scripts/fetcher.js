const fetcher = async (method, pathname, body, options) => {
	if (window.localStorage.getItem('develop')) {
		await new Promise((resolve) => setTimeout(resolve, 400 + Math.random() * 800));
		if (Math.random() < 0.1) {
			throw { status: Math.floor(Math.random() * 200) + 400, statusText: 'Develop' };
		}
	}
	const response = await fetch(pathname, { method, body, headers: options?.headers, signal: options?.controller?.signal });
	if (response.status < 200 || response.status > 299) {
		throw response;
	}
	return response;
};
