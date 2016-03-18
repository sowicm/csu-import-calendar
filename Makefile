
all:
	b2

debug:
	b2 debug

clean:
	rd /s /q bin

distclean: clean
	del CSU-Import-Class-Info-to-Calendar.exe

.PHONY: all clean distclean
