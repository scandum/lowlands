#!/bin/tcsh

# put PID in a file named script.pid
echo $$ > script.pid

# Set the port number.
set port = 4321
set __NOLOG = "NO"
set __AREADEBUG = 0
set __NOSOCKET = ""
set __COUNT = ""

if ( "$1" == "-p" ) set port="$2"
if ( "$2" == "-p" ) set port="$3"
if ( "$3" == "-p" ) set port="$4"
if ( "$4" == "-p" ) set port="$5"
if ( "$5" == "-p" ) set port="$6"
if ( "$6" == "-p" ) set port="$7"

if ( "$1" == "-areadebug") set __AREADEBUG = "$2"
if ( "$2" == "-areadebug") set __AREADEBUG = "$3"
if ( "$3" == "-areadebug") set __AREADEBUG = "$4"
if ( "$4" == "-areadebug") set __AREADEBUG = "$5"
if ( "$5" == "-areadebug") set __AREADEBUG = "$6"
if ( "$6" == "-areadebug") set __AREADEBUG = "$7"

if ( "$1" == "-count") set __COUNT = "-count"
if ( "$2" == "-count") set __COUNT = "-count"
if ( "$3" == "-count") set __COUNT = "-count"
if ( "$4" == "-count") set __COUNT = "-count"
if ( "$5" == "-count") set __COUNT = "-count"
if ( "$6" == "-count") set __COUNT = "-count"
if ( "$7" == "-count") set __COUNT = "-count"

if ( "$1" == "-h" ) then
	echo "syntax : lola.csh [-h]"
	echo "    or : lola.csh [-p <port#>] [-areadebug <level>] [-nosocket] [-n] [-count]"
	exit
endif

if ( "$1" == "-n" ) set __NOLOG = "YES"
if ( "$2" == "-n" ) set __NOLOG = "YES"
if ( "$3" == "-n" ) set __NOLOG = "YES"
if ( "$4" == "-n" ) set __NOLOG = "YES"
if ( "$5" == "-n" ) set __NOLOG = "YES"
if ( "$6" == "-n" ) set __NOLOG = "YES"
if ( "$7" == "-n" ) set __NOLOG = "YES"

if ( "$1" == "-nosocket" ) set __NOSOCKET = "-nosocket"
if ( "$2" == "-nosocket" ) set __NOSOCKET = "-nosocket"
if ( "$3" == "-nosocket" ) set __NOSOCKET = "-nosocket"
if ( "$4" == "-nosocket" ) set __NOSOCKET = "-nosocket"
if ( "$5" == "-nosocket" ) set __NOSOCKET = "-nosocket"
if ( "$6" == "-nosocket" ) set __NOSOCKET = "-nosocket"
if ( "$7" == "-nosocket" ) set __NOSOCKET = "-nosocket"

echo "Starting lola..."

set CUR_DIR = `pwd`
set RUN_FILE = $CUR_DIR/RUNNING

# Change to area directory.
cd $CUR_DIR/area_current

# set the default time to let the script wait before starting anew...
set _SLEEP_ = 60

# Remove the old core file.
if ( -e $CUR_DIR/area_current/core ) then
	rm $CUR_DIR/area_current/core >/dev/null
endif

#echo "About to set limits..."

# Set limits.
# nice -20
#
# Commented this out, to see what happens...
# -Manwe
#
# unlimit

while ( 1 )

	set matches = `netstat -an | grep ":$port " | grep -c LISTEN`

	if ($matches == 1) then
		echo "Port $port is already in use."
		exit
	endif

	if ( -e shutdown.txt ) rm -f shutdown.txt

	if (! -d $CUR_DIR/log ) then
		mkdir $CUR_DIR/log
	endif

	echo Finding log file
	# If you want to have logs in a different directory,
	#   change the 'set logfile' line to reflect the directory name.
	set index = 1000
	while ( 1 )
		set logfile = $CUR_DIR/log/$index.log
		if ( ! -e $logfile ) break
		@ index++
	end


	echo Found new log file $logfile
	# Record starting time
	date > $logfile
	limit > $logfile

	if ("$__NOLOG" != "YES") then
		tail --pid=$$ -f $logfile &
	endif

	# Record initial charges
	# charges >> $logfile

	echo Startup with log file $logfile

	# Do as much as you can to get the binaries 'there'...
	if (! -d $CUR_DIR/bin) then
		mkdir $CUR_DIR/bin
	endif

	if (! -e $CUR_DIR/bin/current_lola ) then
		echo "There is no binary file."
		if ( -e $CUR_DIR/compile ) then
			echo "Run the script 'compile' to create a binary."
			exit 0
		else
			echo Be sure to compile, and copy src/lola to bin/current_lola.
			rm $RUN_FILE
			exit 0
		endif
	endif

	$CUR_DIR/bin/current_lola -p $port -areadebug $__AREADEBUG $__NOSOCKET $__COUNT >&! $logfile

	# Analyze the core file
	if ( -e $CUR_DIR/area_current/core ) then
		echo where | gdb ../bin/current_lola core >> $logfile
		echo "\n"
	endif

	# Restart, giving old connections a chance to die.

	if (-e $RUN_FILE ) then
		rm -f $RUN_FILE
	endif
	
	if ( -e shutdown.txt ) then
		rm -f shutdown.txt
		if ( -e script.pid ) then
			rm script.pid;
		endif
		echo "The game is shutdown and will not reboot now"
		echo "start $0 again when you are ready."
		exit 0
	else
		echo "Starting the mud in ${_SLEEP_} seconds again."
		sleep $_SLEEP_
	endif
end
