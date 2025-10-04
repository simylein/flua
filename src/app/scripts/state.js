const loading = (element, add, remove, text, height, width) => {
	element.classList.add(...add, 'background-neutral-200', 'dark:background-neutral-800');
	element.classList.remove('background-red-200', 'dark:background-red-800', ...remove);
	if (text !== undefined && text !== null) {
		element.innerText = text;
	}
	if (height !== undefined && height !== null) {
		element.style.height = `${height}%`;
	}
	if (width !== undefined && width !== null) {
		element.style.width = `${width}%`;
	}
};

const paint = (element, add, remove, text, height, width) => {
	element.classList.add(...add);
	element.classList.remove('background-neutral-200', 'dark:background-neutral-800', ...remove);
	if (text !== undefined && text !== null) {
		element.innerText = text;
	}
	if (height !== undefined && height !== null) {
		element.style.height = `${height}%`;
	}
	if (width !== undefined && width !== null) {
		element.style.width = `${width}%`;
	}
};

const error = (element) => {
	element.classList.add('background-red-200', 'dark:background-red-800');
	element.classList.remove('background-neutral-200', 'dark:background-neutral-800');
};
