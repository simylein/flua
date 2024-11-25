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

const color = (type) => {
	switch (type) {
		case 'info':
			return ['bg-blue-200', 'dark:bg-blue-800'];
		case 'success':
			return ['bg-green-200', 'dark:bg-green-800'];
		case 'warning':
			return ['bg-amber-200', 'dark:bg-amber-800'];
		case 'error':
			return ['bg-red-200', 'dark:bg-red-800'];
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
	element.classList.add('p-4', ...color(type), 'z-10');
	const head = document.createElement('h2');
	head.classList.add('m-0');
	const text = document.createElement('p');
	text.classList.add('m-0');
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
