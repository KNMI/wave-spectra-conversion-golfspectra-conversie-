#!/bin/csh


#set PRESENTDATE = '/usr/knmi/bin/extdate'
#set BACKUPDATE = '/usr/knmi/bin/backupdate'

#set PRESENTDATE = '/d/appl/linux/knmi/bin/extdate'
#set BACKUPDATE = '/d/appl/linux/knmi/bin/backupdate'

#set PRESENTDATE = '/usr/local/free/bin/extdate'
#set BACKUPDATE = '/usr/local/free/bin/backupdate'
set PRESENTDATE = '/usr/people/stam/bin/extdate'
set BACKUPDATE = '/usr/people/stam/bin/backupdate'





#########################################################################
#                                                                       #
#                                modes                                  #
#                                                                       #
#########################################################################
# 
# dit script aanroepen met 1 argument
# dit argument kan zijn het cijfer 1 of 2 (dus "spectra_control.sc 1" of "spectra_control.sc 2")
#
# bij mode (argument) = 1 : (file)datum-tijd gebaseerd op huidige uur (typisch de aanroep van 35 minuten over hele uur)
# bij mode (argument) = 2 : (file)datum-tijd gebaseerd op 1 uur terug (typisch de aanroep van 5 minuten over hele uur)








#########################################################################
#                                                                       #
#                         ENVIRONMENT VARS                              #
#                                                                       #
#########################################################################

# input dir waar de golven bulletins vandaan moeten komen (golf bulletin bv: MSS_R276123330_595_SOVF82_EHDB_031200)
#setenv ENV_SPECTRA_KNMI /net/bsspop-rtw/rtwndb/data/NSBD
#setenv ENV_SPECTRA_KNMI /bsspop-rtw/rtwndb/data/NSBD
#setenv ENV_SPECTRA_KNMI /data/bsspop-rtw/rtwndb/data/NSBD
setenv ENV_SPECTRA_KNMI /data/rtwndb/data/NSBD

# voor RTWNDB (wind data)
#setenv GVWRNDATA /net/bsspop-rtw/rtwndb/data/WNDB
#setenv GVWRNDATA /bsspop-rtw/rtwndb/data/WNDB
#setenv GVWRNDATA /data/bsspop-rtw/rtwndb/data/WNDB
setenv GVWRNDATA /data/rtwndb/data/WNDB

# spectra home dir waar het volgende geinstalleerd moet zijn:
#
# - directory: input_sovf80
# - directory: input_sovf81
# - directory: input_sovf82
# - file: get_wind_data.exe
# - file: spectra.exe
# - file: spectra_control.sc (dit script file)
#
# - eventueel file: get_wind_data.c
# - eventueel file: spectra_main.c
# - eventueel file: spectra_sovf80_input.c
# - eventueel file: spectra_sovf81_input.c
# - eventueel file: spectra_sovf82_input.c
# - eventueel file: spectra.h
# - eventueel file: makefile_get_wind_data
# - eventueel file: makefile_spectra
#
# - eventueel copy.sc   (aangeroepen via crontab; copy van output files voor Frits Koek)
# - eventueel remove.sc (aangeroepen via crontab; 1x per dag verwijderen files ouder dan 7 dagen uit input_backup, output_cds etc..., maar niet uit input_sovf80, input_sovf81 en sovf82, dit gebeurt in dit script uurlijks)


setenv ENV_SPECTRA_LOCAL /usr/people/stam/SPECTRA

# output dir's voor diverse aangemaakte files
setenv ENV_SPECTRA_CIC /usr/people/stam/SPECTRA/output_cic/
setenv ENV_SPECTRA_CDS /usr/people/stam/SPECTRA/output_cds/
setenv ENV_SPECTRA_CID /usr/people/stam/SPECTRA/output_cid/
setenv ENV_SPECTRA_DSP /usr/people/stam/SPECTRA/output_dsp/
setenv ENV_SPECTRA_LDS /usr/people/stam/SPECTRA/output_lds/
setenv ENV_SPECTRA_LFD /usr/people/stam/SPECTRA/output_lfd/
setenv ENV_SPECTRA_LFR /usr/people/stam/SPECTRA/output_lfr/

# log dir + filenaam
setenv ENV_SPECTRA_LOG /usr/people/stam/SPECTRA/log/log.txt

# wind dir
setenv ENV_SPECTRA_WIND /usr/people/stam/SPECTRA/input_wind/

# dir waar file (suspect.txt) met verdachte stations staat */
setenv ENV_SPECTRA_SUSPECT /usr/people/stam/SPECTRA/suspect/
#
# - file: suspect.txt (niet leeg)








if (-e $PRESENTDATE && $#argv == 1) then

   set dtg = `$PRESENTDATE 2`
   set mode = $argv[1]
   



   if ($mode == 1) then

      #########################################################################
      #                                                                       #
      #                     WIND (verwerken voor GOLVEN)                      #
      #                                                                       #
      #########################################################################


      #
      # ophalen files van 0 uur terug
      # 
      set dtgw = `$BACKUPDATE $dtg -0`
      set dtgw = `echo $dtgw|awk '{printf "%.10s",$0}'`

      cd $ENV_SPECTRA_LOCAL
      #get_wind_data.exe $dtgw
      #/usr/people/stam/SPECTRA/get_wind_data.exe $dtgw
      $ENV_SPECTRA_LOCAL/get_wind_data.exe $dtgw

      #
      # verwijderen van 7 dagen oude files
      # 
      set dtgw7 = `$BACKUPDATE $dtg -168`
      set dtgw7 = `echo $dtgw7|awk '{printf "%.10s",$0}'`

      #verwijderen uit wind directory
      cd $ENV_SPECTRA_WIND
      rm -f $dtgw7".WIND"

   endif





   #########################################################################
   #                                                                       #
   #                              GOLVEN                                   #
   #                                                                       #
   #########################################################################


   if ($mode == 1) then

      #
      # 0 geleden 
      #
      #huidige tijd 0 uur terug (format JJJJMMDDUU, dus minuten verwaarloosd)
      #het gaat hier alleen om de DDUU, deze er dus uitlichten 
      set dtgw0 = `$BACKUPDATE $dtg -0`
      set dtgw0 = `echo $dtgw0|awk '{printf substr($0, 7, 4)}'`
      # NB de DDUU zijn nu afgesplitst !
 
      #
      # input files kopieeren naar spectra input dirs.
      # in input_sovf80 etc. worden de files weer vrij snel verwijderd (om bij inlezen verwarring met oudere files te voorkomen 
      # daarom nog een backup in de input_backup dir (worden verwijderd via remove.sc, via crontab)
      #
      cd $ENV_SPECTRA_LOCAL/input_sovf80
      cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF80_EHDB_$dtgw0"00" $ENV_SPECTRA_LOCAL/input_sovf80
      cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF80_EHDB_$dtgw0"00" $ENV_SPECTRA_LOCAL/input_backup

      cd $ENV_SPECTRA_LOCAL/input_sovf81
      cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF81_EHDB_$dtgw0"00" $ENV_SPECTRA_LOCAL/input_sovf81
      cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF81_EHDB_$dtgw0"00" $ENV_SPECTRA_LOCAL/input_backup

      cd $ENV_SPECTRA_LOCAL/input_sovf82
      cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF82_EHDB_$dtgw0"00" $ENV_SPECTRA_LOCAL/input_sovf82
      cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF82_EHDB_$dtgw0"00" $ENV_SPECTRA_LOCAL/input_backup


      #
      # process de data [LETOP i.v.m. aanmaak LDS files die soms data uit LFR files haalt SOVF80 verwerken VOOR SOVF82]
      #
      #huidige tijd 0 uur terug (format JJJJMMDDUU, dus minuten verwaarloosd)
      set dtgw1 = `$BACKUPDATE $dtg -0`
      set dtgw1 = `echo $dtgw1|awk '{printf "%.10s",$0}'`
   

      cd $ENV_SPECTRA_LOCAL
      #spectra.exe $dtgw1 SOVF80
      #spectra.exe $dtgw1 SOVF81
      #spectra.exe $dtgw1 SOVF82
      $ENV_SPECTRA_LOCAL/spectra.exe $dtgw1 SOVF80
      $ENV_SPECTRA_LOCAL/spectra.exe $dtgw1 SOVF81
      $ENV_SPECTRA_LOCAL/spectra.exe $dtgw1 SOVF82
      #/usr/people/stam/SPECTRA/spectra.exe $dtgw1 SOVF80
      #/usr/people/stam/SPECTRA/spectra.exe $dtgw1 SOVF81
      #/usr/people/stam/SPECTRA/spectra.exe $dtgw1 SOVF82


   endif





   if ($mode == 2) then

      #
      # 1 geleden 
      #
      #huidige tijd 1 uur terug (format JJJJMMDDUU, dus minuten verwaarloosd)
      #het gaat hier alleen om de DDUU, deze er dus uitlichten 
      set dtgw1a = `$BACKUPDATE $dtg -1`
      set dtgw1a = `echo $dtgw1a|awk '{printf substr($0, 7, 4)}'`
      # de DDUU zijn nu afgesplitst
 

      #huidige tijd 1 uur terug (format JJJJMMDDUU, dus minuten verwaarloosd)
      set dtgw1b = `$BACKUPDATE $dtg -1`
      set dtgw1b = `echo $dtgw1b|awk '{printf "%.10s",$0}'`
  



      #
      # alleen indien er niets was aangemaakt input files kopieeren naar spectra input directories en daarna verwerken
      #
      cd $ENV_SPECTRA_LDS
      if (-z WAVE_LDS_$dtgw1b"00_00000_LC" || !(-e WAVE_LDS_$dtgw1b"00_00000_LC")) then

         # dus dan niet aanwezig (!-e) of leeg (-z)

         cd $ENV_SPECTRA_LOCAL/input_sovf80
         cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF80_EHDB_$dtgw1a"00" $ENV_SPECTRA_LOCAL/input_sovf80
         cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF80_EHDB_$dtgw1a"00" $ENV_SPECTRA_LOCAL/input_backup

         cd $ENV_SPECTRA_LOCAL/input_sovf81
         cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF81_EHDB_$dtgw1a"00" $ENV_SPECTRA_LOCAL/input_sovf81
         cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF81_EHDB_$dtgw1a"00" $ENV_SPECTRA_LOCAL/input_backup

         cd $ENV_SPECTRA_LOCAL/input_sovf82
         cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF82_EHDB_$dtgw1a"00" $ENV_SPECTRA_LOCAL/input_sovf82
         cp $ENV_SPECTRA_KNMI/MSS_*_*_SOVF82_EHDB_$dtgw1a"00" $ENV_SPECTRA_LOCAL/input_backup


         #
         # process de data [LETOP i.v.m. aanmaak LDS files die soms data uit LFR files haalt SOVF80 verwerken VOOR SOVF82]
         #

         cd $ENV_SPECTRA_LOCAL
         #spectra.exe $dtgw1b SOVF80
         #spectra.exe $dtgw1b SOVF81
         #spectra.exe $dtgw1b SOVF82
         $ENV_SPECTRA_LOCAL/spectra.exe $dtgw1b SOVF80
         $ENV_SPECTRA_LOCAL/spectra.exe $dtgw1b SOVF81
         $ENV_SPECTRA_LOCAL/spectra.exe $dtgw1b SOVF82


      endif



      #
      # verwijderen golven input files (i.v.m. de slechte datum tijd in SOVF fileaanduiding (alleen dag en uur) alle files verwijderen
      #                                 dit om misverstanden te voorkomen bij ophalen files
      #                                 NB mode 2 volgt altijd op mode 1

      #verwijderen uit input directory
      cd $ENV_SPECTRA_LOCAL/input_sovf80
      rm -f MSS_*

      cd $ENV_SPECTRA_LOCAL/input_sovf81
      rm -f MSS_*

      cd $ENV_SPECTRA_LOCAL/input_sovf82
      rm -f MSS_*

      #verwijderen aangemaakte files (.CID, .LDS etc.)
      #zie remove.sc (files verwijderen uit alle output dir's en uit input_backup)


   endif


endif
