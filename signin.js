const { setValue, setTouched, handleSubmit } = formik(
	{
		button: document.getElementById('signin-button'),
		username: document.getElementById('username-validation'),
		password: document.getElementById('password-validation'),
	},
	{
		username: (value) => {
			if (!value) return 'username is required';
			if (value.length < 4) return 'username is too short';
			if (value.length > 16) return 'username is too long';
		},
		password: (value) => {
			if (!value) return 'password is required';
			if (value.length < 8) return 'password is too short';
			if (value.length > 64) return 'password is too long';
		},
	},
	async (values) => {
		const data = new FormData();
		data.append('username', values.username);
		data.append('password', values.password);
		try {
			if (window.location.origin.startsWith('http://localhost')) {
				await new Promise((resolve) => setTimeout(resolve, 400 + Math.random() * 800));
				if (Math.random() < 0.1) {
					throw Error('explicit develop failure');
				}
			}
			const response = await fetch('/api/signin', { method: 'post', body: data });
			if (response.status >= 200 && response.status <= 299) {
				notification('success', `Successfully signed in as ${values.username}`);
				window.location.href = `/${values.username}`;
				return;
			}
			throw response;
		} catch (err) {
			if (err instanceof TypeError) {
				return notification('info', 'Seems like you might be offline');
			}
			const message = `Request failed with ${err.status ?? 0} ${err.statusText ?? null}`;
			if (err.status < 500) {
				return notification('warning', message);
			}
			return notification('error', message);
		}
	}
);
