int f(int i) {
	switch (i) {
		case 2:
		case 3:
			if (i == 2) {
				h();
			}
			g(i - 1);
			break;
	}
}
