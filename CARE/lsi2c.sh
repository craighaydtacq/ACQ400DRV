#!/bin/sh
# list all i2c devices

get_names() {
	bus=$1
	dev=$2
	devname=$(cat /sys/bus/i2c/devices/$bus/$dev/name)
	
        present=\?	
	case $devname in
	ad7417)
		temp=$(cat /sys/bus/i2c/devices/$bus/$dev/hwmon/hwmon?/temp1_input)
		if [ $temp -eq 0 ]; then
			present=NO
		else
			present="YES $temp"
		fi;;
	24c64|spd)
                [ -e /sys/bus/i2c/devices/$bus/$dev/eeprom ] && present=YES;;
	esac
	echo bus:$bus dev:$dev $devname present:$present
}

echo i2c devices by bus
for bus in $(ls /sys/bus/i2c/devices/ | grep i2c | sort -n -t- -k2); do
	echo $bus $(ls /sys/bus/i2c/devices/$bus/ | grep ${bus#*-}-0);
done

echo i2c device detail
for bus in $(ls /sys/bus/i2c/devices/ | grep i2c | sort -n -t- -k2); do
	for dev in $(ls /sys/bus/i2c/devices/$bus/ | grep ${bus#*-}-0); do
		get_names $bus $dev;
	done;
done


