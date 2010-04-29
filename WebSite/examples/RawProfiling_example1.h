#ifndef _RawProfiling_example1_h_
#define _RawProfiling_example1_h_

#include <vector>

class TowersOfHanoi {
	private:
		std::vector<int> pegs_[3];
		int NBDisks_;

	public:
		inline TowersOfHanoi(int NBDisks) : NBDisks_(NBDisks) {}

		void start();

		void playNDisks(char orgPeg, char destPeg, char tempPeg, int N);
		void moveDisk(char orgPeg, char destPeg);

		void displayGame() const;
};

#endif
