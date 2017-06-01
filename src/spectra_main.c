/**********************************************************************************************************************/
/*                                                                                                                    */
/*                                                                                                                    */
/*                                                                                                                    */
/*                                                                                                                    */
/*      versie 1.3           last updated: 22-03-2010                                                                 */
/*                                                                                                                    */
/*                                                                                                                    */
/*                                                                                                                    */
/*                                                                                                                    */
/**********************************************************************************************************************/
/* zie file aanvullingen.txt */



#define _ANSI_C_SOURCE

#if !defined(spectra_h)                                    /* Sentry use file only if it's not already included. */
#include "spectra.h"
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>                                          /* struct time_t */
#include <stdlib.h>                                        /* exit */

#if defined(WINDOWS)                                       /* zie gps.h */
#include <dir.h>                                           /* o.a. getcwd() in windows */
#else
#include <unistd.h>                                        /* o.a. getcwd() in UNIX */
#endif


/* function prototypes in deze main module */
int Write_Log(char* bericht);
int Bepaal_Suspect_Station(const char* sensorcode, char* flag);
int Bepaal_Lat_Lon_Q(const char* sensorcode, char* latitude, char* longitude, char* quadrant);
int Bepaal_ff_Reductie_Factor(const char* sensorcode, float* ff_factor);
int Check_Alternatieve_dd_ff(const char* sensorcode, char* dd, char* ff);
int Bepaal_Diepte(const char* sensorcode, float* DPT);

/* function prototypes in andere modulen */
extern int Read_SOVF80_Input_Files(void);
extern int Read_SOVF81_Input_Files(void);
extern int Read_SOVF82_Input_Files(void);

/* variables in deze module (kunnen ook in andere module(s) voorkomen) */
char OS[8];                                            /* WINDOWS of __UNIX_ */
char JJJJMMDDUU[11];                                   /* b.v. 2001080606 (run/waarneming datum-tijd zonder minuten) */
char MODE[7];                                          /* b.v. SOVF80 */

/* externe var's */




/*****************************************************************************/
/*                                                                           */
/*                                   Main                                    */
/*                                                                           */
/*****************************************************************************/

/*
// opstart modes:
//    SOVF80:
//    SOVF81:
//    SOVF82:
//
*/



int main (int argc, char *argv[])
{


   /*char buffer[10];*/

   /* in principe te gebruiken onder WINDOWS en UNIX */
   /* echter kleine verschillen soms (vooral bij path dir. en filenamen */

#if defined(WINDOWS)
   strcpy(OS, "WINDOWS");
#else
   strcpy(OS, "__UNIX_");
#endif





   if (argc == 3)
   {
      strcpy(JJJJMMDDUU, argv[1]);                       /* b.v. 2001080600 */
      strcpy(MODE, argv[2]);                             /* b.v. SOVF80, SOVF81, SOVF82 */
   } /* if (argc == 3) */
   else
      exit(1);                                           /* alleen met juiste aantal arguments */


   if (strncmp(MODE, "SOVF80", 6) == 0)                  /* 1D spectra (geen richtingen aanwezig) */
   {
      /*
      // verwerken 1D data
      */
      Read_SOVF80_Input_Files();

   } /* if (strncmp(MODE, "SOVF80", 6) == 0) */
   else if (strncmp(MODE, "SOVF81", 6) == 0)              /* spectra met richtingen (bandjes) */
   {
      /*
      // verwerken wave directional parameters data
      */
      Read_SOVF81_Input_Files();

   } /* if (strncmp(MODE, "SOVF81", 6) == 0) */
   else if (strncmp(MODE, "SOVF82", 6) == 0)              /* spectra met ruichtingen */
   {
      /*
      // verwerken wave directional parameters data
      */
      Read_SOVF82_Input_Files();

   } /* if (strncmp(MODE, "SOVF82", 6) == 0) */


   return 0;
}


/*****************************************************************************/
/*                                                                           */
/*                                  Write_Log                                */
/*                                                                           */
/*****************************************************************************/
int Write_Log(char* bericht)
{
   FILE* log;
   char logfilenaam[256];
   time_t t;
   int i;


   /* initialisatie */
   strcpy(logfilenaam, "\0");

   /* log file naam bepalen */
   if (strcmp(OS, "WINDOWS") == 0)                        /* WINDOWS */
      strcpy(logfilenaam, "log\\log.txt");
   else                                                   /* UNIX */
      /* strcpy(logfilenaam, "log/log.txt"); */
      strcpy(logfilenaam, getenv("ENV_SPECTRA_LOG"));     /* ivm APL */


   /* log file openen */
   if ((log = fopen(logfilenaam, "a")) != NULL)
   {
      /* systeem tijd aanduiding */
      time(&t);
      fprintf(log, "%s", ctime(&t));

      /* de message */
      fprintf(log, "%s\n",  bericht);

      /* scheidingsregel */
      for (i = 0; i < 100; i++)
         fprintf(log, "*");

      fprintf(log, "\n");

      fclose(log);

   } /* if ((log = fopen(logfilenaam, "a")) != NULL) */


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                        Bepaal_Suspect_Station                             */
/*                                                                           */
/*****************************************************************************/
int Bepaal_Suspect_Station(const char* sensorcode, char* flag)
{
   /* indien een sensorcode aanwezig in de file dan SUSPECT (anders blijft default OK staan) */

   FILE* in;
   char dir_en_suspect_filenaam[512];
   char line[256];
   char volledig_path[512];
   char info[1024];


   /* openen input file */
   strcpy(dir_en_suspect_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_suspect_filenaam, "suspect\\");
   else
      /*strcpy(dir_en_suspect_filenaam, "suspect/");*/
      strcpy(dir_en_suspect_filenaam, getenv("ENV_SPECTRA_SUSPECT"));

   strcat(dir_en_suspect_filenaam, "suspect.txt");


   if ((in = fopen(dir_en_suspect_filenaam, "r")) == NULL)              /* dus mislukt */
   {
      getcwd(volledig_path, 512);

      /* bericht samen stellen */
      strcpy(info, "\0");

      if (strcmp(OS, "WINDOWS") == 0)
      {
         strcat(info, volledig_path);      /* de environment dir var geeft al een volledig path onder unix */
         strcat(info, "\\");
      }

      strcat(info, dir_en_suspect_filenaam);
      strcat(info, " niet te lezen\n");

      /* naar log schrijven */
      Write_Log(info);

   } /* if ((in = fopen(dir_en_suspect_filenaam, "r")) == NULL) */
   else /* dus suspect.txt goed geopend */
   {
      while (fgets(line, 255, in) != NULL)
      {
         if (strlen(line) == 5)                                          /* strlen geeft lengte inclusief de '\n' */
         {
            if (strncmp(sensorcode, line, 4) == 0)
               strcpy(flag, SUSPECT_FLAG);
         } /* if (strlen(line)) == 5) */
      } /* while (fgets(line, 255, in) != NULL) */

   } /* else (dus suspect.txt goed geeopend) */


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                        Bepaal_ff_Reductie_Factor                          */
/*                                                                           */
/*****************************************************************************/
int Bepaal_ff_Reductie_Factor(const char* sensorcode, float* ff_factor)
{
   if (strncmp(sensorcode, "NC1 ", 4) == 0)
      *ff_factor = NC1_FF_FACTOR;
   else if (strncmp(sensorcode, "NC2 ", 4) == 0)
      *ff_factor = NC2_FF_FACTOR;
   else if (strncmp(sensorcode, "ANA1", 4) == 0)
      *ff_factor = ANA1_FF_FACTOR;
   else if (strncmp(sensorcode, "AUK1", 4) == 0)
      *ff_factor = AUK1_FF_FACTOR;
   else if (strncmp(sensorcode, "AUK2", 4) == 0)
      *ff_factor = AUK2_FF_FACTOR;
   else if (strncmp(sensorcode, "K131", 4) == 0)
      *ff_factor = K131_FF_FACTOR;
   else if (strncmp(sensorcode, "K132", 4) == 0)
      *ff_factor = K132_FF_FACTOR;
   else if (strncmp(sensorcode, "K133", 4) == 0)
      *ff_factor = K133_FF_FACTOR;
   else if (strncmp(sensorcode, "ELD1", 4) == 0)
      *ff_factor = ELD1_FF_FACTOR;
   else if (strncmp(sensorcode, "SMN1", 4) == 0)
      *ff_factor = SMN1_FF_FACTOR;
   else if (strncmp(sensorcode, "IJ51", 4) == 0)
      *ff_factor = IJ51_FF_FACTOR;
   else if (strncmp(sensorcode, "MUN1", 4) == 0)
      *ff_factor = MUN1_FF_FACTOR;
   else if (strncmp(sensorcode, "MUN2", 4) == 0)
      *ff_factor = MUN2_FF_FACTOR;
   else if (strncmp(sensorcode, "MPN1", 4) == 0)
      *ff_factor = MPN1_FF_FACTOR;
   else if (strncmp(sensorcode, "MPN2", 4) == 0)
      *ff_factor = MPN2_FF_FACTOR;
   else if (strncmp(sensorcode, "EPL2", 4) == 0)
      *ff_factor = EPL2_FF_FACTOR;
   else if (strncmp(sensorcode, "EPL3", 4) == 0)
      *ff_factor = EPL3_FF_FACTOR;
   else if (strncmp(sensorcode, "DWE1", 4) == 0)
      *ff_factor = DWE1_FF_FACTOR;
   else if (strncmp(sensorcode, "LEG1", 4) == 0)
      *ff_factor = LEG1_FF_FACTOR;
   else if (strncmp(sensorcode, "LEG2", 4) == 0)
      *ff_factor = LEG2_FF_FACTOR;
   else if (strncmp(sensorcode, "E131", 4) == 0)
      *ff_factor = E131_FF_FACTOR;
   else if (strncmp(sensorcode, "Q11 ", 4) == 0)
      *ff_factor = Q11_FF_FACTOR;
   else if (strncmp(sensorcode, "Q12 ", 4) == 0)
      *ff_factor = Q12_FF_FACTOR;
   else if (strncmp(sensorcode, "A121", 4) == 0)
      *ff_factor = A121_FF_FACTOR;
   else if (strncmp(sensorcode, "A122", 4) == 0)
      *ff_factor = A122_FF_FACTOR;
   else if (strncmp(sensorcode, "BG2b", 4) == 0)
      *ff_factor = BG2b_FF_FACTOR;
   else
      *ff_factor = 99999;


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                                Bepaal_Lat_Lon_Q                           */
/*                                                                           */
/*****************************************************************************/
int Bepaal_Lat_Lon_Q(const char* sensorcode, char* latitude, char* longitude, char* quadrant)
{
   if (strncmp(sensorcode, "NC1 ", 4) == 0)
   {
      strcpy(latitude, NC1_LAT);
      strcpy(longitude, NC1_LON);
      strcpy(quadrant, NC1_QUADRANT);
   }
   else if (strncmp(sensorcode, "NC2 ", 4) == 0)
   {
      strcpy(latitude, NC2_LAT);
      strcpy(longitude, NC2_LON);
      strcpy(quadrant, NC2_QUADRANT);
   }
   else if (strncmp(sensorcode, "ANA1", 4) == 0)
   {
      strcpy(latitude, ANA1_LAT);
      strcpy(longitude, ANA1_LON);
      strcpy(quadrant, ANA1_QUADRANT);
   }
   else if (strncmp(sensorcode, "AUK1", 4) == 0)
   {
      strcpy(latitude, AUK1_LAT);
      strcpy(longitude, AUK1_LON);
      strcpy(quadrant, AUK1_QUADRANT);
   }
   else if (strncmp(sensorcode, "AUK2", 4) == 0)
   {
      strcpy(latitude, AUK2_LAT);
      strcpy(longitude, AUK2_LON);
      strcpy(quadrant, AUK2_QUADRANT);
   }
   else if (strncmp(sensorcode, "K131", 4) == 0)
   {
      strcpy(latitude, K131_LAT);
      strcpy(longitude, K131_LON);
      strcpy(quadrant, K131_QUADRANT);
   }
   else if (strncmp(sensorcode, "K132", 4) == 0)
   {
      strcpy(latitude, K132_LAT);
      strcpy(longitude, K132_LON);
      strcpy(quadrant, K132_QUADRANT);
   }
   else if (strncmp(sensorcode, "K133", 4) == 0)
   {
      strcpy(latitude, K133_LAT);
      strcpy(longitude, K133_LON);
      strcpy(quadrant, K133_QUADRANT);
   }
   else if (strncmp(sensorcode, "ELD1", 4) == 0)
   {
      strcpy(latitude, ELD1_LAT);
      strcpy(longitude, ELD1_LON);
      strcpy(quadrant, ELD1_QUADRANT);
   }
   else if (strncmp(sensorcode, "SMN1", 4) == 0)
   {
      strcpy(latitude, SMN1_LAT);
      strcpy(longitude, SMN1_LON);
      strcpy(quadrant, SMN1_QUADRANT);
   }
   else if (strncmp(sensorcode, "IJ51", 4) == 0)
   {
      strcpy(latitude, IJ51_LAT);
      strcpy(longitude, IJ51_LON);
      strcpy(quadrant, IJ51_QUADRANT);
   }
   else if (strncmp(sensorcode, "MUN1", 4) == 0)
   {
      strcpy(latitude, MUN1_LAT);
      strcpy(longitude, MUN1_LON);
      strcpy(quadrant, MUN1_QUADRANT);
   }
   else if (strncmp(sensorcode, "MUN2", 4) == 0)
   {
      strcpy(latitude, MUN2_LAT);
      strcpy(longitude, MUN2_LON);
      strcpy(quadrant, MUN2_QUADRANT);
   }
   else if (strncmp(sensorcode, "MPN1", 4) == 0)
   {
      strcpy(latitude, MPN1_LAT);
      strcpy(longitude, MPN1_LON);
      strcpy(quadrant, MPN1_QUADRANT);
   }
   else if (strncmp(sensorcode, "MPN2", 4) == 0)
   {
      strcpy(latitude, MPN2_LAT);
      strcpy(longitude, MPN2_LON);
      strcpy(quadrant, MPN2_QUADRANT);
   }
   else if (strncmp(sensorcode, "EPL2", 4) == 0)
   {
      strcpy(latitude, EPL2_LAT);
      strcpy(longitude, EPL2_LON);
      strcpy(quadrant, EPL2_QUADRANT);
   }
   else if (strncmp(sensorcode, "EPL3", 4) == 0)
   {
      strcpy(latitude, EPL3_LAT);
      strcpy(longitude, EPL3_LON);
      strcpy(quadrant, EPL3_QUADRANT);
   }
   else if (strncmp(sensorcode, "DWE1", 4) == 0)
   {
      strcpy(latitude, DWE1_LAT);
      strcpy(longitude, DWE1_LON);
      strcpy(quadrant, DWE1_QUADRANT);
   }
   else if (strncmp(sensorcode, "LEG1", 4) == 0)
   {
      strcpy(latitude, LEG1_LAT);
      strcpy(longitude, LEG1_LON);
      strcpy(quadrant, LEG1_QUADRANT);
   }
   else if (strncmp(sensorcode, "LEG2", 4) == 0)
   {
      strcpy(latitude, LEG2_LAT);
      strcpy(longitude, LEG2_LON);
      strcpy(quadrant, LEG2_QUADRANT);
   }
   else if (strncmp(sensorcode, "E131", 4) == 0)
   {
      strcpy(latitude, E131_LAT);
      strcpy(longitude, E131_LON);
      strcpy(quadrant, E131_QUADRANT);
   }
   else if (strncmp(sensorcode, "Q11 ", 4) == 0)
   {
      strcpy(latitude, Q11_LAT);
      strcpy(longitude, Q11_LON);
      strcpy(quadrant, Q11_QUADRANT);
   }
   else if (strncmp(sensorcode, "Q12 ", 4) == 0)
   {
      strcpy(latitude, Q12_LAT);
      strcpy(longitude, Q12_LON);
      strcpy(quadrant, Q12_QUADRANT);
   }
   else if (strncmp(sensorcode, "A121", 4) == 0)
   {
      strcpy(latitude, A121_LAT);
      strcpy(longitude, A121_LON);
      strcpy(quadrant, A121_QUADRANT);
   }
   else if (strncmp(sensorcode, "A122", 4) == 0)
   {
      strcpy(latitude, A122_LAT);
      strcpy(longitude, A122_LON);
      strcpy(quadrant, A122_QUADRANT);
   }
   else if (strncmp(sensorcode, "BG2b", 4) == 0)
   {
      strcpy(latitude, BG2b_LAT);
      strcpy(longitude, BG2b_LON);
      strcpy(quadrant, BG2b_QUADRANT);
   }
   else
   {
      strcpy(latitude,  CIC_ONTBREKEND);
      strcpy(longitude, CIC_ONTBREKEND);
      strcpy(quadrant, CIC_ONTBREKEND);
   }


   return 0;
}


/*****************************************************************************/
/*                                                                           */
/*                        Check_Alternatieve_dd_ff                           */
/*                                                                           */
/*****************************************************************************/
int Check_Alternatieve_dd_ff(const char* sensorcode, char* dd, char* ff)
{
   /* sommige locaties hebben geen wind of er is een betere beschikbaar */
   /* dan een alternatief uit de wind file halen */

   FILE* in;
   char dir_en_wind_filenaam[512];
   char line[256];
   char volledig_path[512];
   char info[1024];
   char alternatief[6];
   char locatie_nummer[6];                          /* zoals in de wind file staat */
   int pos;


   /* initialisatie */
   strcpy(alternatief, "\0");
   strcpy(dd, "xx");                                /* kan door aanroepende function op getest worden */
   strcpy(ff, "xx");                                /* kan door aanroepende function op getest worden */

   /* de alternatieven voor de van toepassing zijnde locaties bepalen */
   if (strncmp(sensorcode, "ELD1", 4) == 0)
   {
      strncpy(alternatief, "06242", 5);
      alternatief[5] = '\0';
   }
   else if (strncmp(sensorcode, "SMN1", 4) == 0)
   {
      strncpy(alternatief, "06285", 5);
      alternatief[5] = '\0';
   }
   else if (strncmp(sensorcode, "MUN2", 4) == 0)
   {
      strncpy(alternatief, "06254", 5);
      alternatief[5] = '\0';
   }
   else if (strncmp(sensorcode, "MUN1", 4) == 0)
   {
      strncpy(alternatief, "06209", 5);
      alternatief[5] = '\0';
   }
   else
      strcpy(alternatief, "geen");


   /* alleen als er alternatief is verder gaan */
   if (strcmp(alternatief, "geen") != 0)
   {
      /* openen (wind)input file */
      strcpy(dir_en_wind_filenaam, "\0");

      if (strcmp(OS, "WINDOWS") == 0)
         strcpy(dir_en_wind_filenaam, "input_wind\\");
      else
         /*strcpy(dir_en_wind_filenaam, "input_wind/");*/
         strcpy(dir_en_wind_filenaam, getenv("ENV_SPECTRA_WIND"));

      strcat(dir_en_wind_filenaam, JJJJMMDDUU);
      strcat(dir_en_wind_filenaam, ".WIND");                            /* bv 2005051907.WIND */


      if ((in = fopen(dir_en_wind_filenaam, "r")) == NULL)              /* dus mislukt */
      {
         getcwd(volledig_path, 512);

         /* bericht samen stellen */
         strcpy(info, "\0");

         if (strcmp(OS, "WINDOWS") == 0)
         {
            strcat(info, volledig_path);      /* de environment dir var geeft al een volledig path onder unix */
            strcat(info, "\\");
         }

         strcat(info, dir_en_wind_filenaam);
         strcat(info, " niet te lezen\n");

         /* naar log schrijven */
         Write_Log(info);

      } /* if ((in = fopen(dir_en_wind_filenaam, "r")) == NULL) */
      else /* dus yyyymmdduu.wind goed geopend */
      {
         while (fgets(line, 255, in) != NULL)
         {
            if (strlen(line) == 36)                                          /* strlen geeft lengte inclusief de '\n' */
            {
               strncpy(locatie_nummer, line, 5);                             /* bv 06242 uit de wind file */
               if (strncmp(alternatief, locatie_nummer, 5) == 0)             /* match */
               {
                  /* dd */
                  strcpy(dd, "\0");
                  pos = 19;                                                   /* nb omdat het in de invoer ook zo staat kan je wel bv 02 krijgen (evert geeft dan 2) */
                  strncpy(dd, line + pos, 2);                                 /* dd was hier in tientallen graden = OK */
                  dd[2] = '\0';

                  /* ff */
                  strcpy(ff, "\0");
                  pos = 22;
                  strncpy(ff, line + pos, 2);                                 /* ff hier in hele knots -> OK  */
                  ff[2] = '\0';

                  /* nu kan het zijn dat er voor dd en/of ff "//" staat (dit dan doorgeven als "xx") */
                  if (strncmp(dd, "//", 2) == 0)
                  {
                      strcpy(dd, "\0");
                      strcpy(dd, "xx");                                       /* wordt op getest door aanroepende function */
                  }
                  if (strncmp(ff, "//", 2) == 0)
                  {
                      strcpy(ff, "\0");
                      strcpy(ff, "xx");
                  }

               } /* if (strncmp(alternatief, locatie_nummer, 5) == 0) */
            } /* if (strlen(line)) == 36) */
         } /* while (fgets(line, 255, in) != NULL) */

      } /* else (dus yyyymmdduu.wind.txt goed geeopend) */
   } /* if (strcmp(alternatief, "geen") != 0) */


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                            Bepaal_Diepte                                  */
/*                  (NB in gps_main.c staat ook zo'n function)               */
/*                                                                           */
/*****************************************************************************/
int Bepaal_Diepte(const char* sensorcode, float* DPT)
{
   if (strncmp(sensorcode, "NC1 ", 4) == 0)
      *DPT = NC1_DIEPTE;
   else if (strncmp(sensorcode, "NC2 ", 4) == 0)
      *DPT = NC2_DIEPTE;
   else if (strncmp(sensorcode, "ANA1", 4) == 0)
      *DPT = ANA1_DIEPTE;
   else if (strncmp(sensorcode, "AUK1", 4) == 0)
      *DPT = AUK1_DIEPTE;
   else if (strncmp(sensorcode, "AUK2", 4) == 0)
      *DPT = AUK2_DIEPTE;
   else if (strncmp(sensorcode, "K131", 4) == 0)
      *DPT = K131_DIEPTE;
   else if (strncmp(sensorcode, "K132", 4) == 0)
      *DPT = K132_DIEPTE;
   else if (strncmp(sensorcode, "K133", 4) == 0)
      *DPT = K133_DIEPTE;
   else if (strncmp(sensorcode, "ELD1", 4) == 0)
      *DPT = ELD1_DIEPTE;
   else if (strncmp(sensorcode, "SMN1", 4) == 0)
      *DPT = SMN1_DIEPTE;
   else if (strncmp(sensorcode, "IJ51", 4) == 0)
      *DPT = IJ51_DIEPTE;
   else if (strncmp(sensorcode, "MUN1", 4) == 0)
      *DPT = MUN1_DIEPTE;
   else if (strncmp(sensorcode, "MUN2", 4) == 0)
      *DPT = MUN2_DIEPTE;
   else if (strncmp(sensorcode, "MPN1", 4) == 0)
      *DPT = MPN1_DIEPTE;
   else if (strncmp(sensorcode, "MPN2", 4) == 0)
      *DPT = MPN2_DIEPTE;
   else if (strncmp(sensorcode, "EPL2", 4) == 0)
      *DPT = EPL2_DIEPTE;
   else if (strncmp(sensorcode, "EPL3", 4) == 0)
      *DPT = EPL3_DIEPTE;
   else if (strncmp(sensorcode, "DWE1", 4) == 0)
      *DPT = DWE1_DIEPTE;
   else if (strncmp(sensorcode, "LEG1", 4) == 0)
      *DPT = LEG1_DIEPTE;
   else if (strncmp(sensorcode, "LEG2", 4) == 0)
      *DPT = LEG2_DIEPTE;
   else if (strncmp(sensorcode, "E131", 4) == 0)
      *DPT = E131_DIEPTE;
   else if (strncmp(sensorcode, "Q11 ", 4) == 0)
      *DPT = Q11_DIEPTE;
   else if (strncmp(sensorcode, "Q12 ", 4) == 0)
      *DPT = Q12_DIEPTE;
   else if (strncmp(sensorcode, "A121", 4) == 0)
      *DPT = A121_DIEPTE;
   else if (strncmp(sensorcode, "A122", 4) == 0)
      *DPT = A122_DIEPTE;
   else if (strncmp(sensorcode, "BG2b", 4) == 0)
      *DPT = BG2b_DIEPTE;
   else
      *DPT = 10.0;


   return 0;
}





