/* LAST UPDATE: 19-05-2005 */


#include <stdio.h>
#include <fcntl.h>


#include "rtwndb_access.h"
#include "rtwndb_access.h"
#include "synop_normreport.h"


RTWNDB_ACCESS		rtac;



main(int c, char *argv[])
{
   /* nodig voor RTWNDB access routines */
   SEARCHKEY	      searchtype;
   int		         reportsdate;
   int		         reportstime;
   int		         storetime            = ALL_STORETIMES;
   int		         lng_low              = ALL_AREA;
   int		         lng_high             = ALL_AREA;
   int		         lat_low              = ALL_AREA;
   int		         lat_high             = ALL_AREA;
   int	      	   curindex             = -1;
   SYNOP_NORMREPORT	*synopptr;

   /* zelf geiintroduceerde var */
   FILE                  *extractiefile;
   char                  outputfilenaam[512];
   int                   valint;
   char                  valstr[50];
   int                   int_JaarMaand;
   int                   int_Dag;
   int                   int_Uur;
   int                   sub_reportsdate;
   float                 breedte;
   char                  halfrond[2];
   float                 lengte;
   int doorgaan;
   int i;
   int ff_knots;


   /* inlezen meegeven argument = JJJJMMDDUU */
   /* = via scriptfile eerste argument (argv[1] bij " extdate 2") */

   /* outputfile naam opbouwen b.v. 2000010106.WIND */
   strcpy(outputfilenaam, "\0");
   /*sprintf(outputfilenaam, "%s", "input_wind/");       // sub directory*/
   strcpy(outputfilenaam, (char*)getenv("ENV_SPECTRA_WIND"));   /* i.vm. APL */
   strcat(outputfilenaam, argv[1]);                    // b.v. input_wind/2000010106
   strcat(outputfilenaam, ".WIND");                    // file name extension bv wind/2000010106.WIND

   extractiefile = fopen(outputfilenaam, "w");  // output file

   /* kolom aanduidingen regel */
   fprintf(extractiefile, "___id ___la ____lo dd ff YYYYMMDDHH");

   // nieuwe regel
   fprintf(extractiefile, "\n");   // nieuwe regel



   /* op dit moment 26-1-1999 kan reportsdate argument van   */
   /* rtwndb access routines maar 2 cijfers voor jaartal aan */
   sub_reportsdate = atol(argv[1]) / 100;       // nu: JJJJMMDD b.v. 20000101
   reportsdate = sub_reportsdate % 1000000;     // nu: JJMMDD b.v.(000)101
   reportstime = (atoi(argv[1]) % 100) * 100;   // nu UU00b.v. 1000 (10 uur)

   for (i = 0; i < 2; i++)
   {
      if (i == 0)
         searchtype = SYNOP_BUOY;
      else if (i == 1)
         searchtype = SYNOP_STATION;

      /* RTWNDB access */
      if (rtwndb_access_select(searchtype, reportsdate,
                            reportstime, storetime, ALL_ORIGINS,
		                      lng_low, lng_high, lat_low, lat_high,
         	      	       &rtac) < 0)
      {
         rtwndb_access_perror("RTWNDB_ACCESS(select)");
         exit(rtwndb_access_errno);
      } // if


      /* ophalen elementen */
      while ((synopptr = rtwndb_access_getobs(&rtac)) != NULL)
      {
         if (i == 1)            // serarchtype SYNOP_STATION
         {
            // bepalen of het wel een nuttig station is
            if (TLBX_PRESENT("II", &rtac) && TLBX_PRESENT("iii", &rtac))
            {
               if ( ((synopptr-> II == 06) && (synopptr-> iii == 235)) || // De Kooy
                    ((synopptr-> II == 06) && (synopptr-> iii == 252)) || // K13
                    ((synopptr-> II == 06) && (synopptr-> iii == 254)) || // Meetpost Noordwijk
                    ((synopptr-> II == 06) && (synopptr-> iii == 321)) || // Euro
                    ((synopptr-> II == 06) && (synopptr-> iii == 320)) || // Goeree
                    ((synopptr-> II == 06) && (synopptr-> iii == 242)) || // Vlieland
                    ((synopptr-> II == 06) && (synopptr-> iii == 285)) || // Huibertgat
                    ((synopptr-> II == 06) && (synopptr-> iii == 209)) || // Ymuiden meetpaal
                    ((synopptr-> II == 06) && (synopptr-> iii == 330)) || // H.V.H
                    ((synopptr-> II == 06) && (synopptr-> iii == 312)) || // BG2
                    ((synopptr-> II == 06) && (synopptr-> iii == 313)) || // Vlakte van de Raan
                    ((synopptr-> II == 06) && (synopptr-> iii == 311)) || // Hoofdplaat
                    ((synopptr-> II == 06) && (synopptr-> iii == 229)) || // Texelhors
                    ((synopptr-> II == 06) && (synopptr-> iii == 277)) || // Lauwersoog
                    ((synopptr-> II == 06) && (synopptr-> iii == 209)) || // ijmuiden meetpaal
                    ((synopptr-> II == 06) && (synopptr-> iii == 315)) )  // Hansweert
                {
                   doorgaan = 1;               // true
                }
                else
                   doorgaan = 0;               // false

            } // if (TLBX_PRESENT("II", &rtac) && TLBX_PRESENT("iii", &rtac))
         } // if (i == 1)
         else // SYNOP_BUOY
            doorgaan = 0;                      // false (deze tak wordt dus niet gebruikt


         if (doorgaan == 1)                    // true
         {
            if (searchtype == SYNOP_STATION)
            {
               fprintf(extractiefile, "%02d", synopptr-> II);
               fprintf(extractiefile, "%03d", synopptr-> iii);
               fprintf(extractiefile, " ");
            }
            else // searchtype == SYNOP_BUOY;
            {
               /* nummer */
               fprintf(extractiefile, "%01d", synopptr-> Ai);
               fprintf(extractiefile, "%01d", synopptr-> bw);
               fprintf(extractiefile, "%03d", synopptr-> nb);
               fprintf(extractiefile, " ");
            } // else (searchtype = SYNOP_BUOY)

            /* breedte */
            if (TLBX_I_CONVERT("La", "DGR", 1, &rtac, &valint) == 1)    /* in 0.1 */
            {
               breedte = (float)valint / 10;

               if (breedte < 0.0)
               {
                  strncpy(halfrond, "S", 1);
                  halfrond[1] = '\0';
                  breedte *= -1.0;
               }
               else
               {
                  strncpy(halfrond, "N", 1);
                  halfrond[1] = '\0';
               }

               fprintf(extractiefile, "%04.1f", breedte);
               fprintf(extractiefile, "%s", halfrond);
            } /* if (TLBX_I_CONVERT("La", "DGR", 1, &rtac, &valint) == 1) */
            else
               fprintf(extractiefile, "/////");

            /* spatie */
            fprintf(extractiefile, " ");


            /* lengte */
            if (TLBX_I_CONVERT("Lo", "DGR", 1, &rtac, &valint) == 1)    /* in 0.1 */
            {
               lengte = (float)valint / 10;

               if (lengte < 0.0)
               {
                  strncpy(halfrond, "W", 1);
                  halfrond[1] = '\0';
                  lengte *= -1.0;
               }
               else
               {
                  strncpy(halfrond, "E", 1);
                  halfrond[1] = '\0';
               }

               fprintf(extractiefile, "%05.1f", lengte);
               fprintf(extractiefile, "%s", halfrond);
            } /* if (TLBX_I_CONVERT("Lo", "DGR", 1, &rtac, &valint) == 1) */
            else
               fprintf(extractiefile, "//////");

            /* spatie */
            fprintf(extractiefile, " ");

            /* dd (WMO code tabel 0877) */
            if (TLBX_PRESENT("dd", &rtac))
            {
               if (strncmp((char*)synopptr -> dd, "99", 2) == 0)
                  fprintf(extractiefile, "//");
               else
                  fprintf(extractiefile, "%2s", synopptr -> dd);
            }
            else
               fprintf(extractiefile, "//");

            /* spatie */
            fprintf(extractiefile, " ");

            /* ff (altijd in 0.1 m/s in de RTWNDB) moet in hele kots worden omgezet*/
            /* 1 knot = 0.514444444 meters/second */
            /* let op correcte afronding               */
            /* b.v. 123 + 5 = 128; 128 / 10 = 12 m/s   */
            /* b.v. 126 + 5 = 131; 131 / 10 = 13 m/s   */
            /* */
            /* NB voor SYNOP_STATION staat het wel in 0,1 m/s in RTWNDB maar wel afgerond */
            /* omdat ff uit synop ook in hele m/s binnenkomt */
            /* voor BUOY_SYNOP zouden voor sommige (lang niet alle) wel de echte m/s te bepalen zijn 
            /* echter voor uniformiteit en omdat een nauwkeurigheid van 1 m/s genoeg is dit zo gedaan */
            /* */
            /* het lijkt er ook op dat de waarden van de nederlandse cic stations (via SYNOP_STATION) */
            /* niet altijd goed afgerond zijn (komt door kn - > m/s ????) */
            /* */
            /* bij BUOY_SYNOP wordt wel overal correct afgerond */
            /* */
            if (TLBX_PRESENT("s1_ff", &rtac))
            {  
                ff_knots = (float)synopptr-> s1_ff / (0.514444444 * 10) + 0.5;  // van 0.1 m/s -> knots
                if (ff_knots <= 99)
                   fprintf(extractiefile, "%2d", ff_knots);
                else
                   fprintf(extractiefile, "//");
            } 
            else
               fprintf(extractiefile, "//");



            /* NB iw (staat zoals ingezonden; echter in de RTWNDB  */
            /* is de ff altijd in m/s)                          */




            /* spatie */
            fprintf(extractiefile, " ");

      
            // dtg
            // Jaar en Maand komen uit de scriptfile (eerste argument)
            // Dag en uur uit de synops zelf
    
            if ((int_JaarMaand = atoi(argv[1]) / 10000) == 0)
               int_JaarMaand = -1;
            if (TLBX_PRESENT("YY", &rtac))
               int_Dag = synopptr-> YY;
            else
               int_Dag = -1;

            if (TLBX_PRESENT("GG", &rtac))
               int_Uur = synopptr-> GG;
            else
               int_Uur = -1;
         
            if (int_JaarMaand != -1 && int_Dag != -1 && int_Uur != -1)
            {
               fprintf(extractiefile, "%04d", int_JaarMaand);
               fprintf(extractiefile, "%02d", int_Dag);
               fprintf(extractiefile, "%02d", int_Uur);
            } // if (int_JaarMaand != -1 && int_Dag != -1 && int_Uur != -1)   
            else
               fprintf(extractiefile, "////////");  

            // test
            //fprintf(extractiefile, "%s", str_MiMiMjMj); // AAXX, BBXX, ZZYY

            // nieuwe regel
            fprintf(extractiefile, "\n");   // nieuwe regel


         } // if (doorgaan == 1)

      } // while (( synopptr = rtwndb_access_getobs( &rtac )) != NULL)

      if (synopptr == NULL && rtwndb_access_errno != 0)
         rtwndb_access_perror( "RTWNDB_ACCESS(getobs)");

   } // for

   fclose(extractiefile);
}
