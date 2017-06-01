#if !defined(gps_h)                         /* Sentry use file only if it's not already included. */
#include "spectra.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>                           /* voor sqrt */
#include <values.h>                         /* MAXINT, MAXFLOAT */

#if defined(WINDOWS)                        /* zie gps.h */
#include <dir.h>                            /* o.a. getcwd() in windows */
#else
#include <unistd.h>                         /* o.a. getcwd() in UNIX */
#include <dirent.h>
#endif


/* function prototypes in andere modules */
extern int Write_Log(char* bericht);                                                                  /* module: main */
extern int Bepaal_Lat_Lon_Q(const char* sensorcode, char* latitude, char* longitude, char* quadrant); /* module: main */
extern int Bepaal_ff_Reductie_Factor(const char* sensorcode, float* ff_factor);                       /* module: main */
extern int Bepaal_Suspect_Station(const char* sensorcode, char* flag);                                /* module: main */
extern int Check_Alternatieve_dd_ff(const char* sensorcode, char* dd, char* ff);                      /* module: main */
extern int Bepaal_Diepte(const char* sensorcode, float* DPT);                                         /* module: main */

/* functions in deze module */
int Read_SOVF81_Input_Files(void);
int Bepaal_SOVF81_Filenamen(void);
int Read_SOVF81_Write_CIC_LFR(const char* cic_filenaam, const char* lfr_filenaam);
int Bepaal_CID_en_LFD_Filenamen(char* cid_filenaam, char* lfd_filenaam);
int Read_SOVF81_Write_CID_LFD(const char* cid_filenaam,  const char* lfd_filenaam);
double Compute_Wave_Speed(double f, double d);

/* externe var's */
extern char JJJJMMDDUU[11];                        /* via argument list */
extern char OS[8];

/* var's globaal binnen deze module */
char input_files[AANTAL_INPUT_FILES][LENGTE_INPUT_FILENAAM];





/*****************************************************************************/
/*                                                                           */
/*                          Read_SOVF81_Input_Files                          */
/*                                                                           */
/*****************************************************************************/
int Read_SOVF81_Input_Files()
{
   char cid_filenaam[256];
   char lfd_filenaam[256];


   Bepaal_SOVF81_Filenamen();                                        /* input files */
   Bepaal_CID_en_LFD_Filenamen(cid_filenaam, lfd_filenaam);          /* output files */
   Read_SOVF81_Write_CID_LFD(cid_filenaam, lfd_filenaam);


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                              Bepaal_SOVF81_filenamen                      */
/*                                                                           */
/*****************************************************************************/
int Bepaal_SOVF81_Filenamen()
{
   /* inputfilenaam b.v.: MSS_R098123256_644_SOVF81_EHDB_081200 (totaal: 37 char) */




#if defined(WINDOWS)
   /* hier WINDOWS dir search algoritme implementeren */
   /* ............................................... */



#else
   DIR *pdir;
   struct dirent *pent;
   int pos;
   int i;
   int file_teller;
   char dag_uur_filenaam[5];
   char dag_uur_JJJJMMDDUU[5];


   /* initialisatie */
   for (i = 0; i < AANTAL_INPUT_FILES; i++)
      strcpy(input_files[i], "\0");


   pdir = opendir("input_sovf81/");                       /*"." refers to the current dir */
   if (pdir)
   {
      file_teller = 0;                                   /* initialisatie */
      while ((pent = readdir(pdir)))
      {
         /* nieuwe filenaam */

         /* MSS_R098123256_644_SOVF81_EHDB_081200 */
         /* dus de DDUU (dag uur, hier 0812) is bepalend dit vergelijken met de DDUU uit var. JJJJMMDDUU */

         /* iniialisatie */
         strcpy(dag_uur_filenaam, "\0");
         strcpy(dag_uur_JJJJMMDDUU, "\0");

         pos = 31;
         strncpy(dag_uur_filenaam, pent -> d_name + pos, 4);               /* bv 0812 */


         pos = 6;
         strncpy(dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 4);                /* bv 0812 */

         if (strncmp(dag_uur_filenaam, dag_uur_JJJJMMDDUU, 4) == 0)
         {
            if (file_teller < AANTAL_INPUT_FILES)                         /* voor de veiligheid */
            {
               /* goede file (voor dag-uur) gevonden */
               strcpy(input_files[file_teller], pent -> d_name);
               input_files[file_teller][LENGTE_INPUT_FILENAAM -1] = '\0'; /* voor de zekerheid goed afsluiten */
               file_teller++;

            } /* if (file_teller < AANTAL_INPUT_FILES) */

         } /* if (strncmp(dag_uur_filenaam, dag_uur_JJJJMMDDUU, 4) == 0) */
      } /* while ((pent = readdir(pdir))) */
   } /* if (pdir) */
   else
   {
      Write_Log("sovf81 opendir() failure");
      exit(1);
   } /* else */

#endif


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                          Read_SOVF81_Write_CID_LFD                        */
/*                                                                           */
/*****************************************************************************/
int Read_SOVF81_Write_CID_LFD(const char* cid_filenaam, const char* lfd_filenaam)
{
   /*char buffer[40];*/
   int i;
   int k;
   int a;
   int line_teller;
   int hulp_lengte;
   int pos;
   int hulp_num_dd;
   int hulp_num_ff;
   int num_aantal_fouten;
   int num_aantal_meetwaarden;
   int num_aantal_meetwaarden_methode_1;
   int num_aantal_meetwaarden_methode_2;
   int num_aantal_meetwaarden_methode_3;
   int checks_ok;
   int Hm0_array[10];
   int Fm01_array[10];
   int Th0_array[10];
   int num_Ts;
   int Hm0_Fm01_geldig;                         /* boolean */
   int Th0_geldig;                              /* boolean */
   int ddd;
   int all_swell;                               /* boolean */
   int NWWAVE;
   int LWWAVE;                                  /* boolean */

   double verschilhoek;
   double verschilhoek_array_rad[10];
   double m0_array[10];
   double m0_totaal;
   double m_1_totaal;
   double ETOT;
   double EX;
   double EY;
   double TMSW;
   double hulp_THSW;
   double THSW;
   double HSSW;
   double hulp_energie_PHILLC;
   double hulp_pow_PHILLC;
   double PHILLC;
   double U;
   /*double WNR0; */
   /*double DPTRM;*/
   double CPH;
   double UCOS;
   double DFR;

   float ff_factor;
   float DPT;

   FILE* out_cid;
   FILE* out_lfd;
   FILE* in;

   char volledig_path[512];
   char info[1024];
   char line[256];
   char dir_en_cid_filenaam[512];
   char dir_en_lfd_filenaam[512];
   char dir_en_input_filenaam[512];
   char char_hulp_waarde[20];
   char hulp_ddd[20];                            /* deg */
   char hulp_ff[20];                             /* ff nog niet gecorrigeerd voor hoogte */
   char sovf81_array[AANTAL_SOVF81][20];
   char dd_cid[20];
   char ff_cid[20];
   char dd_lfd[20];
   char ff_lfd[20];
   char Ts[20];
   char HsSW[20];
   char TmSW[20];
   char thSW[20];
   char dd_alternatief[3];
   char ff_alternatief[3];

   /* voor .CID en .LFD output (NB 20 array grootte is gewoon een willekeurig groot genoeg getal) */
   char sensorcode[20];
   char flag[20];
   char jaar_dag_uur_JJJJMMDDUU[20];

   /* alleen voor CID output */
   char latitude[20];
   char longitude[20];
   char quadrant[20];

   /* alleen voor .LFD output */
   char aantal_fouten[20];
   char aantal_meetwaarden[20];




   /* dir en naam .cid output file bepalen */
   strcpy(dir_en_cid_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_cid_filenaam, "output_cid\\");
   else
      /* strcpy(dir_en_cid_filenaam, "output_cid/"); */
      strcpy(dir_en_cid_filenaam, getenv("ENV_SPECTRA_CID"));     /* ivm APL */
   strcat(dir_en_cid_filenaam, cid_filenaam);


   /* dir en naam .lfd output file bepalen */
   strcpy(dir_en_lfd_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_lfd_filenaam, "output_lfd\\");
   else
      /* strcpy(dir_en_lfd_filenaam, "output_lfd/"); */
      strcpy(dir_en_lfd_filenaam, getenv("ENV_SPECTRA_LFD"));     /* ivm APL */
   strcat(dir_en_lfd_filenaam, lfd_filenaam);


   /* openen cid en lfd output files */
   if ( ((out_cid = fopen(dir_en_cid_filenaam, "w")) == NULL) || ((out_lfd = fopen(dir_en_lfd_filenaam, "w")) == NULL) )  /* dus mislukt */
   {
      getcwd(volledig_path, 512);

      /* bericht samen stellen */
      strcpy(info, "\0");

      if (strcmp(OS, "WINDOWS") == 0)
      {
         strcat(info, volledig_path);      /* de environment dir var geeft al een volledig path onder unix */
         strcat(info, "\\");
      }

      if (out_cid == NULL)
      {
         strcat(info, dir_en_cid_filenaam);
         strcat(info, " ");
      }
      if (out_lfd == NULL)
         strcat(info, dir_en_lfd_filenaam);

      strcat(info, " niet te schrijven\n");

      /* naar log schrijven */
      Write_Log(info);

      /* nu zou het kunnen dat een van de 2 wel goed was, deze dan sluiten om geheel correct te zijn */
#if 0            /* geeft cor dump */

      if (out_cid != NULL)
         fclose(out_cid);

      if (out_lfd != NULL)
         fclose(out_lfd);
#endif
   } /* if ((out = fopen(dir_en_cid_filenaam, "w")) == NULL) etc.  */
   else /* outputfiles zijn beide te schrijven */
   {
      /* cid output */

      /* cid file heeft een aantal kopregels */
      fprintf(out_cid, "%s\n", " Wave Directional Data from Measuring Network North Sea (CIC-HMR)");
      fprintf(out_cid, "%s\n", " ================================================================");
      fprintf(out_cid, "\n");
      fprintf(out_cid, "\n");
      fprintf(out_cid, "%s", " Output file:   /.../");
      fprintf(out_cid, "%s", cid_filenaam);
      fprintf(out_cid, "%s\n", "                        ");                /* 24 extra spaties voor erfenis (geeft dan dezelfde regellengte als Evert altijd had) */
      fprintf(out_cid, "\n");
      fprintf(out_cid, "\n");
      fprintf(out_cid, "\n");
      fprintf(out_cid, "%s\n", "    DTG   NAME  QI    LONG.   LAT.      dd      ff      Ts      Hs    thwa      fp    Hs10  thHs10    HsSW    thSW    TmSW FLAG");
      fprintf(out_cid, "%s\n", "                                    [10 deg] [knots] [0.1*s]  [cm]   [deg]   [mHz]    [cm]   [deg]    [cm]   [deg] [0.1*s]");
      fprintf(out_cid, "%s\n", " ===============================================================================================================================");
      fprintf(out_cid, "\n");

      fclose(out_cid);                                          /* file is nu weer leeg */
      out_cid = fopen(dir_en_cid_filenaam, "a");                /* moet nu telkens aangevuld (append worden) */

      /* lfd output */
      fclose(out_lfd);                                          /* file is nu weer leeg */
      out_lfd = fopen(dir_en_lfd_filenaam, "a");                /* moet nu telkens aangevuld (append worden) */


      /* openen input file(s) */
      for (i = 0; i < AANTAL_INPUT_FILES; i++)
      {
         if (strcmp(input_files[i], "\0") != 0)
         {
            /* initialisatie cid + lfd */
            strcpy(sensorcode,   "\0");
            strcpy(hulp_ddd,     "\0");                                 /* 1 deg */
            checks_ok = 0;                                              /* false */

            /* initialisatie specifiek cid */
            strcpy(flag,         "\0");
            strcpy(dd_cid,       "\0");                                 /* 10 deg */
            strcpy(ff_cid,       "\0");                                 /* knots */

            /* NB initialisatie met "0" voor lfd komt voort uit de wijze hoe Evert dit deed (ook 0 in lfd indien niet aanwezig)
            /*    bij dd en ff moet indien ontbrekend dit 99 (dd) en 99 (ff) worden ! */

            /* initialisatie specifiek lfd */
            strcpy(dd_lfd,       "\0");                                 /* 10 deg */
            strcpy(ff_lfd,       "\0");                                 /* knots */

            /* initialisatie */
            for (k = 0; k < AANTAL_SOVF81; k++)
               strcpy(sovf81_array[k], LFD_ONTBREKEND);

            /* initialisatie */
            strcpy(aantal_fouten,      LFD_ONTBREKEND);
            strcpy(aantal_meetwaarden, LFD_ONTBREKEND);

            /* initialisatie */
            for (a = 0; a < 10; a++)
            {
               Hm0_array[a]  = 0;                                       /* hulp-array deze op 0 zetten */
               Fm01_array[a] = 0;                                       /* hulp-array deze op 0 zetten */
               m0_array[a]   = 0;                                       /* hulp-array deze op 0 zetten */
            } /* for (a = 0; a < 15; a++) */


            /* dus een geldige file naam aanwezig in array input_files */
            line_teller = 0;

            /* input file namem + dir bepalen */
            strcpy(dir_en_input_filenaam, "\0");

            if (strcmp(OS, "WINDOWS") == 0)
               strcpy(dir_en_input_filenaam, "input_sovf81\\");
            else
               strcpy(dir_en_input_filenaam, "input_sovf81/");

            strcat(dir_en_input_filenaam, input_files[i]);


            if ((in = fopen(dir_en_input_filenaam, "r")) != NULL)             /* gelukt */
            {
               while (fgets(line, 255, in) != NULL)
               {
                  line_teller++;                                              /* dus de eerste gelezen regel = no 1 */

                  /*
                  //////////////// sensorcode
                  */
                  if (line_teller == 5)
                  {
                     if (strlen(line) >= 4)                                         /* stations naam */
                     {
                        strcpy(sensorcode, line);

                        /* sensorcode kan 3 of 4 char lang zijn (NC1, K131)*/
                        if (strlen(sensorcode) == 4)                                /* strlen telt WEL '\n' */
                        {
                           sensorcode[3] = '\0';
                           strcat(sensorcode, " ");
                        } /* if (strlen(sensorcode) == 4) */

                        sensorcode[4] = '\0';

                     } /* if (strlen(line) >= 4) */
                     else
                        strcpy(sensorcode, "xxxx");

                  } /* if (line_teller == 5) */


                  /*
                  ////////////// Alle records sovf81 inlezen
                  */

                  /* sommige array plaatsen blijven leeg (die bv die niet met "G " beginnen of lengte < 4 hebben) */
                  /* NB bv sensorcode is ook apart (buiten dit sovf81_array om) ingelezen */
                  /* NB er staat dus eigenlijk meer in dan later wordt weggeschreven naar lfd file */

                  if (line_teller <= AANTAL_SOVF81)                       /* veiligheid (array grens van array_sovf81) */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)               /* geeft lengte inclusief '\n' */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                  /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(char_hulp_waarde, line + pos);          /* de "G " overslaan */
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';  /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* de scheefheid (G1) kan buiten format (4 char plaatsen) komen daarom hierop testen */
                           if ( (line_teller == 59)  || (line_teller == 66)  || (line_teller == 73) ||
                                (line_teller == 80)  || (line_teller == 87)  || (line_teller == 95) ||
                                (line_teller == 102) || (line_teller == 109) || (line_teller == 116) ||
                                (line_teller == 123) )
                           {
                              if (atoi(char_hulp_waarde) < -999)
                                 strcpy(char_hulp_waarde, "-999");
                              else if (atoi(char_hulp_waarde) > 9999)
                                 strcpy(char_hulp_waarde, "9999");
                           }

                           strcpy(sovf81_array[line_teller -1], char_hulp_waarde); /* line_teller begint bij 1, array indexen bij 0 */
                        } /* if (strncmp(line, "G ", 2) == 0) */
                     } /* if ((hulp_lengte = strlen(line) == 4) */
#if 0
                     //hulp_lengte = strlen(line);                         /* geeft lengte inclusief '\n' */
                     //strcpy(char_hulp_waarde, line);
                     //char_hulp_waarde[hulp_lengte -1] = '\0';            /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                     //strcpy(sovf81_array[line_teller -1], char_hulp_waarde); /* indices beginnen op 0, line_teller op 1 */
#endif
                  } /* if (line_teller <= AANTAL_SOVF81) */



                  /*
                  //////////// dd en ff
                  */

                  /* ff */
                  if (line_teller == 127)
                  {
                     /* ff (input is niet gecorrigeerd voor hoogte; moet dus  gecorrigeerd worden) */
                     if ((hulp_lengte = strlen(line)) >= 3)         /* bv "G3'\n'" (dus inclusief '\n') */
                     {
                        Bepaal_ff_Reductie_Factor(sensorcode, &ff_factor);
                        if ( (strncmp(line, "G", 1) == 0) &&  (strncmp(line, "G99999", 6) != 0) && (ff_factor != 99999) )   /* regel moet beginnen met "G" */
                        {
                           pos = 1;
                           strcpy(hulp_ff, line + pos);                    /* de "G" niet in lijstje */
                           hulp_ff[hulp_lengte -1 -pos] = '\0';            /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           hulp_num_ff = (int)(atof(hulp_ff) / ff_factor + 0.5);
                           sprintf(ff_cid, "%d", hulp_num_ff);
                           sprintf(ff_lfd, "%d", hulp_num_ff);

                        } /* if ( (strncmp(line, "G", 1) == 0) && (ff_factor != 99999) ) */
                        else
                        {
                           strcpy(ff_cid, CID_ONTBREKEND);
                           strcpy(ff_lfd, FF_LFD_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) >= 3) */
                     else
                     {
                        strcpy(ff_cid, CID_ONTBREKEND);
                        strcpy(ff_lfd, FF_LFD_ONTBREKEND);
                     }
                  } /* if (line_teller == 127) */

                  /* dd */
                  if (line_teller == 128)
                  {
                     if ((hulp_lengte = strlen(line)) == 6)         /* bv "G143='\n'" (dus inclusief '\n') */
                     {
                        /* hulp_ddd in graden, moet dd in tientallen graden worden */
                        if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G99999", 6) != 0) )  /* regel moet beginnen met "G" */
                        {
                           pos = 1;
                           strcpy(hulp_ddd, line + pos);                    /* de "G" niet in lijstje */
                           hulp_ddd[hulp_lengte -1 -pos] = '\0';            /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           hulp_num_dd = (int)(atof(hulp_ddd) / 10 + 0.5);
                           sprintf(dd_cid, "%2d", hulp_num_dd);
                           sprintf(dd_lfd, "%2d", hulp_num_dd);

                        } /* if (strncmp(line, "G", 1) == 0) */
                        else
                        {
                           strcpy(dd_cid, CID_ONTBREKEND);
                           strcpy(dd_lfd, DD_LFD_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) == 6) */
                     else
                     {
                         strcpy(dd_cid, CID_ONTBREKEND);
                         strcpy(dd_lfd, DD_LFD_ONTBREKEND);
                     }
                  } /* if (line_teller == 128) */


                  /* controle waarden (checks) (op positie 128 moet "wind" staan, dan is alles wel goed gegaan)*/
                  if (line_teller == 126)                                 /* label: "wind" */
                  {
                     if ((hulp_lengte = strlen(line)) == 5)               /* geeft lengte inclusief '\ n' */
                     {
                        if (strncmp(line, "wind", 4) == 0)
                           checks_ok = 1;                                 /* true */
                        else
                           checks_ok = 0;
                     }
                     else
                        checks_ok = 0;
                  } /* if (line_teller == 54) */


               } /* while (fgets(line, 255, in) != NULL) */

               /* controle: moeten precies 129 regels gelezen zijn */
               if ( (checks_ok == 1) && (line_teller == 129) )
                  checks_ok = 1;                                 /* true */
               else
                  checks_ok = 0;




               /*
               //
               /////////////////////////   afgeleide waarden buiten while lus houden) //////////////////////
               //
               */


               /*
               // Ndir_R = sovf81_array[6]     aantal werkelijk gebruikte deelreeksen in golfrichtingspectrumbepaling
               // Ngd_xP = sovf81_array[7]     % goed binnengekomen Dx-punten (OW) t.o.v. v/h totale aantal verwachte punten
               // Ngd_yP = sovf81_array[8]     % goed binnengekomen Dy-punten (NZ) t.o.v. v/h totale aantal verwachte punten
               // Nd_x   = sovf81_array[9]     aantal vanwege delta-fout afgekeurde Dx-punten
               // Nu_x   = sovf81_array[10]
               // Nv_x   = sovf81_array[11]
               // Nd_y   = sovf81_array[13]
               // Nu_y   = sovf81_array[14]
               // Nv_y   = sovf81_array[15]
               //
               */

               /*
               //////////////// aantal fouten (Nd_x + Nu_x + Nv_x + Nd_y + Nu_y + Nv_y) [AFGELEIDE WAARDE]
               //// (NB kan niet testen op LFD_ontbrekend (=0) i.v.m. voorkomen 0 waarde)
               */
               if ( (strcmp(sovf81_array[9], "99999") != 0) && (strcmp(sovf81_array[13], "99999") != 0) &&
                    (strcmp(sovf81_array[10], "99999") != 0) && (strcmp(sovf81_array[14], "99999") != 0) &&
                    (strcmp(sovf81_array[11], "99999") != 0) && (strcmp(sovf81_array[15], "99999") != 0) )
               {
                  num_aantal_fouten = atoi(sovf81_array[9]) + atoi(sovf81_array[13]) +
                                      atoi(sovf81_array[10]) + atoi(sovf81_array[14]) +
                                      atoi(sovf81_array[11]) + atoi(sovf81_array[15]);
                  sprintf(aantal_fouten, "%d", num_aantal_fouten);
               }
               else
                  strcpy(aantal_fouten, LFD_ONTBREKEND);

               /*
               //////////////// aantal meetwaarden  [AFGELEIDE WAARDE]
               //// (NB kan niet testen op LFD_ontbrekend (=0) i.v.m. voorkomen 0 waarde)
               */
               if ( (strcmp(sovf81_array[6], "99999") != 0) && (strcmp(sovf81_array[7], "99999") != 0) && (strcmp(sovf81_array[8], "99999") != 0))
               {
                  num_aantal_meetwaarden_methode_1 = atoi(sovf81_array[6]) * 256;          /* 256 lengte deelreeks (Ndir_R is maximaal: 6) */
                  num_aantal_meetwaarden_methode_2 = (int)(atof(sovf81_array[7]) * 6 * 256 / 100 + 0.5);  /* 100 i.v.m. percentage */
                  num_aantal_meetwaarden_methode_3 = (int)(atof(sovf81_array[8]) * 6 * 256 / 100 + 0.5);  /* 100 i.v.m. percentage */

                  /* in overleg met Evert op 23-05-2005 afgesproken de laagste waarde te nemen */
                  if (num_aantal_meetwaarden_methode_1 < num_aantal_meetwaarden_methode_2)
                  {
                     if (num_aantal_meetwaarden_methode_1 < num_aantal_meetwaarden_methode_3)
                        num_aantal_meetwaarden = num_aantal_meetwaarden_methode_1;
                     else
                        num_aantal_meetwaarden = num_aantal_meetwaarden_methode_3;
                  }
                  else
                  {
                     if (num_aantal_meetwaarden_methode_2 < num_aantal_meetwaarden_methode_3)
                        num_aantal_meetwaarden = num_aantal_meetwaarden_methode_2;
                     else
                        num_aantal_meetwaarden = num_aantal_meetwaarden_methode_3;
                  } /* else */

                  sprintf(aantal_meetwaarden, "%d", num_aantal_meetwaarden);
               }
               else
                  strcpy(aantal_meetwaarden, LFD_ONTBREKEND);




               /*
               /////////// dd, ff ALTERNATIEF (van een andere locatie dan uit de sovf input file)
               */
               Check_Alternatieve_dd_ff(sensorcode, dd_alternatief, ff_alternatief);
               if (strncmp(dd_alternatief, "xx", 2) != 0)                     /* dus er is een alternatief */
               {
                  /* alternatief gebruiken (dus de orginele eerder opgehaalde overschrijven) */
                  strcpy(dd_cid,       "\0");                                 /* 10 deg */
                  strcpy(ff_cid,       "\0");                                 /* knots */
                  strcpy(dd_lfd,       "\0");                                 /* 10 deg */
                  strcpy(ff_lfd,       "\0");                                 /* knots */

                  strcpy(dd_cid, dd_alternatief);
                  strcpy(dd_lfd, dd_alternatief);

                  strcpy(ff_cid, ff_alternatief);
                  strcpy(ff_lfd, ff_alternatief);

               } /* if (strncmp(alternatieve_dd, "xx", 2) != 0) */


               /*
               ////////////////// Ts
               */
               /* G1 - G10 zijn de GONO banden, B0 - B4 zijn soort samenvattingen/afgeleiden hiervan */
               /* Hm0 = 4 * sqrt(mo) -> mo = 1/16 * Hm0**2  (1/16 = 0.0625) */
               /* Tm0-1 (Ts) = m-1 / m0(totaal) */

               /* Hm0 per frequentie band */
               Hm0_array[0] = atoi(sovf81_array[54]);   /* cm */         /* HM0_G1, sovf81 regel 55 */
               Hm0_array[1] = atoi(sovf81_array[61]);                    /* HM0_G2, sovf81 regel 62 */
               Hm0_array[2] = atoi(sovf81_array[68]);                    /* HM0_G3, sovf81 regel 69 */
               Hm0_array[3] = atoi(sovf81_array[75]);                    /* HM0_G4, sovf81 regel 76 */
               Hm0_array[4] = atoi(sovf81_array[82]);                    /* HM0_G5, sovf81 regel 83 */

               Hm0_array[5] = atoi(sovf81_array[90]);                    /* HM0_G6, sovf81 regel 91 */
               Hm0_array[6] = atoi(sovf81_array[97]);                    /* HM0_G7, sovf81 regel 98 */
               Hm0_array[7] = atoi(sovf81_array[104]);                   /* HM0_G8, sovf81 regel 105 */
               Hm0_array[8] = atoi(sovf81_array[111]);                   /* HM0_G9, sovf81 regel 112 */
               Hm0_array[9] = atoi(sovf81_array[118]);                   /* HM0_G10,sovf81 regel 119 */

               /* Fm01 per frequentie band */
               Fm01_array[0] = atoi(sovf81_array[60]);  /* mHz */        /* Fm01_G1, sovf81 regel 61 */
               Fm01_array[1] = atoi(sovf81_array[67]);                   /* Fm01_G2, sovf81 regel 68 */
               Fm01_array[2] = atoi(sovf81_array[74]);                   /* Fm01_G3, sovf81 regel 75 */
               Fm01_array[3] = atoi(sovf81_array[81]);                   /* Fm01_G4, sovf81 regel 82 */
               Fm01_array[4] = atoi(sovf81_array[88]);                   /* Fm01_G5, sovf81 regel 89 */

               Fm01_array[5] = atoi(sovf81_array[96]);                   /* Fm01_G6, sovf81 regel 97 */
               Fm01_array[6] = atoi(sovf81_array[103]);                  /* Fm01_G7, sovf81 regel 104 */
               Fm01_array[7] = atoi(sovf81_array[110]);                  /* Fm01_G8, sovf81 regel 111 */
               Fm01_array[8] = atoi(sovf81_array[117]);                  /* Fm01_G9, sovf81 regel 118 */
               Fm01_array[9] = atoi(sovf81_array[124]);                  /* Fm01_G10, sovf81 regel 125 */


               /* testen op 99999 alles moet OK zijn (geen 99999) */
               Hm0_Fm01_geldig = 1;                                          /* in principe geldig */
               for (a = 0; a < 10; a++)                                      /* voor de 10 GONO bandjes */
               {
                  /* Hm0 per frequentie band kan wel 0 zijn, Fm01 kan nooit 0 zijn */
                  if ( (Hm0_array[a] == 99999) || (Fm01_array[a] == 99999) || (Fm01_array[a] == 0))
                  {
                     Hm0_Fm01_geldig = 0;                                    /* ongeldig */
                     break;                                                  /* uit for lus springen */
                  }
               } /* for (a = 0; a < 15; a++) */


               /* alleen nu Ts berekenen als alles ok was (geen 99999 in een v/d array-waarden) */
               if (Hm0_Fm01_geldig == 1)
               {
                  for (a = 0; a < 10; a++)                                   /* 10 (gono) bandjes */
                  {
                     /* mo = energie inhoud (NB HMo stond in cm -> m) */
                     m0_array[a] = 0.0625 * ((double)Hm0_array[a] / 100) * ((double)Hm0_array[a] / 100); /* Hm0 = 4 * sqrt(mo) -> mo = 1/16 * Hm0**2  (1/16 = 0.0625) */
                  } /* for (a = 0; a < 10; a++) */

                  m_1_totaal = 0.0;
                  m0_totaal  = 0.0;
                  for (a = 0; a < 10; a++)                                   /* voor de gono bandjes (indices 0 t/m 9) */
                  {
                     if (Fm01_array[a] != 0)                                 /* om delen door 0 te voorkomen */
                     {
                        m_1_totaal = m_1_totaal + (m0_array[a] / ((double)Fm01_array[a] / 1000)); /* Fmo1 staat in milli- Hz -> Hz */
                        m0_totaal  = m0_totaal + m0_array[a];
                     }
                  } /* for (a = 0; a < 10; a++) */


                  if ( (m_1_totaal > 0.0) && (m0_totaal > 0.0) )
                  {
                     num_Ts = 10 * (m_1_totaal / m0_totaal) + 0.5;        /* 10 omdat Ts in tienden seconden moet staan */
                     sprintf(Ts, "%d", num_Ts);
                  }
                  else
                     strcpy(Ts, CID_ONTBREKEND);
               } /* if (Hm0_Fm01_geldig == 1) */
               else
                  strcpy(Ts, CID_ONTBREKEND);

               /*
               // volgens Evert: vergelijk van golfsnelheid en richting met wind d.m.v. WAM criterium
               //
               // U.cos(THMDD) < Cph -> deining
               //
               // Cph = 1.56 * DPTRM / Fmo1(band)                 (Cph = fasesnelheid)
               // DPTRM = sqrt(tanh ko. diepte)                   (diepteterm)  k0 = golfgetal / wavenumber
               //
               // k0 = 2 pi / golflengte = 4.024 (Fm01(band))**2  (diep water golf getal)
               //
               //
               // 01-11-2005:NB volgens Vladimir Makin zitten hier boven 2 fouten in n.l. de sqrt is niet correct
               //               en er moet een iteratie voor ko bij
               */

               /*
               /////////////////////// HsSw (hoogte swell) , TmSw (periode swell) en thSW (swell richting)
               */

               /* initialisaties */
               for (a = 0; a < 10; a++)                                    /* alleen voor de 10 GONO bandjes */
               {
                  Th0_array[a] = 0;                                        /* hulp array */
                  verschilhoek_array_rad[a] = 0;                           /* hulp array */
               } /* for (a = 0; a < 10; a++) */

               /* initialisatie */
               NWWAVE = 0;              /* number wind waves */
               LWWAVE = 0;              /* boolean; LWWAVE = Logical Wind Waves */
               ETOT   = 0.0;
               EX     = 0.0;
               EY     = 0.0;
               TMSW   = 0.0;
               THSW   = 0.0;
               hulp_THSW = 0.0;

               /* golfrichting van de 10 GONO bandjes */
               Th0_array[0] = atoi(sovf81_array[56]);  /* graden */         /* Th0_G1, sovf81 regel 57 */
               Th0_array[1] = atoi(sovf81_array[63]);                       /* Th0_G2, sovf81 regel 64 */
               Th0_array[2] = atoi(sovf81_array[70]);                       /* Th0_G3, sovf81 regel 71 */
               Th0_array[3] = atoi(sovf81_array[77]);                       /* Th0_G4, sovf81 regel 78 */
               Th0_array[4] = atoi(sovf81_array[84]);                       /* Th0_G5, sovf81 regel 85 */

               Th0_array[5] = atoi(sovf81_array[92]);                       /* Th0_G6, sovf81 regel 93 */
               Th0_array[6] = atoi(sovf81_array[99]);                       /* Th0_G7, sovf81 regel 100 */
               Th0_array[7] = atoi(sovf81_array[106]);                      /* Th0_G8, sovf81 regel 107 */
               Th0_array[8] = atoi(sovf81_array[113]);                      /* Th0_G9, sovf81 regel 114 */
               Th0_array[9] = atoi(sovf81_array[120]);                      /* Th0_G10,sovf81 regel 121 */

               /* alle golfrichtingen (Th0) moeten OK zijn, geen 99999 */
               Th0_geldig = 1;                                              /* in principe geldig */
               for (a = 0; a < 10; a++)
               {
                  if ( (Th0_array[a] == 99999) || (Th0_array[a] == 0) )     /* wordt veel gedeeld door Th0_array[a] */
                  {
                     Th0_geldig = 0;                                        /* Th0 ongeldig maken */
                     break;                                                 /* uit for lus springen */
                  }
               } /* for (a = 0; a < 10; a++) */


               /* heeft alleen zin bij reeele basis waarden */
               if ( (atoi(dd_cid) > 0) && (atoi(dd_cid) <= 36) && (Th0_geldig == 1) && (atoi(ff_cid) < 200) && (Hm0_Fm01_geldig == 1) )
               {
                  ddd = atoi(dd_cid) * 10;                                  /* stond in tientallen graden */

                  /* verschilhoeken golven-wind berekenen */
                  for (a = 0; a < 10; a++)                                  /* alleen voor de 10 gono bandjes */
                  {
                     verschilhoek = Th0_array[a] - ddd;

                     if (verschilhoek > 180)
                        verschilhoek -= 360;

                     if (verschilhoek < -180 && verschilhoek >= -360)
                        verschilhoek += 360;

                     verschilhoek_array_rad[a] = verschilhoek / 57.296;        /* nu in radialen */

                  } /* for (a = 0; a < 10; a++) */


                  /* ff omzetten van knots -> m/s */
                  U = 0.514 * atof(ff_cid);

                  /* diepte ophalen (afhangkelijk van locatie/sensorcode) */
                  Bepaal_Diepte(sensorcode, &DPT);                                      /* diepte in meters */

                  /* soms kan er direct al gezegd worden dat alles swell is (swell at low wind speeds, any directions) */
                  if (U == 0.0)
                     all_swell = 1;
                  else if ( (1.0 / U) > (((double)Fm01_array[9] / 1000) / 1.3) ) /* Fm01_array[9] = van Gono band no. 10 (Fm01_G10 = 335 - 500mHz) */
                     all_swell = 1;
                  else
                     all_swell = 0;

                  /* voor de 10 GONO bandjes swell delen bepalen */
                  for (a = 0; a < 10; a++)
                  {
                     if (all_swell == 0)                                      /* niet alles is swell */
                     {
#if 0
//                        /* Vladimir Makin 01-11-2005: onderstaand is niet correct, deze vevangen door subroutine */
//                        /* Compute_Wave_Speed (de sqrt was fout had ik zelf ook al opgemerkt) maar er moest */
//                        /* dus ook nog een iteratie bij */
//                        /* commentaar van Evert Bouws (bedenker van origineel: is wel goed was een nauwkeurige benaderings formule !!*/
//
//                        WNR0 = 4.024 * ((double)Fm01_array[a] / 1000) * ((double)Fm01_array[a] / 1000); /* WNR0 = wave number deep water */
//                        DPTRM = sqrt(tanh(WNR0 * (double)DPT));               /* DPTRM = wave number benaderd voor ondiep water */
//                        CPH = 1.56 * DPTRM / ((double)Fm01_array[a] / 1000);  /* CPH = fase snelheid */
#endif
                        /* Bovenstaande is vervangen door: */
                        CPH = Compute_Wave_Speed((double)Fm01_array[a] / 1000, (double)DPT);

                        UCOS = 1.2 * U * cos(verschilhoek_array_rad[a]);

                        /* criterium voor swell: U * cos(verschilhoek) < Cph */
                        if (UCOS > CPH)                /* dus dan wind waves */
                        {
                           NWWAVE++;

                           if (NWWAVE > 3)              /* i.v.m. incidenten moet dit wel minimaal 3x voorkomen */
                              LWWAVE = 1;
                        }
                        else /* (UCOS <= CPH) */
                        {
                           /* additional wind wave criterium, based on var. density */
                           if (a > 0 && a < 9)
                           {
                              DFR = 0.5 * (((double)Fm01_array[a + 1] / 1000) - ((double)Fm01_array[a - 1] / 1000));  /* DFR = bandbreedte */

                              hulp_energie_PHILLC = m0_array[a] / DFR;
                              hulp_pow_PHILLC     = pow(((double)Fm01_array[a] / 1000), 5);

                              PHILLC = hulp_energie_PHILLC * hulp_pow_PHILLC;
                              if (PHILLC > 0.0005)       /* 5 * 10-4 */
                                 LWWAVE = 1;                                    /* LWWAVE = (logical) Wind Wave */
                           } /* if (a > 0 && a < 9) */

                           /* high-freq swell not allowed with lf wind waves */
                           if (LWWAVE == 0)
                           {
                              EX = EX + m0_array[a] * cos((double)Th0_array[a] / 57.296);
                              EY = EY + m0_array[a] * sin((double)Th0_array[a] / 57.296);
                              ETOT = ETOT + m0_array[a];
                              TMSW = TMSW + m0_array[a] / ((double)Fm01_array[a] / 1000);
                           } /* if (LWWAVE == 0) */

                        } /* else ((UCOS <= CPH) */

                     } /* if (all_swell == 0) */
                     else /* all_swell == 1 (true) */
                     {
                        EX = EX + m0_array[a] * cos((double)Th0_array[a] / 57.296);
                        EY = EY + m0_array[a] * sin((double)Th0_array[a] / 57.296);
                        ETOT = ETOT + m0_array[a];
                        TMSW = TMSW + m0_array[a] / ((double)Fm01_array[a] / 1000);
                     } /* else (all_swell == 1) */

                  } /* for (a = 0; a < 10; a++) */

                  /* height swell */
                  if (ETOT > 0.0)
                  {
                     HSSW = 4 * sqrt(ETOT);
                     sprintf(HsSW, "%d", (int)(HSSW * 100 + 0.5));      /* in centimeters */
                  }

                  /* period swell */
                  if (ETOT > 0.0)
                  {
                     TMSW = TMSW / ETOT;
                     sprintf(TmSW, "%d", (int)(TMSW * 10 + 0.5));       /* in seconden */
                  }
                  else
                     strcpy(TmSW, CID_ONTBREKEND);

                  /* richting swell */
                  if ( (EX != 0.0) && (EY != 0.0) )
                  {
                     THSW = atan2(EY, EX);

                     hulp_THSW = THSW * 57.296;                        /* omzetten naar graden */
                     if (hulp_THSW < 0.0)
                        hulp_THSW += 360.0;

                     sprintf(thSW, "%d", (int)(hulp_THSW + 0.5));
                  }
                  else
                     strcpy(thSW, CID_ONTBREKEND);

               } /* if (dd_cid > 0 && dd_cid <= 36) etc. */
               else /* swell data undefined */
               {
                  strcpy(HsSW, CID_ONTBREKEND);
                  strcpy(thSW, CID_ONTBREKEND);
                  strcpy(TmSW, CID_ONTBREKEND);
               }  /* swell data undefined */

               fclose(in);


               /*
               ////////////////// SUSPECT of OK
               */
               strcpy(flag, OK_FLAG);                                    /* default */
               Bepaal_Suspect_Station(sensorcode, flag);                 /* kan hier SUSPECT worden */


            } /* if ((in = fopen(dir_en_input_filenaam, "r")) != NULL) */



            /*********************************************************************/
            /*                                                                   */
            /*           schrijven naar LFD output file (was al geopend)         */
            /*                                                                   */
            /*********************************************************************/
            /* NB in de lfd files wordt ontbrekend een 0 (dit is dus anders dan in de cid file) */

            /* alleen als er een HM0 instaat (indien geen HM0 dan nooit iets bijzonders verder per definitie) */
            /*        7654321: (ONTBREKEND) wordt door dit programma er ingezet als er geen waarde voor HM0 staat */
            /*          99999: staat soms in de input files als er (tijdelijk ?) geen data voor HM0 is */
            /* NB HM0 = sovf81_array[18] */
            if ( (strncmp(flag, OK_FLAG, 7) == 0) && (checks_ok == 1) && (atoi(aantal_meetwaarden) > 0) && (strcmp(sovf81_array[18], LFD_ONTBREKEND) != 0) && (strcmp(sovf81_array[18], "99999") != 0) )
            {
               /* checks (via Evert) HM0    > 0   (indien niet: dan alle waarden uit de waarnemingen ongeldig) */
               /*                     Ts    < 999 (indien niet: dan alle waarden uit de waarnemingen ongeldig) */
               /* NB 12-07-2005 afgesproken niet meer op Ts te testen voor al of niet wegschrijven */
               if ( (atoi(sovf81_array[18]) > 0) /*&& (atoi(Ts) < 999)*/ )
               {
                  /* sensorcode (zonder "NZ" ervoor) */
                  fprintf(out_lfd, "%4.4s", sensorcode);

                  /* spatie */
                  fprintf(out_lfd, "%1.1s", " ");

                  /* datum (zonder eeuw aanduiding) */
                  strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
                  pos = 2;
                  strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
                  jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
                  fprintf(out_lfd, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* dd */
                  fprintf(out_lfd, "%3.3s", dd_lfd);

                  /* ff */
                  fprintf(out_lfd, "%2.2s", ff_lfd);

                  /* kwaliteiscode (altijd 1?) */
                  fprintf(out_lfd, "%2.2s", "1");

                  /* statuscode (uit LFr2 sovf81) altijd 0, erfenis wordt niet meer gebruikt */
                  fprintf(out_lfd, "%2.2s", "0");

                  /* statuscode (uit LFr2 sovf81) altijd 0, erfenis wordt niet meer gebruikt */
                  fprintf(out_lfd, "%1.1s", "0");

                  /* statuscode (uit LFr2 sovf81) altijd 0, erfenis wordt niet meer gebruikt */
                  fprintf(out_lfd, "%1.1s", "0");

                  /* statuscode (uit LFr2 sovf81) altijd 0, erfenis wordt niet meer gebruikt */
                  fprintf(out_lfd, "%1.1s", "0");

                  /* spatie */
                  fprintf(out_lfd, "%1.1s", " ");

                  /* [punt 1 Annex Bx] code (ongebruikt), erfenis altijd 0 */
                  fprintf(out_lfd, "%4.4s", "0");

                  /* [punt 2 Annex Bx] statuscode */
                  fprintf(out_lfd, "%4.4s", "0");

                  /* [punt 3 Annex Bx] aantal fouten */
                  fprintf(out_lfd, "%4.4s", aantal_fouten);

                  /* [punt 4 Annex Bx] code (ongebruikt) */
                  fprintf(out_lfd, "%4.4s", "0");

                  /* [punt 5 Annex Bx] aantal meetwaarden */
                  fprintf(out_lfd, "%4.4s", aantal_meetwaarden);

                  /* [punt 6-18 Annex bx] 13 x code (ongebruikt) */
                  for (a = 0; a < 13; a++)
                     fprintf(out_lfd, "%4.4s", "0");

                  /* GONO band G1 (30-45 mHz) sovf81: regel 55 t/m 61 */
                  for (a = 54; a < 61; a++)                         /* regel 55 -> indice 54 */
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G2 (45-60 mHz) sovf81: regel 62 t/m 68 */
                  for (a = 61; a < 68; a++)                         /* regel 62 -> indice 61 */
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G3 (60-85 mHz) sovf81: regel 69 t/m 75 */
                  for (a = 68; a < 75; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G4 (85-100 mHz) sovf81: regel 76 t/m 82 */
                  for (a = 75; a < 82; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G5 (100-125 mHz) sovf81: regel 83 t/m 89 */
                  for (a = 82; a < 89; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G6 (125-165 mHz) sovf81: regel 91 t/m 97 */
                  for (a = 90; a < 97; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G7 (165-200 mHz) sovf81: regel 98 t/m 104 */
                  for (a = 97; a < 104; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G8 (200-250 mHz) sovf81: regel 105 t/m 111 */
                  for (a = 104; a < 111; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G9 (250-335 mHz) sovf81: regel 112 t/m 118 */
                  for (a = 111; a < 118; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* GONO band G10 (335-500 mHz) sovf81: regel 119 t/m 125 */
                  for (a = 118; a < 125; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* Band Bo (30-500 mHz; regel 19 t/m 25) */
                  for (a = 18; a < 25; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* Band B4 (spec. peak; regel 47 t/m 53) */
                  for (a = 46; a < 53; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* Band B1 (200-500 mHz; regel 26 t/m 32) */
                  for (a = 25; a < 32; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* Band B2 (100-200 mHz; regel 33 t/m 39) */
                  for (a = 32; a < 39; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);

                  /* Band B3 (30-100 mHz; regel 40 t/m 46) */
                  for (a = 39; a < 46; a++)
                     fprintf(out_lfd, "%4.4s", sovf81_array[a]);


                  /* code (ongebruikt), erfenis altijd 0 */
                  fprintf(out_lfd, "%4.4s", "0");

                  /* code (ongebruikt), erfenis altijd 0 */
                  fprintf(out_lfd, "%4.4s", "0");

                  /* reserve, altijd 0 (erfenis) */
                  fprintf(out_lfd, "%4.4s", "0");

                  /* reserve, altijd 0 (erfenis) */
                  fprintf(out_lfd, "%4.4s", "0");


                  /* nieuwe regel */
                  fprintf(out_lfd, "\n");

               } /* if ( (atoi(sovf81_array[18]) > 0) && (atoi(Ts) < 999) ) */
            } /* if ( (checks_ok == 1) && (strcmp(sovf81_array[18], LFD_ONTBREKEND) != 0) && etc. */





            /*********************************************************************/
            /*                                                                   */
            /*           schrijven naar CID output file (was al geopend)         */
            /*                                                                   */
            /*********************************************************************/

            /* alleen als er een HM0 instaat (indien geen HM0 dan nooit iets bijzonders verder per definitie) */
            /*        7654321: (ONTBREKEND) wordt door dit programma er ingezet als er geen waarde voor HM0 staat */
            /*          99999: staat soms in de input files als er (tijdelijk ?) geen data voor HM0 is */


            /* specifiek CID_ontbrekend zetten (stonden op LFD_ONTBREKEND) */
            for (k = 0; k < AANTAL_SOVF81; k++)
            {
               if (strcmp(sovf81_array[k], LFD_ONTBREKEND) == 0)        /* LFD_ONTBREKEND = "0" */
               {
                  strcpy(sovf81_array[k], "\0");
                  strcpy(sovf81_array[k], CID_ONTBREKEND);              /* CID_ONTBREKEND = " 7654321" */
               }
            } /* for (k = 0; k < AANTAL_SOVF81; k++) */



/************* TEST ***************/
           /*    fprintf(stderr, "%s", sovf81_array[18]);
               getchar();
           */

/************* TEST ***************/


            /* NB HM0 = sovf81_array[18] */
            if ( (checks_ok == 1) && (atoi(aantal_meetwaarden) > 0) && (strcmp(sovf81_array[18], CID_ONTBREKEND) != 0) && (strcmp(sovf81_array[18], "99999") != 0) )
            {
               /* checks (via Evert) HM0    > 0   (dan alle waarden uit de waarnemingen ongeldig) */
               /*                     Ts < 999 (dan alle waarden uit de waarnemingen ongeldig) */
               /* NB 12-07-2005 afgesproken niet meer op Ts te testen voor al of niet wegschrijven */
               if ( (atoi(sovf81_array[18]) > 0) /*&& (atoi(Ts) < 999)*/ )
               {
                  /* spatie (overblijfsel uit Fortran tijdperk) */
                  fprintf(out_cid, "%1.1s", " ");

                  /* datum (zonder eeuw aanduiding) */
                  strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
                  pos = 2;
                  strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
                  jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
                  fprintf(out_cid, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* sensorcode met "NZ" ervoor */
                  fprintf(out_cid, "%3.3s", "NZ");
                  fprintf(out_cid, "%4.4s", sensorcode);

                  /* Latitude, Longitude and Quadrant */
                  Bepaal_Lat_Lon_Q(sensorcode, latitude, longitude, quadrant);
                  fprintf(out_cid, "%2.2s", quadrant);
                  fprintf(out_cid, "%8.8s", longitude);
                  fprintf(out_cid, "%8.8s", latitude);

                  /* dd */
                  fprintf(out_cid, "%8.8s", dd_cid);

                  /* ff */
                  fprintf(out_cid, "%8.8s", ff_cid);

                  /* Ts */
                  fprintf(out_cid, "%8.8s", Ts);

                  /* hs (in sovf81: HM0_B0, regel 19) */
                  fprintf(out_cid, "%8.8s", sovf81_array[18]);

                  /* thwa (in sovf81: Th0_B0, regel 21) */
                  fprintf(out_cid, "%8.8s", sovf81_array[20]);

                  /* fp (in sovf81: Fm0_B4, regel 53) */
                  fprintf(out_cid, "%8.8s", sovf81_array[52]);

                  /* Hs10 (in sovf81: Hm0_B3, regel 40) */
                  fprintf(out_cid, "%8.8s", sovf81_array[39]);

                  /* thHs10 (in sovf81: Th0_B3, regel 42) */
                  fprintf(out_cid, "%8.8s", sovf81_array[41]);

                  /* HsSW (afgeleide waarde) */
                  fprintf(out_cid, "%8.8s", HsSW);

                  /* thSW (afgeleide waarde) */
                  fprintf(out_cid, "%8.8s", thSW);

                  /* TmSW (afgeleide waarde) */
                  fprintf(out_cid, "%8.8s", TmSW);

                  /* flag */
                  /*strcpy(flag, OK_FLAG);                    */                /* default */
                  /*Bepaal_Suspect_Station(sensorcode, flag); */                /* kan hier SUSPECT worden */
                  fprintf(out_cid, "%8.8s", flag);

                  /* nieuwe regel */
                  fprintf(out_cid, "\n");

               } /* if ( (HM0 > 0) && (Ts < 999) ) */
            } /* if ( (checks_ok == 1) && (strcmp(HM0, ONTBREKEND) != 0) || (strcmp(HM0, "99999") != 0) ) */

         } /* if (strcmp(input_files[i], "\0") != 0) */
      } /* for (i = 0; i < AANTAL_INPUT_FILES; i++) */

      fclose(out_cid);
      fclose(out_lfd);

   } /* else (outputfiles zijn te schrijven) */


   return 0;
}





/*****************************************************************************/
/*                                                                           */
/*                        Bepaal_CID_en_LFD_Filenamen                        */
/*                                                                           */
/*****************************************************************************/
int Bepaal_CID_en_LFD_Filenamen(char* cid_filenaam, char* lfd_filenaam)
{
   /* CID filenaam voorbeeld: WAVE_CID_200504081200_00000_LC */
   /* LFD filenaam voorbeeld: WAVE_LFD_200504081200_00000_LC */

   /* initialisatie */
   strcpy(cid_filenaam, "\0");
   strcpy(lfd_filenaam, "\0");

   /* CID */
   strcpy(cid_filenaam, "WAVE_CID_");
   strcat(cid_filenaam, JJJJMMDDUU);
   strcat(cid_filenaam, "00_00000_LC");

   /* LFD */
   strcpy(lfd_filenaam, "WAVE_LFD_");
   strcat(lfd_filenaam, JJJJMMDDUU);
   strcat(lfd_filenaam, "00_00000_LC");


   /* ////// TEST ///////////
   fprintf(stderr, "%s", cid_filenaam);
   getchar();
    ////// TEST ///////////*/

   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                               Compute_Wave_Speed                          */
/*                                                                           */
/*****************************************************************************/
double Compute_Wave_Speed(double f, double d)
{
   /* overgenomen van function[cp_s, cp_d] = shallow(f, d) van Vladimir Makin */
   /* phase speed in shallow water as function of freq. in Hz */

   double g = 9.8;
   double wp;
   double k0;
   double k;
   double eps;
   double t;
   double cp_s;


   wp = 2 * PI * f;
   k0 = wp * wp / g;

   eps = 1;
   k = k0;
   while (eps > 0.000001)
   {
      t = tanh(d * k);
      eps = fabs(k0 - k * t);
      k = k0 / t;
   } /* while (eps > 0.000001) */

   cp_s = wp / k;


   /* ook de diep water wave speed kan nog berekend worden */
   /* kp_s = wp / cp_s;                    */
   /* cp_d = wp / (kp_s * tanh(kp_s * d)); */


   return cp_s;                           /* phase speed shallow water */
}







