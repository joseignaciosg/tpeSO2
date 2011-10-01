.SILENT
MAKE = d:\bc\bin\make 

all:
	cd kernel
	echo -- Directorio kernel --
	echo
	$(MAKE)
	sleep 2
	cd ..
	cd progs
	echo
	echo -- Directorio progs --
	echo
	$(MAKE)
	cd ..

clean:
	del kernel\*.obj >nul
	del kernel\*.lib >nul
	del progs\*.obj >nul
	del progs\*.exe >nul

tiny:
	echo MODEL = -mt >model.mak
	$(MAKE) clean all
small:
	echo MODEL = -ms >model.mak
	$(MAKE) clean all
medium:
	echo MODEL = -mm >model.mak
	$(MAKE) clean all
compact:
	echo MODEL = -mc >model.mak
	$(MAKE) clean all
large:
	echo MODEL = -ml >model.mak
	$(MAKE) clean all
huge:
	echo MODEL = -mh >model.mak
	$(MAKE) clean all
