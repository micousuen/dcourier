#! /bin/sh

basepath=$(cd `dirname $0`; pwd)

print_usage(){
	echo -e "\033[32mUsage:\033[0m\n\t Use \"\033[34mdcourier install\033[0m\" to install environment. \n\t Use \"\033[34mdcourier make\033[0m\" to automatically make files \n\t Use \"\033[34mdcourier debug\033[0m\" to generate dcourier debug file \n\t Use \"\033[34mdcourier start\033[0m\" to start dcourier, use \"\033[34mdcourier run\033[0m\" if you like it\n\t Use \"\033[34mdcourier clear\033[0m\" to clear running environment \n\t Use \"\033[34mdcourier help\033[0m\" to get more dcourier commands details "
}

print_usage_detail(){
	echo -e "\033[32mUsage:\033[0m\n\t Use \"\033[34mdcourier install\033[0m\" to install environment. \n\t  |----- Use \"\033[34mdcourier install online\033[0m\", if you can access to network. \n\t  |----- Use \"\033[34mdcourier install offline\033[0m\" to install environment with not network connection\n\t Use \"\033[34mdcourier make\033[0m\" to automatically make files \n\t  |----- Use \"\033[34mdcourier make gcc\033[0m\" to use gcc to compile dcourier\n\t  |----- Use \"\033[34mdcourier make gcc <PATH>\033[0m\" to use gcc under this path, input like this /usr/local/gcc-4.9.3/bin\n\t  |----- Use \"\033[34mdcourier make cmake\033[0m\" to use cmake to compile dcourier\n\t Use \"\033[34mdcourier debug\033[0m\" to generate dcourier debug file \n\t  |----- Use \"\033[34mdcourier debug run\033[0m\" to load file to gdb after the generation\n\t Use \"\033[34mdcourier start\033[0m\" to start dcourier, use \"\033[34mdcourier run\033[0m\" if you like it\n\t Use \"\033[34mdcourier clear\033[0m\" to clear running environment \n\t  |----- Use \"\033[34mdcourier clear data\033[0m\" to clear data only \n\t  |----- Use \"\033[34mdcourier clear trash\033[0m\" to clear running trash only. "
}
clear_data(){
	rm -rf ${basepath}/dataout/*
}
clear_running_trash(){
	rm -rf ${basepath}/log/
	rm -rf ${PWD}/log/
	rm -rf ${basepath}/trash/*
}

cmake_build(){
	echo -e "\033[32mUsing CMake to compile\033[0m"
	cd ${basepath}
	echo -e "\033[32mCleaning make environment\033[0m"
	rm -f CMakeCache.txt
	echo -e "\033[32mMaking files\033[0m"
	cmake .
	make
	echo -e "\033[32mFile generated at exe/dcourier\033[0m"
}

if [ $# -lt 1 ];then
	print_usage
	exit
fi


case $1 in
	"start" | "run" | "-s" | "-r" | "--start" | "--run")
	echo -e "\033[32mStarting program\033[0m"
	chmod 0755 ${basepath}/exe/DCOURIER
	${basepath}/exe/DCOURIER
	;;

	"install" | "-i" | "--install")
	if [ $# -gt 1 ];then
		if [ $2 = "online" ];then
			echo -e "\033[32mBegin online yum installation, network connection is required\033[0m"
			yum install glibc.i686 glibc-devel.i686 libgcc.i686 libstdc++.i686 nss-softokn-freebl.i686 -y
			echo -e "\033[32myum finished.\033[0m"
			cd ${basepath}/INSTALL
			exit
		else 
			echo -e "\033[32mInstalling environment without network\033[0m"
			cd ${basepath}/INSTALL
			chmod 0755 ./envinstall
			./envinstall
			echo -e "\033[32mOffline environment installation all finished.\033[0m"
			exit
		fi
	else 
		echo -e "\033[32mInstalling environment without network\033[0m"
		cd ${basepath}/INSTALL
		chmod 0755 ./envinstall
		./envinstall
		echo -e "\033[32mOffline environment installation all finished.\033[0m"
		exit
	fi
	;;

	"debug" | "-d" | "--debug" )
	echo -e "\033[32mCompiling project \033[0m"
	g++ -m64 -g -rdynamic -o ${basepath}/exe/gdb_test ${basepath}/src/main.cpp -std=gnu++11 -Wl,--no-as-needed -pthread -L${basepath}/lib/ -lstdc++ 
	echo -e "\033[32mCompile finished\033[0m"
	if [ $# -gt 1 ];then
		if [ $2 = "run" ];then
			echo -e "\033[34mrunning gdb to get debug information,use \"run\" to start debuging, and use \"quit\" to stop debug\033[0m"
			gdb ${basepath}/exe/gdb_test
		else 
			echo -e "\033[31mError: Parameter Error, file generated at exe/gdb_test\033[0m"
			exit
		fi
	else
		echo -e "\033[32mdebug file generated at exe/gdb_test\033[0m"
		exit
	fi
	;;

	"make" | "-m" | "--make" )
	if [ $# -gt 1 ];then
		if [ $2 = "gcc" ];then
			if [ $# -gt 2 ];then
				if [ -d $3 ];then
					if [ -f $3/g++ ];then
						echo -e "\033[32mUsing g++ at $3 to compile project\033[0m"
						$3/g++ -m32 -o ${basepath}/exe/dcourier ${basepath}/src/main.cpp -std=gnu++11 -Wl,--no-as-needed -pthread -L${basepath}/lib/ -lfwlib32 -lstdc++
					else
						echo -e "\033[31mError: there is no g++ under path $3\033[0m"
						exit
					fi
				else
					echo -e "\033[31mError: $3 path don't exist\033[0m"
					exit
				fi
			else
				echo -e "\033[32mUsing g++ to compile project\033[0m"
				g++ -m32 -o ${basepath}/exe/dcourier ${basepath}/src/main.cpp -std=gnu++11 -Wl,--no-as-needed -pthread -L${basepath}/lib/ -lfwlib32 -lstdc++
			fi
		elif [ $2 = "cmake" ];then
			cmake_build
		else 
			echo -e "\033[33mWarning: Unvaliable command argument\033[0m"
			cmake_build
		fi
	else
		cmake_build
	fi
	echo -e "\033[32mCompile finished.\033[0m"
	exit
	;;

	"clear" | "-c" | "--clear" )
	if [ $# -gt 1 ];then
		if [ $2 = "data" ];then
			echo -e "\033[32mClearing data......\033[0m"
			clear_data
			echo -e "\033[32mData cleaned\033[0m"
		elif [ $2 = "trash" ];then
			echo -e "\033[32mClearing running trash...\033[0m"
			clear_running_trash
			echo -e "\033[32mRunning trash cleaned.\033[0m"
		fi
	else
		echo -e "\033[32mClearing all data and running trash......\033[0m"
		clear_data
		echo -e "\033[32mdataout cleaned\033[0m"
		clear_running_trash
		echo -e "\033[32mrunning trash cleaned\033[0m"
	fi
	;;

	"help" | "-h" | "--help" )
	print_usage_detail
	exit
	;;

	* )
	print_usage
	exit
	;;
esac
