const report = (err) => {
	if (err instanceof TypeError) {
		return notification('info', 'Seems like you might be offline');
	}
	const message = `Request failed with ${err.status ?? 0} ${err.statusText ?? null}`;
	if (err.status < 500) {
		return notification('warning', message);
	}
	return notification('error', message);
};
