obj-m := charmod.o

all:
	make -C /lib/modules/6.8.0-60-generic/build M=$(PWD) modules

clean:
	make -C /lib/modules/6.8.0-60-generic/build M=$(PWD) clean
