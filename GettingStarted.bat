@ECHO OFF
REM ##marker##"getting started"
REM ##begin##"getting started"
@ECHO ON
bin\CodeWorker  -script Scripts/Tutorial/GettingStarted/Tiny-leaderScript0.cws
bin\CodeWorker  -script Scripts/Tutorial/GettingStarted/Tiny-leaderScript1.cws
bin\CodeWorker  -script Scripts/Tutorial/GettingStarted/Tiny-leaderScript2.cws
bin\CodeWorker  -I Scripts/Tutorial/GettingStarted  -define DESIGN_FILE=SolarSystem0.sml -script LeaderScript0.cws
bin\CodeWorker  -I Scripts/Tutorial -path .  -define DESIGN_FILE=GettingStarted/SolarSystem0.sml  -script GettingStarted/LeaderScript1.cws
bin\CodeWorker  -I Scripts/Tutorial -path .  -define DESIGN_FILE=GettingStarted/SolarSystem0.sml  -script GettingStarted/LeaderScript2.cws
bin\CodeWorker  -I Scripts/Tutorial -path .  -define DESIGN_FILE=GettingStarted/SolarSystem0.sml  -script GettingStarted/LeaderScript3.cws
bin\CodeWorker  -I Scripts/Tutorial -path .  -define DESIGN_FILE=GettingStarted/SolarSystem0.sml  -script GettingStarted/LeaderScript4.cws
bin\CodeWorker  -I Scripts/Tutorial -path .  -define DESIGN_FILE=GettingStarted/SolarSystem0.sml  -script GettingStarted/LeaderScript5.cws  -stdin GettingStarted/Debugger.cmd -debug
bin\CodeWorker  -I Scripts/Tutorial -path .  -define DESIGN_FILE=GettingStarted/SolarSystem0.sml  -script GettingStarted/LeaderScript6.cws  -quantify Scripts/Tutorial/GettingStarted/quantify.html
@ECHO OFF
REM ##end##"getting started"

