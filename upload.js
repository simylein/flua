const upload = document.getElementById('upload');

const readFile = (file) => {
	return new Promise((resolve) => {
		const reader = new FileReader();
		reader.onload = (event) => resolve(event.target.result);
		reader.readAsText(file);
	});
};

const hashFile = async (content) => {
	const data = new TextEncoder().encode(content);
	const buffer = await crypto.subtle.digest('sha-256', data);
	const array = Array.from(new Uint8Array(buffer));
	const hex = array.map((byte) => byte.toString(16).padStart(2, '0'));
	return hex.join('');
};

const postFlight = async (hash, startsAt, endsAt) => {
	const data = new FormData();
	data.append('hash', hash.substring(0, 32));
	data.append('starts_at', startsAt);
	data.append('ends_at', endsAt);
	try {
		if (window.location.origin.startsWith('http://localhost')) {
			await new Promise((resolve) => setTimeout(resolve, 400 + Math.random() * 800));
			if (Math.random() < 0.1) {
				throw Error('explicit develop failure');
			}
		}
		const response = await fetch('/api/flight', { method: 'post', body: data });
		if (response.status >= 200 && response.status <= 299) {
			notification('success', `Successfully uploaded flight ${hash.substring(0, 8)}`);
			return loadFlights();
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
};

const uploadFlights = async (event) => {
	const store = upload.children[0].innerText;
	upload.children[0].innerText = 'uploading...';
	upload.children[0].classList.add('cursor-not-allowed');
	upload.children[1].disabled = true;
	const files = Array.from(event.target.files);
	for (const file of files) {
		const content = await readFile(file);
		const hash = await hashFile(content);
		const data = parseIgcFile(content);
		await postFlight(hash, data[0].time, data[data.length - 1].time);
	}
	upload.children[0].innerText = store;
	upload.children[0].classList.remove('cursor-not-allowed');
	upload.children[1].disabled = false;
};

const parseIgcFile = (content) => {
	const lines = content.split('\n').map((line) => line.substring(0, line.length - 1));
	const head = lines.filter((line) => line.startsWith('HF'));
	const body = lines.filter((line) => line.startsWith('B'));

	let date = null;
	const raw = head.find((line) => line.match(/^HFDTE\d{2}([0-5][0-9]){2}$/));
	if (raw) {
		date = `20${raw[9]}${raw[10]}-${raw[7]}${raw[8]}-${raw[5]}${raw[6]}`;
	} else {
		notification('warning', 'Could not extract date');
		date = '1970-01-01';
	}

	const data = [];
	body.map((line) => {
		if (line.match(/^B\d{6}\d{7}(N|S)\d{8}(E|W)A\d{10}$/)) {
			const time = `${line[1]}${line[2]}:${line[3]}${line[4]}:${line[5]}${line[6]}`;
			const lat = line[14] === 'N' ? +line.substring(7, 14) : -line.substring(7, 14);
			const lon = line[23] === 'E' ? +line.substring(15, 23) : -line.substring(15, 23);
			const alt = +line.substring(25, 31) / 10;
			data.push({ time: Math.floor(new Date(`${date}T${time}Z`).getTime() / 1000), lat, lon, alt });
		}
	});
	return data;
};
