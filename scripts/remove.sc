#!/bin/csh


#files ouder dan 7 dagen verwijderen (t.o.v. aanmaak datum)

#input files
/usr/bin/find /usr/people/stam/SPECTRA/input_backup/ -ctime +7 -type f -name "*" -exec rm {} \;

#output files
/usr/bin/find /usr/people/stam/SPECTRA/output_cds/ -ctime +7 -type f -name "*" -exec rm {} \;
/usr/bin/find /usr/people/stam/SPECTRA/output_cic/ -ctime +7 -type f -name "*" -exec rm {} \;
/usr/bin/find /usr/people/stam/SPECTRA/output_cid/ -ctime +7 -type f -name "*" -exec rm {} \;
/usr/bin/find /usr/people/stam/SPECTRA/output_dsp/ -ctime +7 -type f -name "*" -exec rm {} \;
/usr/bin/find /usr/people/stam/SPECTRA/output_lds/ -ctime +7 -type f -name "*" -exec rm {} \;
/usr/bin/find /usr/people/stam/SPECTRA/output_lfd/ -ctime +7 -type f -name "*" -exec rm {} \;
/usr/bin/find /usr/people/stam/SPECTRA/output_lfr/ -ctime +7 -type f -name "*" -exec rm {} \;
