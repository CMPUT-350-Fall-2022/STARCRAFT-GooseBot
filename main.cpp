#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "GooseBot.h"
#include "LadderInterface.h"

// LadderInterface allows the bot to be tested against the built-in AI or
// played against other bots
// Testing
int main(int argc, char* argv[]) {

	RunBot(argc, argv, new GooseBot(), sc2::Race::Zerg);

	return 0;
}