#if !defined(gps_h)                         /* Sentry use file only if it's not already included. */
#include "spectra.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>                           /* voor sqrt */

#if defined(WINDOWS)                        /* zie gps.h */
#include <dir.h>                            /* o.a. getcwd() in windows */
#else
#include <unistd.h>                         /* o.a. getcwd() in UNIX */
#include <dirent.h>
#endif


/* function prototypes in andere module */
extern int Write_Log(char* bericht);                                                                   /* module: main */
extern int Bepaal_Suspect_Station(const char* sensorcode, char* flag);                                 /* module: main */
extern int Bepaal_Lat_Lon_Q(const char* sensorcode, char* latitude, char* longitude, char* quadrant);  /* module: main */
extern int Bepaal_ff_Reductie_Factor(const char* sensorcode, float* ff_factor);                        /* module: main */
extern int Check_Alternatieve_dd_ff(const char* sensorcode, char* dd, char* ff);                       /* module: main */

/* functions in deze module */
int Read_SOVF80_Input_Files(void);
int Bepaal_SOVF80_Filenamen(void);
int Read_SOVF80_Write_CIC_LFR(const char* cic_filenaam, const char* lfr_filenaam);
int Bepaal_CIC_en_LFR_Filenamen(char* cic_filenaam, char* lfr_filenaam);


/* externe var's */
extern char JJJJMMDDUU[11];                        /* via argument list */
extern char OS[8];

/* var's globaal binnen deze module */
char input_files[AANTAL_INPUT_FILES][LENGTE_INPUT_FILENAAM];





/*****************************************************************************/
/*                                                                           */
/*                          Read_SOVF80_Input_Files                          */
/*                                                                           */
/*****************************************************************************/
int Read_SOVF80_Input_Files()
{
   char cic_filenaam[256];
   char lfr_filenaam[256];


   Bepaal_SOVF80_Filenamen();                                        /* input files */
   Bepaal_CIC_en_LFR_Filenamen(cic_filenaam, lfr_filenaam);          /* output files */
   Read_SOVF80_Write_CIC_LFR(cic_filenaam, lfr_filenaam);


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                              Bepaal_SOVF80_filenamen                      */
/*                                                                           */
/*****************************************************************************/
int Bepaal_SOVF80_Filenamen()
{
   /* inputfilenaam b.v.: MSS_R098123256_644_SOVF80_EHDB_081200 (totaal: 37 char) */




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


   pdir = opendir("input_sovf80/");                       /*"." refers to the current dir */
   if (pdir)
   {
      file_teller = 0;                                   /* initialisatie */
      while ((pent = readdir(pdir)))
      {
         /*printf("%s", pent->d_name);*/
         /*  ////////////// TEST ////////////// */
         /* Write_Log(pent -> d_name); */
         /*  ////////////// TEST ////////////// */

         /* nieuwe filenaam */

         /* MSS_R098123256_644_SOVF80_EHDB_081200 */
         /* dus de DDUU (dag uur, hier 0812) is bepalend dit vergelijken met de DDUU uit var. JJJJMMDDUU */

         /* iniialisatie */
         strcpy(dag_uur_filenaam, "\0");
         strcpy(dag_uur_JJJJMMDDUU, "\0");

         pos = 31;
         strncpy(dag_uur_filenaam, pent -> d_name + pos, 4);               /* bv 0812 */

         /* ////// TEST ///////////
         fprintf(stderr, "%s", dag_uur_filenaam);
         getchar();
         ////// TEST ///////////*/



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

               /*  ////////////// TEST ////////////// */
               /*Write_Log(pent -> d_name);*/
               /*  ////////////// TEST ////////////// */

            } /* if (file_teller < AANTAL_INPUT_FILES) */

            //rename
         } /* if (strncmp(dag_uur_filenaam, dag_uur_JJJJMMDDUU, 4) == 0) */
      } /* while ((pent = readdir(pdir))) */
   } /* if (pdir) */
   else
   {
      /*printf ("opendir() failure; terminating");*/
      Write_Log("sovf80 opendir() failure");
      exit(1);
   } /* else */

#endif


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                          Read_SOVF80_Write_CIC_LFR                        */
/*                                                                           */
/*****************************************************************************/
int Read_SOVF80_Write_CIC_LFR(const char* cic_filenaam, const char* lfr_filenaam)
{
   int i;
   int k;
   int m;
   int line_teller;
   int hulp_lengte;
   int pos;
   int pos_old;
   int hulp_num_dd;
   int hulp_num_ff;
   int czz5_array_teller;
   int czz5_line_teller;
   int czz10_array_teller;
   int czz10_line_teller;
   int num_aantal_fouten;
   int num_aantal_meetwaarden;
   int num_aantal_meetwaarden_methode_1;
   int num_aantal_meetwaarden_methode_2;
   int num_ghc_interpol_7_9;
   int num_ghc_interpol_9_11;
   int num_ghc_interpol_11_13;
   int num_hulp_waarde;
   int Czz5_ok;
   int Czz10_ok;

   float ff_factor;
   double double_hulp_waarde;

   FILE* out_cic;
   FILE* out_lfr;
   FILE* in;

   char volledig_path[512];
   char info[1024];
   char line[256];
   char dir_en_cic_filenaam[512];
   char dir_en_lfr_filenaam[512];
   char dir_en_input_filenaam[512];
   char char_hulp_waarde[20];
   char hulp_ddd[20];                            /* deg */
   char hulp_ff[20];                             /* ff nog niet gecorrigeerd voor hoogte */

   /* NB 20 array grootte is gewoon een willekeurig groot genoeg getal) */
   char sensorcode[20];
   char TE1[20];                                 /* cm**2 */        /* HFE (HE1) */
   char TE2[20];                                 /* cm**2 */        /* MFE (HE2) */
   char TE3[20];                                 /* cm**2 */        /* LFE (HE3) */
   char HM0_cic[20];                             /* cm */
   char Tm_10_cic[20];                           /* 0.1*s */
   char Tm_10_lfr[20];                           /* 0.1*s */
   char Hs7_cic[20];                             /* cm */
   char Hs7_lfr[20];                             /* mm */
   char dd_cic[20];                              /* 10 deg */
   char ff_cic[20];                              /* knots */
   char flag[20];
   char jaar_dag_uur_JJJJMMDDUU[20];
   char latitude[20];
   char longitude[20];
   char quadrant[20];
   char czz5_array[AANTAL_CZZ5][20];
   char czz10_array[AANTAL_CZZ10][20];
   char aantal_fouten[20];
   char aantal_meetwaarden[20];
   char AV10_H[20];
   char Fp[20];
   char Tm02[20];
   char aantal_golven[20];
   char H1d3[20];
   char H1d10[20];
   char H1d50[20];
   char GGh[20];
   char Hmax[20];
   char T1d3[20];
   char TH1d3[20];
   char GGT[20];
   char Tmax[20];
   char THmax[20];
   char Ndir_H[20];
   char Ngd_zP[20];
   char Nd_z[20];
   char Nu_z[20];
   char Nv_z[20];
   char Ni_z[20];
   char ghc_interpol_7_9[20];
   char ghc_interpol_9_11[20];
   char ghc_interpol_11_13[20];
   char HM0_lfr[20];                             /* mm */
   char dd_lfr[20];
   char ff_lfr[20];
   char TE0[20];
   char HE0[20];
   char HE1[20];
   char HE2[20];
   char HE3[20];
   char dd_alternatief[3];
   char ff_alternatief[3];


   /* dir en naam .cic output file bepalen */
   strcpy(dir_en_cic_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_cic_filenaam, "output_cic\\");
   else
      /* strcpy(dir_en_cic_filenaam, "output_cic/"); */
      strcpy(dir_en_cic_filenaam, getenv("ENV_SPECTRA_CIC"));     /* ivm APL */
   strcat(dir_en_cic_filenaam, cic_filenaam);


   /* dir en naam .lfr output file bepalen */
   strcpy(dir_en_lfr_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_lfr_filenaam, "output_lfr\\");
   else
      /* strcpy(dir_en_lfr_filenaam, "output_lfr/"); */
      strcpy(dir_en_lfr_filenaam, getenv("ENV_SPECTRA_LFR"));     /* ivm APL */
   strcat(dir_en_lfr_filenaam, lfr_filenaam);


   /* openen cic en lfr output files */
   if ( ((out_cic = fopen(dir_en_cic_filenaam, "w")) == NULL) || ((out_lfr = fopen(dir_en_lfr_filenaam, "w")) == NULL) )  /* dus mislukt */
   {
      getcwd(volledig_path, 512);

      /* bericht samen stellen */
      strcpy(info, "\0");

      if (strcmp(OS, "WINDOWS") == 0)
      {
         strcat(info, volledig_path);      /* de environment dir var geeft al een volledig path onder unix */
         strcat(info, "\\");
      }

      if (out_cic == NULL)
      {
         strcat(info, dir_en_cic_filenaam);
         strcat(info, " ");
      }
      if (out_lfr == NULL)
         strcat(info, dir_en_lfr_filenaam);

      strcat(info, " niet te schrijven\n");

      /* naar log schrijven */
      Write_Log(info);

      /* nu zou het kunnen dat een van de 2 wel goed was, deze dan sluiten om geheel correct te zijn */
#if 0            /* geeft cor dump */

      if (out_cic != NULL)
         fclose(out_cic);

      if (out_lfr != NULL)
         fclose(out_lfr);
#endif
   } /* if ((out = fopen(dir_en_cic_filenaam, "w")) == NULL) etc.  */
   else /* outputfiles zijn beide te schrijven */
   {
      /* cic output */
      fclose(out_cic);                                          /* file is nu weer leeg */
      out_cic = fopen(dir_en_cic_filenaam, "a");                /* moet nu telkens aangevuld (append worden) */

      /* lfr output */
      fclose(out_lfr);                                          /* file is nu weer leeg */
      out_lfr = fopen(dir_en_lfr_filenaam, "a");                /* moet nu telkens aangevuld (append worden) */


      /* openen input file(s) */
      for (i = 0; i < AANTAL_INPUT_FILES; i++)
      {
         if (strcmp(input_files[i], "\0") != 0)
         {
            /* initialisatie (voor lfr) */
            Czz5_ok  = 1;                                       /* true */
            Czz10_ok = 1;                                       /* true */

            /* initialisatie cic + lfr */
            strcpy(sensorcode, "\0");
            strcpy(hulp_ddd,   "\0");                                   /* deg */

            /* initialisatie specifiek cic */
            strcpy(TE1,          "\0");                                 /* cm**2 */
            strcpy(TE2,          "\0");                                 /* cm**2 */
            strcpy(TE3,          "\0");                                 /* cm**2 */
            strcpy(HM0_cic,      "\0");                                 /* cm */
            strcpy(Tm_10_cic,    "\0");                                 /* 0.1*s */   /* Ts */
            strcpy(Hs7_cic,      "\0");                                 /* cm */
            strcpy(flag,         "\0");
            strcpy(dd_cic,       "\0");                                 /* 10 deg */
            strcpy(ff_cic,       "\0");                                 /* knots */

            /* NB initialisatie met "0" voor lfr komt voort uit de wijze hoe Evert dit deed (ook 0 in lfr indien niet aanwezig)
            /*    bij dd en ff moet indien ontbrekend dit 99 (dd) en 99 (ff) worden ! */

            /* initialisatie specifiek LFR */
            for (k = 0; k < AANTAL_CZZ5; k++)
               strcpy(czz5_array[k], LFR_ONTBREKEND);

            for (k = 0; k < AANTAL_CZZ10; k++)
               strcpy(czz10_array[k], LFR_ONTBREKEND);

            strcpy(ghc_interpol_7_9,   LFR_ONTBREKEND);
            strcpy(ghc_interpol_9_11,  LFR_ONTBREKEND);
            strcpy(ghc_interpol_11_13, LFR_ONTBREKEND);

            /*strcpy(kwaliteitscode,     "\0");*/
            strcpy(aantal_fouten,      LFR_ONTBREKEND);
            strcpy(aantal_meetwaarden, LFR_ONTBREKEND);
            strcpy(AV10_H,             LFR_ONTBREKEND);
            strcpy(TE0,                LFR_ONTBREKEND);
            strcpy(Fp,                 LFR_ONTBREKEND);
            strcpy(Tm02,               LFR_ONTBREKEND);
            strcpy(aantal_golven,      LFR_ONTBREKEND);
            strcpy(H1d3,               LFR_ONTBREKEND);
            strcpy(H1d10,              LFR_ONTBREKEND);
            strcpy(GGh,                LFR_ONTBREKEND);
            strcpy(Hmax,               LFR_ONTBREKEND);
            strcpy(T1d3,               LFR_ONTBREKEND);
            strcpy(TH1d3,              LFR_ONTBREKEND);
            strcpy(GGT,                LFR_ONTBREKEND);
            strcpy(Tmax,               LFR_ONTBREKEND);
            strcpy(THmax,              LFR_ONTBREKEND);
            strcpy(Ndir_H,             LFR_ONTBREKEND);
            strcpy(Ngd_zP,             LFR_ONTBREKEND);
            strcpy(Nd_z,               LFR_ONTBREKEND);
            strcpy(Nu_z,               LFR_ONTBREKEND);
            strcpy(Nv_z,               LFR_ONTBREKEND);
            strcpy(Ni_z,               LFR_ONTBREKEND);
            strcpy(HM0_lfr,            LFR_ONTBREKEND);
            strcpy(dd_lfr,             LFR_ONTBREKEND);                                 /* 10 deg */
            strcpy(ff_lfr,             LFR_ONTBREKEND);                                 /* knots */
            strcpy(TE0,                "\0");
            strcpy(HE0,                LFR_ONTBREKEND);
            strcpy(HE1,                LFR_ONTBREKEND);
            strcpy(HE2,                LFR_ONTBREKEND);
            strcpy(HE3,                LFR_ONTBREKEND);
            strcpy(Tm_10_lfr,          LFR_ONTBREKEND);                                 /* 0.1*s */   /* Ts, TM0-1 */
            strcpy(Hs7_lfr,            LFR_ONTBREKEND);                                 /* mm */
            strcpy(H1d50,              LFR_ONTBREKEND);


            /* dus een geldige file naam aanwezig in array input_files */
            line_teller = 0;

            /* input file namem + dir bepalen */
            strcpy(dir_en_input_filenaam, "\0");

            if (strcmp(OS, "WINDOWS") == 0)
               strcpy(dir_en_input_filenaam, "input_sovf80\\");
            else
               strcpy(dir_en_input_filenaam, "input_sovf80/");

            strcat(dir_en_input_filenaam, input_files[i]);


            if ((in = fopen(dir_en_input_filenaam, "r")) != NULL)             /* gelukt */
            {
               while (fgets(line, 255, in) != NULL)
               {
                  line_teller++;                                             /* dus de eerste gelezen regel = no 1 */


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

                  } /* if (line_teller == `5) */


                  /*
                  //////////////// Ndir_H (aantal werkelijk gebruikte deelreeksen in de golfhoogtespectrumbepaling)
                  */

                  if (line_teller == 7)                            /* Ndir_H */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )      /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Ndir_H, line + pos);                   /* de "G " niet in lijstje */
                           Ndir_H[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Ndir_H, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Ndir_H, LFR_ONTBREKEND);
                  } /* if (line_teller == 7) */


                  /*
                  //////////////// Ngd_zP (% goed binnengekomen golfhoogte puntent.o.v. totaal aantal verwachte punten)
                  */
                  if (line_teller == 8)                            /* Ngd_zP */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )    /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Ngd_zP, line + pos);                   /* de "G " niet in lijstje */
                           Ngd_zP[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Ngd_zP, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Ngd_zP, LFR_ONTBREKEND);
                  } /* if (line_teller == 8) */


                  /*
                  //////////////// Nd_z (aantal vanwegw delta-fout afgekeurde golfhoogtepunten)
                  */
                  if (line_teller == 9)                            /* Ndir_H */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Nd_z, line + pos);                   /* de "G " niet in lijstje */
                           Nd_z[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Nd_z, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Nd_z, LFR_ONTBREKEND);
                  } /* if (line_teller == 9) */


                  /*
                  //////////////// Nv_z (aantal vanwege 4-sigma-fout afgekeurde golfhoogtepunten)
                  */
                  if (line_teller == 10)                            /* Nv_Z */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Nv_z, line + pos);                   /* de "G " niet in lijstje */
                           Nv_z[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Nv_z, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Nv_z, LFR_ONTBREKEND);
                  } /* if (line_teller == 10) */

                  /*
                  //////////////// Nu_z (aantal vanwegwe 0-sigma-fout afgekeurde golfhoogtepunten)
                  */
                  if (line_teller == 11)                             /* Nu_z */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )             /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Nu_z, line + pos);                   /* de "G " niet in lijstje */
                           Nu_z[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                          strcpy(Nu_z, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Nu_z, LFR_ONTBREKEND);
                  } /* if (line_teller == 11) */


                  /*
                  //////////////// Ni_z (aantal geinterpoleerde of geextrapoleerde golfhoogtepunten)
                  */
                  if (line_teller == 12)                             /* Ni_z */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Ni_z, line + pos);                   /* de "G " niet in lijstje */
                           Ni_z[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Ni_z, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Ni_z, LFR_ONTBREKEND);
                  } /* if (line_teller == 7) */


                  /*
                  //////////// H1d3
                  */
                  if (line_teller == 14)                            /* H 1/3 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor LFR:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(H1d3, char_hulp_waarde);
                        }
                        else
                           strcpy(H1d3, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(H1d3, LFR_ONTBREKEND);
                  } /* if (line_teller == 14) */


                  /*
                  //////////// H1d10
                  */
                  if (line_teller == 15)                            /* H 1/10 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )             /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor LFR:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(H1d10, char_hulp_waarde);
                        }
                        else
                           strcpy(H1d10, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(H1d10, LFR_ONTBREKEND);
                  } /* if (line_teller == 15) */


                  /*
                  //////////// H1d50
                  */
                  if (line_teller == 16)                            /* H 1/50 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor LFR:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(H1d50, char_hulp_waarde);
                        }
                        else
                           strcpy(H1d50, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(H1d50, LFR_ONTBREKEND);
                  } /* if (line_teller == 16) */


                  /*
                  //////////// GGh (gemiddelde golfhoogte
                  */
                  if (line_teller == 17)                            /* GGh */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor LFR:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(GGh, char_hulp_waarde);
                        }
                        else
                           strcpy(GGh, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(GGh, LFR_ONTBREKEND);
                  } /* if (line_teller == 17) */


                  /*
                  //////////// GGT (T gem)
                  */
                  if (line_teller == 18)
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(GGT, line + pos);                   /* de "G " niet in lijstje */
                           GGT[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(GGT, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(GGT, LFR_ONTBREKEND);
                  } /* if (line_teller == 18) */



                  /*
                  //////////// Hmax
                  */
                  if (line_teller == 19)                            /* Hmax */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor LFR:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(Hmax, char_hulp_waarde);
                        }
                        else
                           strcpy(Hmax, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Hmax, LFR_ONTBREKEND);
                  } /* if (line_teller == 19) */


                  /*
                  //////////// Tmax
                  */
                  if (line_teller == 20)
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Tmax, line + pos);                   /* de "G " niet in lijstje */
                           Tmax[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Tmax, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Tmax, LFR_ONTBREKEND);
                  } /* if (line_teller == 20) */


                  /*
                  //////////// THmax
                  */
                  if (line_teller == 21)
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(THmax, line + pos);                   /* de "G " niet in lijstje */
                           THmax[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(THmax, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(THmax, LFR_ONTBREKEND);
                  } /* if (line_teller == 20) */

                  /*
                  //////////// T1d3 (T 1/3)
                  */
                  if (line_teller == 24)
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(T1d3, line + pos);                   /* de "G " niet in lijstje */
                           T1d3[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(T1d3, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(T1d3, LFR_ONTBREKEND);
                  } /* if (line_teller == 24) */


                  /*
                  //////////// TH1d3 (T 1/3)
                  */
                  if (line_teller == 25)
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(TH1d3, line + pos);                   /* de "G " niet in lijstje */
                           TH1d3[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(TH1d3, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(TH1d3, LFR_ONTBREKEND);
                  } /* if (line_teller == 25) */



                  /*
                  //////////// aantal golven (AG)
                  */
                  if (line_teller == 26)
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(aantal_golven, line + pos);          /* de "G " niet in lijstje */
                           aantal_golven[hulp_lengte -1 -pos] = '\0';  /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(aantal_golven, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(aantal_golven, LFR_ONTBREKEND);
                  } /* if (line_teller == 26) */


                  /*
                  //////////////// HE0 (afgeleid van TE0)
                  */
                  if (line_teller == 29)                            /* TE1 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(TE0, line + pos);                 /* de "G " niet in lijstje */
                           TE0[hulp_lengte -1 -pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lfr [HE0 = wortel(TE0) * 4]                                                         */
                           /*                  LET op: in de sovf80 file staat: TE0 in cm**                       */
                           /*                          in de LFR file staat HE0 in mm [HE0 = wortel(TE0) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE0) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE0, "%d", num_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                           strcpy(HE0, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(HE0, LFR_ONTBREKEND);
                  } /* if (line_teller == 29) */


                  /*
                  //////////////// TE1 (HFE) hoog frequentie en HE1 (afgele1d van TE1)
                  */
                  if (line_teller == 30)                            /* TE1 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* cic */
                           strcpy(TE1, line + pos);                   /* de "G " niet in lijstje */
                           TE1[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lfr [HE1 = wortel(TE1) * 4]                                                         */
                           /*                  LET op: in de sovf80 file staat: TE1 in cm**                       */
                           /*                          in de LFR file staat HE1 in mm [HE1 = wortel(TE1) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE1) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE1, "%d", num_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                        {
                           strcpy(TE1, CIC_ONTBREKEND);
                           strcpy(HE1, LFR_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(TE1, CIC_ONTBREKEND);
                        strcpy(HE1, LFR_ONTBREKEND);
                     }
                  } /* if (line_teller == 30) */


                  /*
                  //////////// TE2 (MFE) midden frequentie en HE2 (afgeleid van TE2)
                  */
                  if (line_teller == 31)                            /* TE2 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* cic */
                           strcpy(TE2, line + pos);                 /* de "G " niet in lijstje */
                           TE2[hulp_lengte -1 -pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lfr [HE2 = wortel(TE2) * 4]                                                         */
                           /*                  LET op: in de sovf80 file staat: TE2 in cm**                       */
                           /*                          in de LFR file staat HE2 in mm [HE2 = wortel(TE2) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE2) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE2, "%d", num_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                        {
                           strcpy(TE2, CIC_ONTBREKEND);
                           strcpy(HE2, LFR_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(TE2, CIC_ONTBREKEND);
                        strcpy(HE2, LFR_ONTBREKEND);
                     }
                  } /* if (line_teller == 31) */


                  /*
                  //////////// TE3 (LFE)  laag frequentie  en HE3 (afgeleid van TE3)
                  */
                  if (line_teller == 32)                            /* TE3 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* cic */
                           strcpy(TE3, line + pos);                 /* de "G " niet in lijstje */
                           TE3[hulp_lengte -1 -pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lfr [HE3 = wortel(TE3) * 4]                                                         */
                           /*                  LET op: in de sovf80 file staat: TE3 in cm**                       */
                           /*                          in de LFR file staat HE3 in mm [HE3 = wortel(TE3) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE3) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE3, "%d", num_hulp_waarde);

                        }  /* if (strncmp(line, "G ", 2) == 0) */
                        else
                        {
                           strcpy(TE3, CIC_ONTBREKEND);
                           strcpy(HE3, LFR_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(TE3, CIC_ONTBREKEND);
                        strcpy(HE3, LFR_ONTBREKEND);
                     }
                  } /* if (line_teller == 32) */



                  /*
                  //////////// HM0
                  */
                  if (line_teller == 35)                            /* HM0 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor CIC */
                           strcpy(HM0_cic, line + pos);                 /* de "G " niet in lijstje */
                           HM0_cic[hulp_lengte -1 -pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* voor LFR:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(HM0_lfr, char_hulp_waarde);
                        }
                        else
                        {
                           strcpy(HM0_cic, CIC_ONTBREKEND);
                           strcpy(HM0_lfr, LFR_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(HM0_cic, CIC_ONTBREKEND);
                        strcpy(HM0_lfr, LFR_ONTBREKEND);
                     }
                  } /* if (line_teller == 35) */



                  /*
                  //////////// Tm02 (Tm02)
                  */
                  if (line_teller == 36)                               /* Tm02 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Tm02, line + pos);               /* de "G " niet in lijstje */
                           Tm02[hulp_lengte -1 -pos] = '\0';       /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Tm02, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Tm02, LFR_ONTBREKEND);
                  } /* if (line_teller == 36) */



                  /*
                  //////////// Tm_10 (TM0-1, Ts)
                  */
                  if (line_teller == 37)                            /* Tm_10 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* cic */
                           strcpy(Tm_10_cic, line + pos);               /* de "G " niet in lijstje */
                           Tm_10_cic[hulp_lengte -1 -pos] = '\0';       /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lfr */
                           strcpy(Tm_10_lfr, line + pos);               /* de "G " niet in lijstje */
                           Tm_10_lfr[hulp_lengte -1 -pos] = '\0';       /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                        {
                           strcpy(Tm_10_cic, CIC_ONTBREKEND);
                           strcpy(Tm_10_lfr, LFR_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(Tm_10_cic, CIC_ONTBREKEND);
                        strcpy(Tm_10_lfr, LFR_ONTBREKEND);
                     }
                  } /* if (line_teller == 37) */


                  /*
                  //////////// Fp (piekfrequentie)
                  */
                  if (line_teller == 38)                            /* fp */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Fp, line + pos);                  /* de "G " niet in lijstje */
                           Fp[hulp_lengte -1 -pos] = '\0';          /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Fp, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Fp, LFR_ONTBREKEND);
                  } /* if (line_teller == 38) */



                  /*
                  //////////// Hs7
                  */
                  if (line_teller == 39)                            /* Hs7 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* cic */
                           strcpy(Hs7_cic, line + pos);                 /* de "G " niet in lijstje */
                           Hs7_cic[hulp_lengte -1 -pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* voor lfr:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(Hs7_lfr, char_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                        {
                           strcpy(Hs7_cic, CIC_ONTBREKEND);
                           strcpy(Hs7_lfr, LFR_ONTBREKEND);
                        }
                      } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(Hs7_cic, CIC_ONTBREKEND);
                        strcpy(Hs7_lfr, LFR_ONTBREKEND);
                     }
                  } /* if (line_teller == 39) */


                  /*
                  //////////// AV10_H (vrijheidsgraden spectrum componenten)
                  */
                  if (line_teller == 40)                                /* fp */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)             /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(AV10_H, line + pos);                  /* de "G " niet in lijstje */
                           AV10_H[hulp_lengte -1 -pos] = '\0';          /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(AV10_H, LFR_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(AV10_H, LFR_ONTBREKEND);
                  } /* if (line_teller == 40) */


                  /*
                  //////////// dd en ff
                  //           omdat de regels met frequenties wat kan verschillen kan je na het freq blok (va regel 42)
                  //           niet meer op de regel nummering afgaan daarom testen op label "wind"
                  //           (NB als ff niet aanwezig dan staat er een N als dd niet aanwezig een N=, dus geen 99999)
                  */

                  /* eerst line met aanduiding "wind" localiseren  (daarna altijd eerst de ff line en dan de dd line) */
                  if ((hulp_lengte = strlen(line)) == 5)               /* geeft lengte inclusief '\ n' */
                  {
                     if (strncmp(line, "wind", 4) == 0)
                     {
                        /* ff */
                        if (fgets(line, 255, in) != NULL)
                        {
                           /* ff (input is niet gecorrigeerd voor hoogte; moet dus  gecorrigeerd worden) */
                           if ((hulp_lengte = strlen(line)) >= 3)         /* bv "G3'\n'" (dus inclusief '\n') */
                           {
                              Bepaal_ff_Reductie_Factor(sensorcode, &ff_factor);
                              if ( (strncmp(line, "G", 1) == 0)  && (strncmp(line, "G99999", 6) != 0) && (ff_factor != 99999) )   /* regel moet beginnen met "G" */
                              {
                                 pos = 1;
                                 strcpy(hulp_ff, line + pos);                    /* de "G" niet in lijstje */
                                 hulp_ff[hulp_lengte -1 -pos] = '\0';            /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                                 hulp_num_ff = (int)(atof(hulp_ff) / ff_factor + 0.5);
                                 sprintf(ff_cic, "%d", hulp_num_ff);
                                 sprintf(ff_lfr, "%d", hulp_num_ff);

                              } /* if ( (strncmp(line, "G", 1) == 0) && (ff_factor != 99999) ) */
                              else
                              {
                                 strcpy(ff_cic, CIC_ONTBREKEND);
                                 strcpy(ff_lfr, FF_LFR_ONTBREKEND);
                              }
                           } /* if ((hulp_lengte = strlen(line)) >= 3) */
                           else
                           {
                              strcpy(ff_cic, CIC_ONTBREKEND);
                              strcpy(ff_lfr, FF_LFR_ONTBREKEND);
                           }
                        } /* if (fgets(line, 255, in) != NULL) */

                        /* dd */
                        if (fgets(line, 255, in) != NULL)
                        {
                           if ((hulp_lengte = strlen(line)) == 6)         /* bv "G143='\n'" (dus inclusief '\n') */
                           {
                              /* hulp_ddd in graden, moet dd in tientallen graden worden */
                              if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G99999", 6) != 0) )              /* regel moet beginnen met "G" */
                              {
                                 pos = 1;
                                 strcpy(hulp_ddd, line + pos);                    /* de "G" niet in lijstje */
                                 hulp_ddd[hulp_lengte -1 -pos] = '\0';            /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                                 hulp_num_dd = (int)(atof(hulp_ddd) / 10 + 0.5);
                                 sprintf(dd_cic, "%2d", hulp_num_dd);
                                 sprintf(dd_lfr, "%2d", hulp_num_dd);

                              } /* if (strncmp(line, "G", 1) == 0) */
                              else
                              {
                                 strcpy(dd_cic, CIC_ONTBREKEND);
                                 strcpy(dd_lfr, DD_LFR_ONTBREKEND);
                              }
                           } /* if ((hulp_lengte = strlen(line)) == 6) */
                           else
                           {
                              strcpy(dd_cic, CIC_ONTBREKEND);
                              strcpy(dd_lfr, DD_LFR_ONTBREKEND);
                           }
                        } /* if (fgets(line, 255, in) != NULL) */

                        break;                                     /* uit while lus springen */

                     } /* if (strncmp(line, wind_label, 4) == 0) */
                  } /* if ((hulp_lengte = strlen(line)) == 5) */



                  /*
                  //////////// Czz5 en Czz10
                  //           omdat de regels met frequenties wat kan verschillen testen op GHC label
                  */
                  /* eerst line met aanduiding "GHC" localiseren  */
                  if ((hulp_lengte = strlen(line)) == 4)                        /* geeft lengte inclusief '\ n' */
                  {
                     if (strncmp(line, "GHC", 3) == 0)
                     {
                        /* initialisatie  */
                        czz5_array_teller = 0;
                        czz5_line_teller  = 0;

                        while (czz5_array_teller < AANTAL_CZZ5)                  /* zijn altijd 25 waarden in czz5 blok */
                        {
                           if (fgets(line, 255, in) != NULL)                     /* regel inlezen */
                           {
                              czz5_line_teller++;
                              if ((hulp_lengte = strlen(line)) >= 2)
                              {
                                 if (czz5_line_teller == 1)                      /* eerst regel van czz5 staat een G als het goed is */
                                 {
                                    if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel OK */
                                    {
                                       /* ingelezen line scannen */
                                       pos_old = 2;
                                       for (pos = 3; pos < hulp_lengte; pos++)   /* "G " overslaan */
                                       {
                                          if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )  /* voorbij eerste " " */
                                          {
                                             /* moet .cm2s worden (staat in cm2s) */
                                             char_hulp_waarde[0] = '\0';
                                             strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                             char_hulp_waarde[pos - pos_old] = '\0';
                                             num_hulp_waarde = atoi(char_hulp_waarde) * 10;

                                             if (num_hulp_waarde != 999990)      /* als er 99999 stond een ongeldige waarde */
                                             {
                                                char_hulp_waarde[0] = '\0';
                                                sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                                strcpy(czz5_array[czz5_array_teller], char_hulp_waarde);
                                             }

                                             pos_old = pos +1;                   /* +1 om net verder dan de " " komen */
                                             czz5_array_teller++;
                                          } /* if (strncmp(line + pos, " ", 1) == 0) */
                                       } /* for (i = 0; i < hulp_lengte; i++) */
                                    } /* if (strncmp(line, "G", 1) == 0) */
                                    else
                                    {
                                       Czz5_ok = 0;                                 /* false */
                                       break;                                       /* dus bij een "N" er uit springen (wordt niets in aaray geschreven) */
                                    }
                                 } /* if (czz5_line_teller == 1) */
                                 else /* dus niet eerste regel van czz5 blok */
                                 {
                                    /* ingelezen line scannen (i.t.t. de eerste line beginnen volgende niet met "G") */
                                    pos_old = 0;
                                    for (pos = 1; pos < hulp_lengte; pos++)
                                    {
                                       if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )
                                       {
                                          /* moet .cm2s worden (staat in cm2s) */
                                          char_hulp_waarde[0] = '\0';
                                          strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                          char_hulp_waarde[pos - pos_old] = '\0';
                                          num_hulp_waarde = atoi(char_hulp_waarde) * 10;

                                          if (num_hulp_waarde != 999990)      /* als er 99999 stond een ongeldige waarde */
                                          {
                                             char_hulp_waarde[0] = '\0';
                                             sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                             strcpy(czz5_array[czz5_array_teller], char_hulp_waarde);
                                          }

                                          pos_old = pos +1;                   /* +1 om net verder dan de " " komen */
                                          czz5_array_teller++;
                                       } /* if (strncmp(line + pos, " ", 1) == 0) */
                                    } /* for (i = 0; i < hulp_lengte; i++) */
                                 } /* else (niet eerst eregel van czz5 blok) */

                              } /* if ((hulp_lengte = strlen(line)) >= 2) */
                              else
                              {
                                 Czz5_ok = 0;                                 /* false */
                                 break;
                              }  /* else */
                           } /* if (fgets(line, 255, in) != NULL) */
                           else
                           {
                              Czz5_ok = 0;                                    /* false */
                              break;
                           }  /* else */
                        } /* while (czz5_array_teller < AANTAL_CZZ5) */


                        /* initialisatie  */
                        czz10_array_teller = 0;
                        czz10_line_teller = 0;

                        while (czz10_array_teller < AANTAL_CZZ10)                /* zijn altijd 51 waarden in czz10 blok */
                        {
                           if (fgets(line, 255, in) != NULL)                     /* regel inlezen */
                           {
                              /* ////// TEST ///////////
                              fprintf(stderr, "%s", line);
                              getchar();
                              ///// TEST ///////////*/

                              czz10_line_teller++;
                              if ((hulp_lengte = strlen(line)) >= 2)
                              {
                                 if (czz10_line_teller == 1)                     /* eerst regel van czz5 staat een G als het goed is */
                                 {
                                    if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G 99999", 7) != 0) )               /* regel OK */
                                    {
                                       /* ingelezen line scannen */
                                       pos_old = 2;
                                       for (pos = 3; pos < hulp_lengte; pos++)   /* "G " overslaan */
                                       {
                                          if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )  /* voorbij eerste " " */
                                          {
                                             /* moet .cm2s worden (staat in cm2s) */
                                             char_hulp_waarde[0] = '\0';
                                             strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                             char_hulp_waarde[pos - pos_old] = '\0';
                                             num_hulp_waarde = atoi(char_hulp_waarde) * 10;

                                             if (num_hulp_waarde != 999990)      /* als er 99999 stond een ongeldige waarde */
                                             {
                                                char_hulp_waarde[0] = '\0';
                                                sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                                strcpy(czz10_array[czz10_array_teller], char_hulp_waarde);
                                             }

                                             pos_old = pos + 1;                  /* +1 om net verder dan de " " komen */
                                             czz10_array_teller++;
                                          } /* if (strncmp(line + pos, " ", 1) == 0) */
                                       } /* for (i = 0; i < hulp_lengte; i++) */
                                    } /* if (strncmp(line, "G", 1) == 0) */
                                    else
                                    {
                                       Czz10_ok = 0;                                /* false */
                                       break;                                       /* dus bij een "N" er uit springen (wordt niets in aaray geschreven) */
                                    }    
                                 } /* if (czz10_line_teller == 1) */

                                 else /* dus niet eerste regel van czz10 blok */
                                 {
                                    /* ingelezen line scannen (i.t.t. de eerste line beginnen volgende niet met "G") */
                                    pos_old = 0;
                                    for (pos = 1; pos < hulp_lengte; pos++)
                                    {
                                       if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )
                                       {
                                          /* moet .cm2s worden (staat in cm2s) */
                                          char_hulp_waarde[0] = '\0';
                                          strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                          char_hulp_waarde[pos - pos_old] = '\0';
                                          num_hulp_waarde = atoi(char_hulp_waarde) * 10;

                                          if (num_hulp_waarde != 999990)      /* als er 99999 stond een ongeldige waarde */
                                          {
                                             char_hulp_waarde[0] = '\0';
                                             sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                             strcpy(czz10_array[czz10_array_teller], char_hulp_waarde);
                                          }

                                          pos_old = pos +1;                   /* +1 om net verder dan de " " komen */
                                          czz10_array_teller++;
                                       } /* if (strncmp(line + pos, " ", 1) == 0) */
                                    } /* for (i = 0; i < hulp_lengte; i++) */
                                 } /* else (niet eerst eregel van czz10 blok) */

                              } /* if ((hulp_lengte = strlen(line)) >= 2) */
                              else
                              {
                                 Czz10_ok = 0;                               /* false */
                                 break;
                              }  /* else */
                           } /* if (fgets(line, 255, in) != NULL) */
                           else
                           {
                              Czz10_ok = 0;                                  /* false */
                              break;
                           }  /* else */
                        } /* while (czz10_array_teller < AANTAL_CZZ10) */

                     } /* if (strncmp(line, "GHC", 3) == 0) */
                  } /* if ((hulp_lengte = strlen(line)) == 4) */

               } /* while (fgets(line, 255, in) != NULL) */





               /*
               //
               /////////////////////////   afgeleide waarden buiten while lus houden) //////////////////////
               //
               */


               /*
               /////////// dd, ff ALTERNATIEF (van een andere locatie dan uit de sovf input file)
               */
               Check_Alternatieve_dd_ff(sensorcode, dd_alternatief, ff_alternatief);
               if (strncmp(dd_alternatief, "xx", 2) != 0)                     /* dus er is een alternatief */
               {
                  /* alternatief gebruiken (dus de orginele eerder opgehaalde overschrijven) */
                  strcpy(dd_cic,       "\0");                                 /* 10 deg */
                  strcpy(ff_cic,       "\0");                                 /* knots */
                  strcpy(dd_lfr,       "\0");                                 /* 10 deg */
                  strcpy(ff_lfr,       "\0");                                 /* knots */

                  strcpy(dd_cic, dd_alternatief);
                  strcpy(dd_lfr, dd_alternatief);

                  strcpy(ff_cic, ff_alternatief);
                  strcpy(ff_lfr, ff_alternatief);

               } /* if (strncmp(alternatieve_dd, "xx", 2) != 0) */



               /*
               //////////////// aantal fouten (Nd_z + Nv_z + Nu_z) [AFGELEIDE WAARDE]
               // (nb kan niet testen op LFR_ONTBREKEND (=0) i.v.m. 0 fouten)
               */
               if ( (strcmp(Nd_z, "99999") != 0) && (strcmp(Nv_z, "99999") != 0) && (strcmp(Nu_z, "99999") != 0) )
               {
                  num_aantal_fouten = atoi(Nd_z) + atoi(Nv_z) + atoi(Nu_z);
                  sprintf(aantal_fouten, "%d", num_aantal_fouten);
               }
               else
                  strcpy(aantal_fouten, LFR_ONTBREKEND);


               /*
               //////////////// aantal meetwaarden  [AFGELEIDE WAARDE]
               // (nb kan niet testen op LFR_ONTBREKEND (0) i.v.m. 0 meetwaarden)
               */
               if ( (strcmp(Ndir_H, "99999") != 0) && (strcmp(Ngd_zP, "99999") != 0) )
               {
                  num_aantal_meetwaarden_methode_1 = atoi(Ndir_H) * 256;          /* 256 lengte deelreeks (Ndir_H is maximaal: 6) */
                  num_aantal_meetwaarden_methode_2 = (int)(atof(Ngd_zP) * 6 * 256 / 1000 + 0.5); /* 1000 i.v.m. promillage */

                  /* in overleg met Evert op 23-05-2005 afgesproken de laagste waarde te nemen */
                  if (num_aantal_meetwaarden_methode_1 < num_aantal_meetwaarden_methode_2)
                     num_aantal_meetwaarden = num_aantal_meetwaarden_methode_1;
                  else
                     num_aantal_meetwaarden = num_aantal_meetwaarden_methode_2;

                  sprintf(aantal_meetwaarden, "%d", num_aantal_meetwaarden);
               }
               else
                  strcpy(aantal_meetwaarden, LFR_ONTBREKEND);


               /*
               //////////// Czz5 en Czz10 interpolaties [AFGELEIDE WAARDEN]
               */

               /* (Annex C punt 8) */
               if ( (strcmp(czz10_array[0], "-1") != 0)           && (strcmp(czz10_array[1], "-1") != 0) &&
                    ((strcmp(czz10_array[0], LFR_ONTBREKEND) != 0) || (strcmp(czz10_array[1], LFR_ONTBREKEND) != 0)) )
               {
                  num_ghc_interpol_7_9 = (atoi(czz10_array[0]) + atoi(czz10_array[1])) / 2;
                  sprintf(ghc_interpol_7_9, "%d", num_ghc_interpol_7_9);
               } /* if ( (strcmp(czz10_array[0], "-1" != 0) && (strcmp(czz10_array[1], "-1" != 0) ) */

               /* (Annex C punt 10) */
               if ( (strcmp(czz10_array[1], "-1") != 0)           && (strcmp(czz10_array[2], "-1") != 0) &&
                    ((strcmp(czz10_array[1], LFR_ONTBREKEND) != 0) || (strcmp(czz10_array[2], LFR_ONTBREKEND) != 0)) )
               {
                  num_ghc_interpol_9_11 = (atoi(czz10_array[1]) + atoi(czz10_array[2])) / 2;
                  sprintf(ghc_interpol_9_11, "%d", num_ghc_interpol_9_11);
               } /* if ( (strcmp(czz10_array[1], "-1" != 0) && (strcmp(czz10_array[2], "-1" != 0) ) */

               /* (Annex C punt 12) */
               if ( (strcmp(czz10_array[2], "-1") != 0)           && (strcmp(czz5_array[0], "-1") != 0) &&
                    ((strcmp(czz10_array[2], LFR_ONTBREKEND) != 0) || (strcmp(czz5_array[0], LFR_ONTBREKEND) != 0)) )
               {
                  num_ghc_interpol_11_13 = (atoi(czz10_array[2]) + atoi(czz5_array[0])) / 2;
                  sprintf(ghc_interpol_11_13, "%d", num_ghc_interpol_11_13);
               } /* if ( (strcmp(czz10_array[2], "-1" != 0) && (strcmp(czz5_array[0], "-1" != 0) ) */

               /* sluiten input file */
               fclose(in);


               /*
               //////////// SUSPECT FLAG [AFGELEIDE WAARDEN]
               */
               strcpy(flag, OK_FLAG);                                    /* default */
               Bepaal_Suspect_Station(sensorcode, flag);                 /* kan hier SUSPECT worden */



            } /* if ((in = fopen(dir_en_input_filenaam, "r")) != NULL) */






            /*********************************************************************/
            /*                                                                   */
            /*           schrijven naar CIC output file (was al geopend)         */
            /*                                                                   */
            /*********************************************************************/

            /* alleen als er een HM0 instaat (indien geen HM0 dan nooit iets bijzonders verder per definitie) */
            /*        7654321: (ONTBREKEND) wordt door dit programma er ingezet als er geen waarde voor HM0 staat */
            /*          99999: staat soms in de input files als er (tijdelijk ?) geen data voor HM0 is */
            if ( (atoi(aantal_meetwaarden) > 0) && (strcmp(HM0_cic, CIC_ONTBREKEND) != 0) && (strcmp(HM0_cic, "99999") != 0) )
            {
               /* checks (via Evert) HM0    > 0   (dan alle waarden uit de waarnemingen ongeldig) */
               /*                     Tm_10 < 999 (dan alle waarden uit de waarnemingen ongeldig) */
               /* NB 12-07-2005 afgesproken om niet meer te testen op Tm_10 */
               if ( (atoi(HM0_cic) > 0) /*&& (atoi(Tm_10_cic) < 999)*/ )
               {
                  /* spatie (overblijfsel uit Fortran tijdperk) */
                  fprintf(out_cic, "%1s", " ");

                  /* datum (zonder eeuw aanduiding) */
                  strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
                  pos = 2;
                  strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
                  jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
                  fprintf(out_cic, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* sensorcode met "NZ" ervoor */
                  fprintf(out_cic, "%3.3s", "NZ");
                  fprintf(out_cic, "%4.4s", sensorcode);

                  /* Latitude, Longitude and Quadrant */
                  Bepaal_Lat_Lon_Q(sensorcode, latitude, longitude, quadrant);
                  fprintf(out_cic, "%2.2s", quadrant);
                  fprintf(out_cic, "%8.8s", longitude);
                  fprintf(out_cic, "%8.8s", latitude);

                  /* dd */
                  fprintf(out_cic, "%8.8s", dd_cic);

                  /* ff */
                  fprintf(out_cic, "%8.8s", ff_cic);

                  /* Ts */
                  fprintf(out_cic, "%8.8s", Tm_10_cic);

                  /* HM0 (cic) */
                  fprintf(out_cic, "%8.8s", HM0_cic);

                  /* TE3 */
                  fprintf(out_cic, "%8.8s", TE3);

                  /* TE2 */
                  fprintf(out_cic, "%8.8s", TE2);

                  /* TE1 */
                  fprintf(out_cic, "%8.8s", TE1);

                  /* Hs7 */
                  fprintf(out_cic, "%8.8s", Hs7_cic);

                  /* flag */
                  /*strcpy(flag, OK_FLAG);                    */                /* default */
                  /*Bepaal_Suspect_Station(sensorcode, flag); */                /* kan hier SUSPECT worden */
                  fprintf(out_cic, "%8.8s", flag);

                  /* nieuwe regel */
                  fprintf(out_cic, "\n");

               } /* if ( (HM0 > 0) && (Ts < 999) ) */
            } /* if ( (aantal_meetwaarden > 0) && (strcmp(HM0, ONTBREKEND) != 0) || (strcmp(HM0, "99999") != 0) ) */


            /* ZOU OOK KUNNEN :
            //fwrite(jaar_dag_uur_JJJJMMDDUU, strlen(jaar_dag_uur_JJJJMMDDUU), 1, out);
            //fwrite(" NZ", strlen(" NZ"), 1, out);
            //fwrite(sensorcode, strlen(sensorcode), 1, out);
            //fwrite(" TEST", strlen(" TEST"), 1, out);
            //  putc('\n', out);
            */



            /*********************************************************************/
            /*                                                                   */
            /*           schrijven naar LFR output file (was al geopend)         */
            /*                                                                   */
            /*********************************************************************/

            /* NB in de lfr files wordt ontbrekend een -1 (dit is dus anders dan in de cic file) */

            /* alleen als er een HM0 instaat (indien geen HM0 dan nooit iets bijzonders verder per definitie) */
            /*        7654321: (ONTBREKEND) wordt door dit programma er ingezet als er geen waarde voor HM0 staat */
            /*          99999: staat soms in de input files als er (tijdelijk ?) geen data voor HM0 is */
            if ( (strncmp(flag, OK_FLAG, 7) == 0) && (atoi(aantal_meetwaarden) > 0) && (strcmp(HM0_lfr, LFR_ONTBREKEND) != 0) && (strcmp(HM0_lfr, "99999") != 0) )
            {
               /* checks (via Evert) HM0    > 0   (dan alle waarden uit de waarnemingen ongeldig) */
               /*                     Tm_10 < 999 (dan alle waarden uit de waarnemingen ongeldig) */
               /* NB 12-07-2005: afgesproken om niet meer te testen op Tm_10 */
               /* if ( (atoi(HM0_lfr) > 0) && (czz5_array[0] != LFR_ONTBREKEND) && (czz10_array[0] != LFR_ONTBREKEND) && (atoi(Tm_10_lfr) < 999) ) */
               /* if ( (atoi(HM0_lfr) > 0) && (czz5_array[0] != LFR_ONTBREKEND) && (czz10_array[0] != LFR_ONTBREKEND) ) */
               if ( (atoi(HM0_lfr) > 0) && (Czz5_ok == 1) && (Czz10_ok == 1) )
               {
                  /* sensorcode (zonder "NZ" ervoor) */
                  fprintf(out_lfr, "%4.4s", sensorcode);

                  /* spatie */
                  fprintf(out_lfr, "%1.1s", " ");

                  /* datum (zonder eeuw aanduiding) */
                  strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
                  pos = 2;
                  strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
                  jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
                  fprintf(out_lfr, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* dd */
                  fprintf(out_lfr, "%3.3s", dd_lfr);

                  /* ff */
                  fprintf(out_lfr, "%2.2s", ff_lfr);

                  /* kwaliteitscode (overblijfsel van vroeger (1970) is altijd 1) */
                  fprintf(out_lfr, "%2.2s", "1");

                  /* -1 (annex Ax punt 1) */
                  fprintf(out_lfr, "%4.4s", "-1");

                  /* -1 (annex Ax punt 2) */
                  fprintf(out_lfr, "%4.4s", "-1");

                  /* aantal fouten (afgeleide waarde kan nooit 99999 zijn) (annex Ax punt 3) */
                  fprintf(out_lfr, "%4.4s", aantal_fouten);

                  /* -1 (annex Ax punt 4) */
                  fprintf(out_lfr, "%4.4s", "-1");

                  /* aantal meetwaarden (afgeleide waarde kan nooit 99999 zijn ) (annex Ax punt 5) */
                  fprintf(out_lfr, "%4.4s", aantal_meetwaarden);

                  /* -1 (annex Ax punt 6) */
                  fprintf(out_lfr, "%4.4s", "-1");

                  /* spectrum 5 mHz (0.0 Hz) (annex Ax punt 7) */
                  fprintf(out_lfr, "%9.9s", czz10_array[0]);

                  /* spectrum interpolatie van GHC el. 7 en 9 (annex Ax punt 8) */
                  fprintf(out_lfr, "%9.9s", ghc_interpol_7_9);

                  /* spectrum 5 mHz (0.01 Hz) (annex Ax punt 9) */
                  fprintf(out_lfr, "%9.9s", czz10_array[1]);

                  /* spectrum interpolatie van GHC el. 9 en 11 (annex Ax punt 10) */
                  fprintf(out_lfr, "%9.9s", ghc_interpol_9_11);

                  /* spectrum 5 mHz (0.02 Hz) (annex Ax punt 11) */
                  fprintf(out_lfr, "%9.9s", czz10_array[2]);

                  /* spectrum interpolatie van GHC el. 11 en 13 (annex Ax punt 12) */
                  fprintf(out_lfr, "%9.9s", ghc_interpol_11_13);

                  /* spectrum 5 Mhz (0.03 Hz - 0.15 Hz) (Annex Ax punt 13 - 37) */
                  for (m = 0; m < AANTAL_CZZ5; m++)
                     fprintf(out_lfr, "%9.9s", czz5_array[m]);

                  /* vrijh.graden sp. komp. (Annex Ax punt 38) */
                  fprintf(out_lfr, "%3.3s", AV10_H);

                  /* spectrum 10 Mhz (0.15 Hz - 0.5 Hz) (Annex Ax punt 39 - 74) */
                  for (m = 15; m < AANTAL_CZZ10; m++)
                     fprintf(out_lfr, "%7.7s", czz10_array[m]);

                  /* HM0 (Annex Ax punt 75) */
                  fprintf(out_lfr, "%5.5s", HM0_lfr);

                  /* HE0 (Annex Ax punt 76) */
                  fprintf(out_lfr, "%5.5s", HE0);

                  /* HE1 (Annex Ax punt 77) */
                  fprintf(out_lfr, "%5.5s", HE1);

                  /* HE2 (Annex Ax punt 78) */
                  fprintf(out_lfr, "%5.5s", HE2);

                  /* HE3 (Annex Ax punt 79) */
                  fprintf(out_lfr, "%5.5s", HE3);

                  /* vrijheids graden HM0 (standaard op 0, staat niet in sovf80) (Annex Ax punt 80) */
                  fprintf(out_lfr, "%4.4s", "0");

                  /* -1 (annex Ax punt 81) */
                  fprintf(out_lfr, "%4.4s", "-1");

                  /* Tm-10 (Tm0-1) (annex Ax punt 82) */
                  fprintf(out_lfr, "%4.4s", Tm_10_lfr);

                  /* fp (piekfrequentie) (annex Ax punt 83) */
                  fprintf(out_lfr, "%4.4s", Fp);

                  /* Hs7 (mm) (annex Ax punt 84) */
                  fprintf(out_lfr, "%5.5s", Hs7_lfr);

                  /* Tm02 (annex Ax punt 85) */
                  fprintf(out_lfr, "%3.3s", Tm02);

                  /* -1 (annex Ax punt 86) */
                  fprintf(out_lfr, "%3.3s", "-1");

                  /* aantal golven (AG) (annex Ax punt 87) */
                  fprintf(out_lfr, "%3.3s", aantal_golven);

                  /* spectrum 10 Mhz (0.0 Hz - 0.14 Hz) (Annex Ax punt 88 - 102) */
                  for (m = 0; m < 15; m++)
                     fprintf(out_lfr, "%9.9s", czz10_array[m]);

                  /* -1 (dummy) (Annex Ax punt 103 - 113) */
                  for (m = 0; m < 11; m++)
                     fprintf(out_lfr, "%3.3s", "-1");

                  /* H1d3 (H 1/3) (annex Ax punt 114) */
                  fprintf(out_lfr, "%5.5s", H1d3);

                  /* H1d10 (H 1/10) (annex Ax punt 115) */
                  fprintf(out_lfr, "%5.5s", H1d10);

                  /* H1d50 (H 1/50) (annex Ax punt 116) */
                  fprintf(out_lfr, "%5.5s", H1d50);

                  /* GGh (H gem) (annex Ax punt 117) */
                  fprintf(out_lfr, "%5.5s", GGh);

                  /* Hmax (H max) (annex Ax punt 118) */
                  fprintf(out_lfr, "%5.5s", Hmax);

                  /* T1d3 (T 1/3) (annex Ax punt 119) */
                  fprintf(out_lfr, "%3.3s", T1d3);

                  /* TH1d3 (T H1/3) (annex Ax punt 120) */
                  fprintf(out_lfr, "%3.3s", TH1d3);

                  /* GGT (T gem) (annex Ax punt 121) */
                  fprintf(out_lfr, "%3.3s", GGT);

                  /* Tmax (T max) (annex Ax punt 122) */
                  fprintf(out_lfr, "%3.3s", Tmax);

                  /* THmax (T Hmax) (annex Ax punt 123) */
                  fprintf(out_lfr, "%3.3s", THmax);

                  /* statuscode (altijd "0" overblijfsel van vroeger (annex Ax punt 124) */
                  fprintf(out_lfr, "%1.1s", "0");

                  /* statuscode (altijd "0" overblijfsel van vroeger (annex Ax punt 125) */
                  fprintf(out_lfr, "%1.1s", "0");

                  /* statuscode (altijd "0" overblijfsel van vroeger (annex Ax punt 126) */
                  fprintf(out_lfr, "%1.1s", "0");

                  /* statuscode (altijd "0" overblijfsel van vroeger (annex Ax punt 127) */
                  fprintf(out_lfr, "%1.1s", "0");

                  /* nieuwe regel */
                  fprintf(out_lfr, "\n");

               } /* if ( (atoi(HM0_lfr) > 0) && (czz5_array[0] != LFR_ONTBREKEND) && (czz10_array[0] != LFR_ONTBREKEND) */
            } /* if ( (strcmp(HM0, ONTBREKEND) != 0) && (strcmp(HM0, "99999") != 0) ) */

         } /* if ( (aantal_meetwaarden > 0) && strcmp(input_files[i], "\0") != 0) */
      } /* for (i = 0; i < AANTAL_INPUT_FILES; i++) */

      fclose(out_cic);
      fclose(out_lfr);

   } /* else (outputfiles zijn te schrijven) */


   return 0;
}




/*****************************************************************************/
/*                                                                           */
/*                        Bepaal_CIC_en_LFR_Filenamen                        */
/*                                                                           */
/*****************************************************************************/
int Bepaal_CIC_en_LFR_Filenamen(char* cic_filenaam, char* lfr_filenaam)
{
   /* CIC filenaam voorbeeld: 2005040812.CIC */
   /* LFR filenaam voorbeeld: 2005040812.LFR */

   /* initialisatie */
   strcpy(cic_filenaam, "\0");
   strcpy(lfr_filenaam, "\0");

   /* datum in file tijd moet met eeuw aanduiding */
   strcpy(cic_filenaam, JJJJMMDDUU);
   strcat(cic_filenaam, ".CIC");

   strcpy(lfr_filenaam, JJJJMMDDUU);
   strcat(lfr_filenaam, ".LFR");

   /* ////// TEST ///////////
   fprintf(stderr, "%s", cic_filenaam);
   getchar();
    ////// TEST ///////////*/

   return 0;
}



