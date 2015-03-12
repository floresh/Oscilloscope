# invoke SourceDir generated makefile for app.pem3
app.pem3: .libraries,app.pem3
.libraries,app.pem3: package/cfg/app_pem3.xdl
	$(MAKE) -f C:\Users\Harold\DOCUME~1\SCHOOL~1\ECE\ECE384~1\labs\lab0\WORKSP~1\COPYOF~2/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Harold\DOCUME~1\SCHOOL~1\ECE\ECE384~1\labs\lab0\WORKSP~1\COPYOF~2/src/makefile.libs clean

