#!/bin/bash

MODULE=charmod
DEVICE=charmod
COUNT=2

function load() {
    echo "Loading module..."
    sudo insmod $MODULE.ko || { echo "Failed to load module"; exit 1; }

    MAJOR=$(awk "\$2==\"$DEVICE\" {print \$1}" /proc/devices)
    if [ -z "$MAJOR" ]; then
        echo "Could not find major number for $DEVICE"
        exit 1
    fi

    echo "Creating /dev nodes..."
    for i in $(seq 0 $((COUNT - 1))); do
        sudo mknod /dev/${DEVICE}${i} c $MAJOR $i
        sudo chmod 666 /dev/${DEVICE}${i}
    done

    echo "Module loaded with major=$MAJOR"
}

function unload() {
    echo "Unloading module..."
    sudo rmmod $MODULE || { echo "Failed to unload module"; exit 1; }

    echo "Removing /dev nodes..."
    for i in $(seq 0 $((COUNT - 1))); do
        sudo rm -f /dev/${DEVICE}${i}
    done

    echo "Module unloaded."
}

case "$1" in
    load)
        load
        ;;
    unload)
        unload
        ;;
    reload)
        unload
        load
        ;;
    *)
        echo "Usage: $0 {load|unload|reload}"
        exit 1
        ;;
esac
