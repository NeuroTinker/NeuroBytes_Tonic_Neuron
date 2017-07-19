tar extended-remote /dev/ttyACM0
file main.elf
monitor tpwr enable
monitor swdp_scan
attach 1
load
detach
quit