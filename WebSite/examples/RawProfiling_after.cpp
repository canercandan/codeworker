#include <iostream>

#ifdef RAW_PROFILING
#	include "RawProfiling.h"
#endif

#include "RawProfiling_example1.h"


void TowersOfHanoi::start() {
#ifdef RAW_PROFILING
	RawProfiling::RawProfiling::TowersOfHanoi_start_counter++;
	RawProfiling::RawProfiling theProfiling(RawProfiling::RawProfiling::TowersOfHanoi_start_chronograph);
#endif
	// Remove all disks from the game
	pegs_[0].clear();
	pegs_[1].clear();
	pegs_[2].clear();
	// A number designates each disks according to its size:
	//     0 is the smallest, <NBDisks>-1 is the biggest.
	// The disks are piled up in a decrease order.
	for (int i = NBDisks_ - 1; i >= 0; i--) pegs_[0].push_back(i);
	playNDisks('A', 'B', 'C', NBDisks_);
}

// Recursive resolution of the towers of hanoi
//
// inputs:
//   - orgPeg:  origin peg of the disks,
//   - destPeg: destination peg of the disks,
//   - tempPeg: the last peg serves as an intermediate for moving
//       disks from the origin to the destination,
//   - N: number of disks to move from the origin
void TowersOfHanoi::playNDisks(char orgPeg, char destPeg, char tempPeg, int N) {
#ifdef RAW_PROFILING
	RawProfiling::RawProfiling::TowersOfHanoi_playNDisks_counter++;
	RawProfiling::RawProfiling theProfiling(RawProfiling::RawProfiling::TowersOfHanoi_playNDisks_chronograph);
#endif
	// we should isolate the test (N == 1) to move the only disk from the
	// origin to the destination, but we don't care about too much calls.
	if (N >= 1) {
		// Move (N - 1) disks from the origin to the temporary location
		playNDisks(orgPeg, tempPeg, destPeg, N - 1);
		// The top disk of the origin is moved to the destination
		moveDisk(orgPeg, destPeg);
		// Move the (N - 1) disks we have just put at the temporary location
		// to the destination
		playNDisks(tempPeg, destPeg, orgPeg, N - 1 );
	}
}

void TowersOfHanoi::moveDisk(char orgPeg, char destPeg) {
#ifdef RAW_PROFILING
	RawProfiling::RawProfiling::TowersOfHanoi_moveDisk_counter++;
	RawProfiling::RawProfiling theProfiling(RawProfiling::RawProfiling::TowersOfHanoi_moveDisk_chronograph);
#endif
	int topDisk = pegs_[orgPeg  - 'A'].back();
	pegs_[orgPeg  - 'A'].pop_back();
	pegs_[destPeg - 'A'].push_back(topDisk);
	displayGame();
}

void TowersOfHanoi::displayGame() const {
#ifdef RAW_PROFILING
	RawProfiling::RawProfiling::TowersOfHanoi_displayGame_counter++;
	RawProfiling::RawProfiling theProfiling(RawProfiling::RawProfiling::TowersOfHanoi_displayGame_chronograph);
#endif
	for (int iLevel = NBDisks_ - 1; iLevel >= 0; iLevel--) {
		for (int i = 0; i < 3; i++) {
			if (iLevel >= pegs_[i].size()) {
				// no disk yet at this level on this peg
				std::cout << std::string(NBDisks_, ' ') << "|" << std::string(NBDisks_, ' ');
			} else {
				std::string sHalfSpace = std::string(NBDisks_ - pegs_[i][iLevel], ' ');
				std::string sHalfDisk  = std::string(pegs_[i][iLevel], '*');
				std::cout << sHalfSpace << sHalfDisk << "*" << sHalfDisk << sHalfSpace;
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
