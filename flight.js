const skeletons = [4, 8, 17, 24, 42, 69, 82, 73, 56, 37, 16, 8];

const years = document.getElementById('years');

const flightCount = document.getElementById('flight-count');
const flightScale = document.getElementById('flight-scale');
const flightGraph = document.getElementById('flight-graph');

const airtimeCount = document.getElementById('airtime-count');
const airtimeScale = document.getElementById('airtime-scale');
const airtimeGraph = document.getElementById('airtime-graph');

const array = (length) => {
	return Array(length)
		.fill(null)
		.map((__, ind) => ind);
};

const units = (amount, title) => {
	switch (true) {
		case amount >= 1e4:
			return `${(amount / 1e3).toFixed(1)}k`;
		case amount >= 1e3:
			return `${(amount / 1e3).toFixed(2)}k`;
		case amount >= 1e1:
			return amount.toFixed(0);
		default:
			return amount.toFixed(title ? 0 : 1);
	}
};

const times = (seconds) => {
	switch (true) {
		case seconds >= 36000:
			return `${Math.floor(seconds / 3600)}h`;
		case seconds >= 3600:
			return `${(seconds / 3600).toFixed(1)}h`;
		case seconds >= 600:
			return `${Math.floor(seconds / 60)}m`;
		case seconds >= 60:
			return `${(seconds / 60).toFixed(1)}m`;
		default:
			return `${seconds}s`;
	}
};

const buckets = (items, max) => {
	return items.reduce(
		(accumulator, item) => {
			const month = new Date(Number(item.startsAt) * 1000).getMonth();
			accumulator[month].push(item);
			return accumulator;
		},
		array(max).map(() => [])
	);
};

const seconds = (item) => {
	return Number(item.endsAt - item.startsAt);
};

const skeleton = (element, length) => {
	let index = 0;
	while (element.children.length - length) {
		const diff = element.children.length - length;
		if (diff < 0) {
			const copy = element.children[index % 2].cloneNode(true);
			element.appendChild(copy);
		} else if (diff > 0) {
			element.children[element.children.length - 1].remove();
		}
		index++;
	}
};

const getYear = () => {
	const year = new URLSearchParams(window.location.search).get('year');
	return +(year ?? new Date().getFullYear());
};

const setYear = (year) => {
	const params = new URLSearchParams(window.location.search);
	params.set('year', year);
	window.history.replaceState({}, '', `${window.location.pathname}?${params.toString()}`);
};

const highlightYear = (year) => {
	array(years.children.length).forEach((ind) => {
		if (+years.children[ind].children[0].innerText === year) {
			years.children[ind].children[0].classList.add('text-blue-600', 'dark:text-blue-400');
		} else {
			years.children[ind].children[0].classList.remove('text-blue-600', 'dark:text-blue-400');
		}
	});
};

const loadingYears = () => {
	skeleton(years, years.children.length);
	array(years.children.length).forEach((ind) => {
		years.children[ind].classList.remove('cursor-pointer');
		years.children[ind].classList.add('cursor-default');
		years.children[ind].children[0].innerText = '';
		years.children[ind].onclick = null;
		years.children[ind].children[0].classList.add('w-9', 'h-6', 'bg-neutral-200', 'dark:bg-neutral-800');
	});
};

const paintYears = (data) => {
	skeleton(years, data.length);
	array(years.children.length).forEach((ind) => {
		years.children[ind].classList.remove('cursor-default');
		years.children[ind].classList.add('cursor-pointer');
		years.children[ind].children[0].classList.remove('w-9', 'h-6', 'bg-neutral-200', 'dark:bg-neutral-800');
		years.children[ind].children[0].innerText = data[ind].year;
		years.children[ind].onclick = () => {
			highlightYear(data[ind].year);
			setYear(data[ind].year);
			loadFlights();
		};
	});
	highlightYear(getYear());
};

const erroredYears = () => {
	array(years.children.length).forEach((ind) => {
		years.children[ind].classList.remove('bg-neutral-100', 'dark:bg-neutral-900', 'cursor-default');
		years.children[ind].children[0].classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
		years.children[ind].classList.add('bg-red-200', 'dark:bg-red-800', 'cursor-not-allowed');
	});
};

const loadingFlights = () => {
	flightCount.innerText = '';
	flightCount.classList.remove('bg-red-200', 'dark:bg-red-800');
	flightCount.classList.add('w-10', 'h-6', 'bg-neutral-200', 'dark:bg-neutral-800');
	array(flightScale.children.length).forEach((ind) => {
		flightScale.children[ind].innerText = '';
		flightScale.children[ind].classList.remove('bg-red-200', 'dark:bg-red-800');
		flightScale.children[ind].classList.add('h-4', 'bg-neutral-200', 'dark:bg-neutral-800');
	});
	array(flightGraph.children.length).forEach((ind) => {
		flightGraph.children[ind].children[0].classList.remove('bg-red-200', 'dark:bg-red-800', 'bg-green-400/80', 'dark:bg-green-600/80');
		flightGraph.children[ind].children[0].classList.add('bg-neutral-200', 'dark:bg-neutral-800');
		flightGraph.children[ind].children[0].style.height = `${skeletons[ind]}%`;
	});
};

const paintFlights = (data) => {
	flightCount.classList.remove('w-10', 'h-6', 'bg-neutral-200', 'dark:bg-neutral-800');
	flightCount.innerText = units(data.length, true);

	const windows = buckets(data, flightGraph.children.length);
	const max = Math.max(...windows.map((window) => window.length));

	array(flightScale.children.length).forEach((ind) => {
		flightScale.children[ind].classList.remove('h-4', 'bg-neutral-200', 'dark:bg-neutral-800');
		flightScale.children[ind].innerText = units((max / flightScale.children.length) * (5 - ind));
	});

	windows.forEach((window, index) => {
		array(flightGraph.children[index].children.length).forEach((ind) => {
			flightGraph.children[index].children[ind].style.height = null;
		});
		const percent = (window.length / max) * 100;
		flightGraph.children[index].children[0].style.height = `${percent}%`;
		flightGraph.children[index].children[0].classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
		flightGraph.children[index].children[0].classList.add('bg-green-400/80', 'dark:bg-green-600/80');
	});
};

const erroredFlights = () => {
	flightCount.classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
	flightCount.classList.add('bg-red-200', 'dark:bg-red-800');
	array(flightScale.children.length).forEach((ind) => {
		flightScale.children[ind].classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
		flightScale.children[ind].classList.add('bg-red-200', 'dark:bg-red-800');
	});
	array(flightGraph.children.length).forEach((ind) => {
		flightGraph.children[ind].children[0].classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
		flightGraph.children[ind].children[0].classList.add('bg-red-200', 'dark:bg-red-800');
	});
};

const loadingAirtime = () => {
	airtimeCount.innerText = '';
	airtimeCount.classList.remove('bg-red-200', 'dark:bg-red-800');
	airtimeCount.classList.add('w-10', 'h-6', 'bg-neutral-200', 'dark:bg-neutral-800');
	array(airtimeScale.children.length).forEach((ind) => {
		airtimeScale.children[ind].innerText = '';
		airtimeScale.children[ind].classList.remove('bg-red-200', 'dark:bg-red-800');
		airtimeScale.children[ind].classList.add('h-4', 'bg-neutral-200', 'dark:bg-neutral-800');
	});
	array(airtimeGraph.children.length).forEach((ind) => {
		airtimeGraph.children[ind].children[0].classList.remove('bg-red-200', 'dark:bg-red-800', 'bg-blue-400/80', 'dark:bg-blue-600/80');
		airtimeGraph.children[ind].children[0].classList.add('bg-neutral-200', 'dark:bg-neutral-800');
		airtimeGraph.children[ind].children[0].style.height = `${skeletons[ind]}%`;
	});
};

const paintAirtime = (data) => {
	airtimeCount.classList.remove('w-10', 'h-6', 'bg-neutral-200', 'dark:bg-neutral-800');
	airtimeCount.innerText = times(
		data.reduce((accumulator, item) => accumulator + seconds(item), 0),
		0
	);

	const windows = buckets(data, airtimeGraph.children.length);
	const max = Math.max(...windows.map((window) => window.reduce((accumulator, item) => accumulator + seconds(item), 0)));

	array(airtimeScale.children.length).forEach((ind) => {
		airtimeScale.children[ind].classList.remove('h-4', 'bg-neutral-200', 'dark:bg-neutral-800');
		airtimeScale.children[ind].innerText = times((max / airtimeScale.children.length) * (5 - ind));
	});

	windows.forEach((window, index) => {
		array(airtimeGraph.children[index].children.length).forEach((ind) => {
			airtimeGraph.children[index].children[ind].style.height = null;
		});
		const percent = (window.reduce((accumulator, item) => accumulator + seconds(item), 0) / max) * 100;
		airtimeGraph.children[index].children[0].style.height = `${percent}%`;
		airtimeGraph.children[index].children[0].classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
		airtimeGraph.children[index].children[0].classList.add('bg-blue-400/80', 'dark:bg-blue-600/80');
	});
};

const erroredAirtime = () => {
	airtimeCount.classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
	airtimeCount.classList.add('bg-red-200', 'dark:bg-red-800');
	array(airtimeScale.children.length).forEach((ind) => {
		airtimeScale.children[ind].classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
		airtimeScale.children[ind].classList.add('bg-red-200', 'dark:bg-red-800');
	});
	array(airtimeGraph.children.length).forEach((ind) => {
		airtimeGraph.children[ind].children[0].classList.remove('bg-neutral-200', 'dark:bg-neutral-800');
		airtimeGraph.children[ind].children[0].classList.add('bg-red-200', 'dark:bg-red-800');
	});
};

const parseYears = async (response) => {
	const blob = await response.blob();
	const buffer = await blob.arrayBuffer();
	const data = new DataView(buffer);
	return array(buffer.byteLength / 2).map((ind) => {
		const year = data.getUint16(ind * 2 + 0);
		return { year };
	});
};

const parseFlights = async (response) => {
	const blob = await response.blob();
	const buffer = await blob.arrayBuffer();
	const data = new DataView(buffer);
	return array(buffer.byteLength / 16).map((ind) => {
		const startsAt = (BigInt(data.getUint32(ind * 16 + 0)) << 32n) | BigInt(data.getUint32(ind * 16 + 4));
		const endsAt = (BigInt(data.getUint32(ind * 16 + 8)) << 32n) | BigInt(data.getUint32(ind * 16 + 12));
		return { startsAt, endsAt };
	});
};

const loadYears = async () => {
	window.requestAnimationFrame(() => loadingYears());
	try {
		if (window.location.origin.startsWith('http://localhost')) {
			await new Promise((resolve) => setTimeout(resolve, 400 + Math.random() * 800));
			if (Math.random() < 0.1) {
				throw { status: Math.floor(Math.random() * 200) + 400, statusText: 'explicit develop failure' };
			}
		}
		const response = await fetch('/api/flight');
		if (response.status < 200 || response.status > 299) {
			throw Error(response);
		}
		const data = await parseYears(response);
		window.requestAnimationFrame(() => paintYears(data));
	} catch {
		window.requestAnimationFrame(() => erroredYears());
	}
};

const loadFlights = async () => {
	window.requestAnimationFrame(() => loadingFlights());
	window.requestAnimationFrame(() => loadingAirtime());
	try {
		if (window.location.origin.startsWith('http://localhost')) {
			await new Promise((resolve) => setTimeout(resolve, 400 + Math.random() * 800));
			if (Math.random() < 0.1) {
				throw { status: Math.floor(Math.random() * 200) + 400, statusText: 'explicit develop failure' };
			}
		}
		const response = await fetch(`/api/flight?year=${getYear()}`);
		if (response.status < 200 || response.status > 299) {
			throw Error(response);
		}
		const data = await parseFlights(response);
		window.requestAnimationFrame(() => paintFlights(data));
		window.requestAnimationFrame(() => paintAirtime(data));
	} catch {
		window.requestAnimationFrame(() => erroredFlights());
		window.requestAnimationFrame(() => erroredAirtime());
	}
};
