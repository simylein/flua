const notifications = document.getElementById('notifications');

const kind = (type) => {
	switch (type) {
		case 'info':
			return 'Info';
		case 'success':
			return 'Success';
		case 'warning':
			return 'Warning';
		case 'error':
			return 'Error';
	}
};

const color = (element, type) => {
	switch (type) {
		case 'info':
			return element.classList.add('background-blue-200', 'dark:background-blue-800');
		case 'success':
			return element.classList.add('background-green-200', 'dark:background-green-800');
		case 'warning':
			return element.classList.add('background-amber-200', 'dark:background-amber-800');
		case 'error':
			return element.classList.add('background-red-200', 'dark:background-red-800');
	}
};

const duration = (type) => {
	switch (type) {
		case 'info':
			return 4000;
		case 'success':
			return 3000;
		case 'warning':
			return 6000;
		case 'error':
			return 8000;
	}
};

const notification = (type, message) => {
	const element = document.createElement('div');
	element.classList.add('p-4');
	color(element, type);
	const head = document.createElement('h2');
	head.classList.add('m-0', 'mb-1', 'font-semibold');
	const text = document.createElement('p');
	text.classList.add('m-0', 'font-normal');
	element.appendChild(head);
	element.appendChild(text);
	element.children[0].innerText = kind(type);
	element.children[1].innerText = message;
	if (notifications.children.length > 0) {
		notifications.children[0].remove();
	}
	notifications.appendChild(element);
	setTimeout(() => element.remove(), duration(type));
};
