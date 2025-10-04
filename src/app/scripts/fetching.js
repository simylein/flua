const fetching = (context, loading, fetch, parse, paint, error) => {
	const load = async (silent) => {
		if (context.timeout) {
			clearTimeout(context.timeout);
			context.timeout = null;
		}
		if (context.controller) {
			context.controller.abort();
		}
		context.controller = new AbortController();
		if (!silent) {
			loading(context);
		}
		try {
			const response = await fetch(context);
			await parse(context, response);
			context.retries = 0;
			paint(context);
			if (context.reload !== null) {
				context.timeout = setTimeout(() => load(true), context.reload * 1000);
			}
		} catch (err) {
			if (err?.name !== 'AbortError') {
				error(context);
				if (context.retries < 8) {
					context.timeout = setTimeout(() => {
						context.retries++;
						load();
					}, 2 ** context.retries * 1000);
				}
			}
		}
	};
	return load;
};
