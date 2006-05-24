
.PHONY: zip

ZIPNAME=kpinwa2_`date +%Y%m%d`
ARC = release/kpinwa2.kpi kpinwa2.txt kpinwa2.ini nwk2nwa.zip

zip:
	zip -9j $(ZIPNAME) $(ARC)
