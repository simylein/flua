const formik = (elements, schema, submit) => {
	const backgrounds = [];
	const [values, errors, touched] = [{}, {}, {}];

	const validate = (all) => {
		if (all) Object.keys(schema).forEach((key) => (touched[key] = true));
		Object.keys(touched).forEach((key) => {
			if (touched[key]) errors[key] = schema[key](values[key]);
		});
		Object.keys(errors).forEach((key) => {
			Object.keys(elements)
				.filter((element) => element === key)
				.forEach((element) => {
					if (errors[key]) {
						elements[element].innerText = errors[key];
						elements[element].classList.remove('hidden');
					} else {
						elements[element].innerText = '';
						elements[element].classList.add('hidden');
					}
				});
		});
		const valid = Object.keys(errors).every((key) => !errors[key]);
		if (valid) {
			elements.button.disabled = false;
			elements.button.classList.remove('background-neutral-300', 'dark:background-neutral-700', 'cursor-not-allowed');
			while (backgrounds.length > 0) {
				const cls = backgrounds.pop();
				elements.button.classList.add(cls);
			}
			elements.button.classList.add('cursor-pointer');
		} else if (all) {
			elements.button.disabled = true;
			for (let ind = elements.button.classList.length - 1; ind >= 0; ind--) {
				const cls = elements.button.classList[ind];
				if (cls.startsWith('background-') || cls.startsWith('dark:background-')) {
					backgrounds.push(cls);
					elements.button.classList.remove(cls);
				}
			}
			elements.button.classList.remove('cursor-pointer');
			elements.button.classList.add('background-neutral-300', 'dark:background-neutral-700', 'cursor-not-allowed');
		}
		return valid;
	};

	const setValue = (key, value) => {
		values[key] = value;
		validate();
	};

	const setTouched = (key, value) => {
		touched[key] = value;
		validate();
	};

	const handleSubmit = async (event) => {
		event.preventDefault();
		if (!validate(true)) return;
		const store = elements.button.innerText;
		elements.button.innerText = 'loading...';
		elements.button.disabled = true;
		elements.button.classList.add('cursor-progress');
		elements.button.classList.remove('cursor-pointer');
		await submit(values);
		elements.button.innerText = store;
		elements.button.disabled = false;
		elements.button.classList.add('cursor-pointer');
		elements.button.classList.remove('cursor-progress');
	};

	const reset = () => {
		Object.keys(values).forEach((key) => delete values[key]);
		Object.keys(errors).forEach((key) => delete errors[key]);
		Object.keys(touched).forEach((key) => delete touched[key]);
	};

	return { values, errors, touched, validate, setValue, setTouched, handleSubmit, reset };
};
