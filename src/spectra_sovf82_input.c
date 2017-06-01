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
int Read_SOVF82_Input_Files(void);
int Bepaal_SOVF82_Filenamen(void);
int Read_SOVF82_Write_CDS_LDS_DSP(const char* cds_filenaam, const char* lds_filenaam, const char* dsp_filenaam, const char* lds2_filenaam, const char* dsp2_filenaam);
int Bepaal_CDS_en_LDS_en_DSP_Filenamen(char* cds_filenaam, char* lds_filenaam, char* dsp_filenaam, char* lds2_filenaam, char* dsp2_filenaam);
int Bepaal_Dir_en_LFR_filenaam(char* dir_en_lfr_filenaam);

/* externe var's */
extern char JJJJMMDDUU[11];                        /* via argument list */
extern char OS[8];

/* var's globaal binnen deze module */
char input_files[AANTAL_INPUT_FILES][LENGTE_INPUT_FILENAAM];





/*****************************************************************************/
/*                                                                           */
/*                          Read_SOVF82_Input_Files                          */
/*                                                                           */
/*****************************************************************************/
int Read_SOVF82_Input_Files()
{
   char cds_filenaam[256];
   char lds_filenaam[256];
   char dsp_filenaam[256];
   char lds2_filenaam[256];                          /* voor frits koek i.v.m. database opslag (iets andere indeling dan lds) */
   char dsp2_filenaam[256];                          /* voor frits koek i.v.m. database opslag (iets andere indeling dan dsp) */


   Bepaal_SOVF82_Filenamen();                                                     /* input files */
   Bepaal_CDS_en_LDS_en_DSP_Filenamen(cds_filenaam, lds_filenaam, dsp_filenaam, lds2_filenaam, dsp2_filenaam);  /* output files */
   Read_SOVF82_Write_CDS_LDS_DSP(cds_filenaam, lds_filenaam, dsp_filenaam, lds2_filenaam, dsp2_filenaam);


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                              Bepaal_SOVF82_filenamen                      */
/*                                                                           */
/*****************************************************************************/
int Bepaal_SOVF82_Filenamen()
{
   /* inputfilenaam b.v.: MSS_R098123256_644_SOVF82_EHDB_081200 (totaal: 37 char) */




#if defined(WINDOWS)
   /* hier WINDOWS dir search algoritme implementeren */
   /* ............................................... */




   /*
   // hieronder test situatie bv opstarten als "spectra.exe 2006120512 SOVF82"
   */

   /* goede file, wel lds file aanmaken */

   strcpy(input_files[0], "test3.txt");
   input_files[0][9] = '\0';
   


   /* "foute file" zou geen lds file aangemaakt moeten worden */
   /*
   strcpy(input_files[0], "test.txt");
   input_files[0][8] = '\0';
   */

/*///// TEST ///////////
   fprintf(stderr, "%s", input_files[0]);
   getchar();
//// TEST ///////////*/


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


   pdir = opendir("input_sovf82/");                       /*"." refers to the current dir */
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

         /* MSS_R098123256_644_SOVF82_EHDB_081200 */
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

            //rename
         } /* if (strncmp(dag_uur_filenaam, dag_uur_JJJJMMDDUU, 4) == 0) */
      } /* while ((pent = readdir(pdir))) */
   } /* if (pdir) */
   else
   {
      /*printf ("opendir() failure; terminating");*/
      Write_Log("sovf82 opendir() failure");
      exit(1);
   } /* else */

#endif


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                      Read_SOVF82_Write_CDS_LDS_DSP                        */
/*                                                                           */
/*****************************************************************************/
int Read_SOVF82_Write_CDS_LDS_DSP(const char* cds_filenaam, const char* lds_filenaam, const char* dsp_filenaam, const char* lds2_filenaam, const char* dsp2_filenaam)
{
   char dir_en_cds_filenaam[512];
   char dir_en_lds_filenaam[512];
   char dir_en_dsp_filenaam[512];
   char dir_en_lds2_filenaam[512];
   char dir_en_dsp2_filenaam[512];
   char dir_en_input_filenaam[512];
   char volledig_path[512];
   char info[512];
   char line[256];
   char lfr_line[1024];
   char char_hulp_waarde[20];

   int m;
   int i;
   int k;
   int p;
   int a;
   int line_teller;
   int hulp_lengte;
   int pos;
   int num_hulp_waarde;
   int czz5_array_teller;
   int czz5_line_teller;
   int czz10_array_teller;
   int czz10_line_teller;
   int pos_old;
   int th010_array_teller;
   int th010_line_teller;
   int S0bh10_array_teller;
   int S0bh10_line_teller;
   int hulp_num_dd;
   int hulp_num_ff;
   int Dir_fp_indice;
   int num_aantal_fouten;
   int num_aantal_meetwaarden_methode_1;
   int num_aantal_meetwaarden_methode_2;
   int num_aantal_meetwaarden;
   int num_ghc_interpol_7_9;
   int num_ghc_interpol_9_11;
   int num_ghc_interpol_11_13;
   /*int lfr_alternatief;*/
   int b;
   int dsp_schrijven;
   int lds_sensorcode_no;
   int lfr_sensorcode_no;
   int aanvullen;                                           /* gebruikt als boolean */
   int z;                                                   /* teller */

   FILE* out_cds;
   FILE* out_lds;
   FILE* out_dsp;
   FILE* out_lds2;
   FILE* out_dsp2;
   FILE* in;
   FILE* lfr_in;
   
   /* (NB 20 array grootte is gewoon een willekeurig groot genoeg getal) */
   char sensorcode[20];
   char TE1[20];                                 /* cm**2 */        /* HFE (HE1) */
   char TE2[20];                                 /* cm**2 */        /* MFE (HE2) */
   char TE3[20];                                 /* cm**2 */        /* LFE (HE3) */
   char HM0_cds[20];                             /* cm */
   char Tm_10_cds[20];                           /* 0.1*s */
   char Tm_10_lds[20];                           /* 0.1*s */
   char dd_cds[20];                              /* 10 deg */
   char ff_cds[20];                              /* knots */
   char flag[20];
   char jaar_dag_uur_JJJJMMDDUU[20];
   char hulp_ff[20];
   char hulp_ddd[20];
   char latitude[20];
   char longitude[20];
   char quadrant[20];
   char th010_array[AANTAL_TH010][20];
   char czz5_array[AANTAL_CZZ5][20];
   char czz10_array[AANTAL_CZZ10][20];
   char S0bh10_array[AANTAL_S0BH10][20];
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
   char HM0_lds [20];                             /* mm */
   char dd_lds[20];
   char ff_lds[20];
   char TE0[20];
   char HE0[20];
   char HE1[20];
   char HE2[20];
   char HE3[20];
   char dd_alternatief[3];
   char ff_alternatief[3];
   char Hs7_lds[20];                             /* mm */
   char S_fp[20];
   char Dir_fp[20];
   char Th0_cds[20];
   char Th0_lds[20];
   char ghc_interpol_7_9[20];
   char ghc_interpol_9_11[20];
   char ghc_interpol_11_13[20];
   char S0bh[20];
   char Th3[20];
   char AV10_R[20];
   char DL_index[20];
   char lds_sensorcode_array[AANTAL_SENSORCODES][20];     /* opslag alle sensorcodes (bv K131, ANA1) die daadwerkelijk in lds file komen */
   char lfr_sensorcode_array[AANTAL_SENSORCODES][20];     /* opslag alle sensorcodes (bv K131, ANA1) die daadwerkelijk in lds file komen */
   char dir_en_lfr_filenaam[256];
   char lfr_sensorcode[5];

   float ff_factor;

   double double_hulp_waarde;
   double SP[AANTAL_CZZ10];                         /* voor dsp */
   double TH[AANTAL_CZZ10];                         /* voor dsp */
   double SPR[AANTAL_CZZ10];                        /* voor dsp */
   double SPTH[AANTAL_CZZ10][36];                   /* voor dsp */
   double FR[AANTAL_CZZ10];                         /* voor dsp */
   double DSTR[36];                                 /* voor dsp */
   double X;                                        /* voor dsp */
   double SMDSTR;                                   /* voor dsp */
   double AMPL;                                     /* voor dsp */
   double COSTH0;                                   /* voor dsp */

   int Czz5_ok;
   int Czz10_ok;


   /*
   // dir en naam .cds output file bepalen
   */
   strcpy(dir_en_cds_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_cds_filenaam, "output_cds\\");
   else
      /* strcpy(dir_en_cds_filenaam, "output_cds/"); */
      strcpy(dir_en_cds_filenaam, getenv("ENV_SPECTRA_CDS"));     /* ivm APL */
   strcat(dir_en_cds_filenaam, cds_filenaam);


   /*
   // dir en naam .lds output file bepalen
   */
   strcpy(dir_en_lds_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_lds_filenaam, "output_lds\\");
   else
      /* strcpy(dir_en_lds_filenaam, "output_lds/"); */
      strcpy(dir_en_lds_filenaam, getenv("ENV_SPECTRA_LDS"));     /* ivm APL */
   strcat(dir_en_lds_filenaam, lds_filenaam);


   /*
   // dir en naam .dsp output file bepalen
   */
   strcpy(dir_en_dsp_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_dsp_filenaam, "output_dsp\\");
   else
      /*strcpy(dir_en_dsp_filenaam, "output_dsp/");*/
      strcpy(dir_en_dsp_filenaam, getenv("ENV_SPECTRA_DSP"));     /* ivm APL */
   strcat(dir_en_dsp_filenaam, dsp_filenaam);


   /*
   // dir en naam .lds2 output file bepalen
   */
   strcpy(dir_en_lds2_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_lds2_filenaam, "output_lds\\");
   else
      strcpy(dir_en_lds2_filenaam, getenv("ENV_SPECTRA_LDS"));
   strcat(dir_en_lds2_filenaam, lds2_filenaam);


   /*
   // dir en naam .dsp2 output file bepalen
   */
   strcpy(dir_en_dsp2_filenaam, "\0");

   if (strcmp(OS, "WINDOWS") == 0)
      strcpy(dir_en_dsp2_filenaam, "output_dsp\\");
   else
      strcpy(dir_en_dsp2_filenaam, getenv("ENV_SPECTRA_DSP"));
   strcat(dir_en_dsp2_filenaam, dsp2_filenaam);


   /*
   // openen cds, lds, dsp, lds2 en dsp2 output files
   */
   if ( ((out_cds  = fopen(dir_en_cds_filenaam, "w")) == NULL) ||
        ((out_lds  = fopen(dir_en_lds_filenaam, "w")) == NULL) ||
        ((out_dsp  = fopen(dir_en_dsp_filenaam, "w")) == NULL) ||
        ((out_lds2 = fopen(dir_en_lds2_filenaam, "w")) == NULL) ||
        ((out_dsp2 = fopen(dir_en_dsp2_filenaam, "w")) == NULL) )               /* dus mislukt */

   {
      getcwd(volledig_path, 512);

      /* bericht samen stellen */
      strcpy(info, "\0");

      if (strcmp(OS, "WINDOWS") == 0)
      {
         strcat(info, volledig_path);      /* de environment dir var geeft al een volledig path onder unix */
         strcat(info, "\\");
      }

      if (out_cds == NULL)
      {
         strcat(info, dir_en_cds_filenaam);
         strcat(info, " ");
      }

      if (out_lds == NULL)
      {
         strcat(info, dir_en_lds_filenaam);
         strcat(info, " ");
      }

      if (out_dsp == NULL)
      {
         strcat(info, dir_en_dsp_filenaam);
         strcat(info, " ");
      }

      if (out_lds2 == NULL)
      {
         strcat(info, dir_en_lds2_filenaam);
         strcat(info, " ");
      }

      if (out_dsp2 == NULL)
      {
         strcat(info, dir_en_dsp2_filenaam);
         strcat(info, " ");
      }

      strcat(info, " niet te schrijven\n");

      /* naar log schrijven */
      Write_Log(info);

      /* nu zou het kunnen dat een van de 3 wel goed was, deze dan sluiten om geheel correct te zijn */
#if 0            /* geeft core dump */

      if (out_cds != NULL)
         fclose(out_cds);

      if (out_lds != NULL)
         fclose(out_lds);

      if (out_dsp != NULL)
         fclose(out_dsp);
#endif

   } /* if ((out = fopen(dir_en_cds_filenaam, "w")) == NULL) etc.  */
   else /* outputfiles zijn alle drie te schrijven */
   {
      /* cds output */
      fclose(out_cds);                                          /* file is nu weer leeg */
      out_cds = fopen(dir_en_cds_filenaam, "a");                /* moet nu telkens aangevuld (append) worden */

      /* lds output */
      fclose(out_lds);                                          /* file is nu weer leeg */
      out_lds = fopen(dir_en_lds_filenaam, "a");                /* moet nu telkens aangevuld (append) worden */

      /* dsp output */
      fclose(out_dsp);                                          /* file is nu weer leeg */
      out_dsp = fopen(dir_en_dsp_filenaam, "a");                /* moet nu telkens aangevuld (append) worden */

      /* lds2 output */
      fclose(out_lds2);                                          /* file is nu weer leeg */
      out_lds2 = fopen(dir_en_lds2_filenaam, "a");               /* moet nu telkens aangevuld (append) worden */

      /* dsp2 output */
      fclose(out_dsp2);                                          /* file is nu weer leeg */
      out_dsp2 = fopen(dir_en_dsp2_filenaam, "a");               /* moet nu telkens aangevuld (append) worden */



      /* array met lfr sensorcode namen i.v.m. preprocessing lds aanvulling met lfr data) */
      lfr_sensorcode_no = 0;
      for (k = 0; k < AANTAL_SENSORCODES; k++)
         strcpy(lfr_sensorcode_array[k], LFR_ONTBREKEND);

      /* array met sensorcode namen waarvan de data daadwerkelijk in lds files is opgeslagen (om te kunnen aanvullen met data uit lfr file) */
      lds_sensorcode_no = 0;
      for (k = 0; k < AANTAL_SENSORCODES; k++)
         strcpy(lds_sensorcode_array[k], LDS_ONTBREKEND);



      /* openen input file(s) */
      for (i = 0; i < AANTAL_INPUT_FILES; i++)
      {
         if (strcmp(input_files[i], "\0") != 0)
         {
            /* initialisatie */
            Czz5_ok = 1;                                 /* true */
            Czz10_ok = 1;                                /* true */

            /* initialisatie cds + lds */
            strcpy(sensorcode, "\0");
            /*strcpy(hulp_ddd,   "\0"); */                                  /* deg */

            /* initialisatie specifiek cds */
            strcpy(HM0_cds,      "\0");                                 /* cm */
            strcpy(Tm_10_cds,    "\0");                                 /* 0.1*s */   /* Ts */
            strcpy(flag,         "\0");
            strcpy(dd_cds,       "\0");                                 /* 10 deg */
            strcpy(ff_cds,       "\0");                                 /* knots */
            strcpy(Th0_cds,      "\0");                                 /* graden */

            /* NB initialisatie met "0" voor lds komt voort uit de wijze hoe Evert dit deed (ook 0 in lds indien niet aanwezig)
            /*    bij dd en ff moet indien ontbrekend dit 99 (dd) en 99 (ff) worden ! */

            for (k = 0; k < AANTAL_CZZ5; k++)
               strcpy(czz5_array[k], LDS_ONTBREKEND);

            for (k = 0; k < AANTAL_CZZ10; k++)
               strcpy(czz10_array[k], LDS_ONTBREKEND);

            for (k = 0; k < AANTAL_TH010; k++)
               strcpy(th010_array[k], LDS_TH010_ONTBREKEND);

            for (k = 0; k < AANTAL_S0BH10; k++)
               strcpy(S0bh10_array[k], LDS_S0BH10_ONTBREKEND);


            strcpy(aantal_fouten,      LDS_ONTBREKEND);
            strcpy(aantal_meetwaarden, LDS_ONTBREKEND);
            strcpy(AV10_H,             LDS_ONTBREKEND);
            strcpy(TE0,                LDS_ONTBREKEND);
            strcpy(Fp,                 LDS_ONTBREKEND);
            strcpy(Tm02,               LDS_ONTBREKEND);
            strcpy(aantal_golven,      LDS_ONTBREKEND);
            strcpy(H1d3,               LDS_ONTBREKEND);
            strcpy(H1d10,              LDS_ONTBREKEND);
            strcpy(GGh,                LDS_ONTBREKEND);
            strcpy(Hmax,               LDS_ONTBREKEND);
            strcpy(T1d3,               LDS_ONTBREKEND);
            strcpy(TH1d3,              LDS_ONTBREKEND);
            strcpy(GGT,                LDS_ONTBREKEND);
            strcpy(Tmax,               LDS_ONTBREKEND);
            strcpy(THmax,              LDS_ONTBREKEND);
            strcpy(Ndir_H,             LDS_ONTBREKEND);
            strcpy(Ngd_zP,             LDS_ONTBREKEND);
            strcpy(Nd_z,               LDS_ONTBREKEND);
            strcpy(Nu_z,               LDS_ONTBREKEND);
            strcpy(Nv_z,               LDS_ONTBREKEND);
            strcpy(Ni_z,               LDS_ONTBREKEND);
            strcpy(HM0_lds,            LDS_ONTBREKEND);
            strcpy(dd_lds,             LDS_ONTBREKEND);                       /* 10 deg */
            strcpy(ff_lds,             LDS_ONTBREKEND);                       /* knots */
            strcpy(TE0,                "\0");
            strcpy(TE1,                "\0");                                 /* cm**2 */
            strcpy(TE2,                "\0");                                 /* cm**2 */
            strcpy(TE3,                "\0");                                 /* cm**2 */
            strcpy(HE0,                LDS_ONTBREKEND);
            strcpy(HE1,                LDS_ONTBREKEND);
            strcpy(HE2,                LDS_ONTBREKEND);
            strcpy(HE3,                LDS_ONTBREKEND);
            strcpy(Tm_10_lds,          LDS_ONTBREKEND);                       /* 0.1*s */   /* Ts, TM0-1 */
            strcpy(Hs7_lds,            LDS_ONTBREKEND);                       /* mm */
            strcpy(H1d50,              LDS_ONTBREKEND);
            strcpy(ghc_interpol_7_9,   LDS_ONTBREKEND);
            strcpy(ghc_interpol_9_11,  LDS_ONTBREKEND);
            strcpy(ghc_interpol_11_13, LDS_ONTBREKEND);
            strcpy(Th0_lds,            LDS_ONTBREKEND);                       /* graden */
            strcpy(S0bh,               LDS_ONTBREKEND);
            strcpy(Th3,                LDS_ONTBREKEND);
            strcpy(AV10_R,             LDS_ONTBREKEND);
            strcpy(DL_index,           LDS_ONTBREKEND);



            /* initilisatie DSP */
            for (a = 0; a < AANTAL_CZZ10; a++)
               for (b = 0; b < 36; b++)
                  SPTH[a][b] = 0.0;

            for (b = 0; b < AANTAL_CZZ10; b++)
            {
               FR[b] = 0.0;
               SP[b] = 0.0;
               TH[b] = 0.0;
               SPR[b] = 0.0;
            } /* for (b = 0; b < AANTAL_CZZ10; b++) */

            for (b = 0; b < 36; b++)
               DSTR[b] = 0.0;


            /* dus een geldige file naam aanwezig in array input_files */
            line_teller = 0;

            /* input file namem + dir bepalen */
            strcpy(dir_en_input_filenaam, "\0");

            if (strcmp(OS, "WINDOWS") == 0)
               strcpy(dir_en_input_filenaam, "input_sovf82\\");
            else
               strcpy(dir_en_input_filenaam, "input_sovf82/");

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
                        if (strlen(sensorcode) == 4)                                /* bv NC1; strlen telt WEL '\n' */
                        {
                           sensorcode[3] = '\0';
                           strcat(sensorcode, " ");
                        } /* if (strlen(sensorcode) == 4) */

                        sensorcode[4] = '\0';                                       /* bv sensorcode K131 */

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
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Ndir_H, line + pos);                   /* de "G " niet in lijstje */
                           Ndir_H[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Ndir_H, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Ndir_H, LDS_ONTBREKEND);
                  } /* if (line_teller == 7) */


                  /*
                  //////////////// Ngd_zP (% goed binnengekomen golfhoogte puntent.o.v. totaal aantal verwachte punten)
                  */
                  if (line_teller == 8)                            /* Ngd_zP */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Ngd_zP, line + pos);                   /* de "G " niet in lijstje */
                           Ngd_zP[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Ngd_zP, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Ngd_zP, LDS_ONTBREKEND);
                  } /* if (line_teller == 8) */


                  /*
                  //////////////// Nd_z (aantal vanwegw delta-fout afgekeurde golfhoogtepunten)
                  */
                  if (line_teller == 9)                            /* Ndir_H */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Nd_z, line + pos);                   /* de "G " niet in lijstje */
                           Nd_z[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Nd_z, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Nd_z, LDS_ONTBREKEND);
                  } /* if (line_teller == 9) */


                  /*
                  //////////////// Nv_z (aantal vanwege 4-sigma-fout afgekeurde golfhoogtepunten)
                  */
                  if (line_teller == 10)                            /* Nv_Z */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Nv_z, line + pos);                   /* de "G " niet in lijstje */
                           Nv_z[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Nv_z, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Nv_z, LDS_ONTBREKEND);
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
                           strcpy(Nu_z, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Nu_z, LDS_ONTBREKEND);
                  } /* if (line_teller == 11) */


                  /*
                  //////////////// Ni_z (aantal geinterpoleerde of geextrapoleerde golfhoogtepunten)
                  */
                  if (line_teller == 12)                            /* Ni_z */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)           /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )            /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Ni_z, line + pos);                /* de "G " niet in lijstje */
                           Ni_z[hulp_lengte -1 -pos] = '\0';        /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                          strcpy(Ni_z, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Ni_z, LDS_ONTBREKEND);
                  } /* if (line_teller == 7) */



                  /*
                  //////////// aantal golven (AG)
                  */
                  if (line_teller == 26)
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                 /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(aantal_golven, line + pos);          /* de "G " niet in lijstje */
                           aantal_golven[hulp_lengte -1 -pos] = '\0';  /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(aantal_golven, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(aantal_golven, LDS_ONTBREKEND);
                  } /* if (line_teller == 26) */


                  /*
                  //////////////// HE0 (afgeleid van TE0)
                  */
                  if (line_teller == 29)                            /* TE1 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(TE0, line + pos);                 /* de "G " niet in lijstje */
                           TE0[hulp_lengte -1 - pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lds [HE0 = wortel(TE0) * 4]                                                         */
                           /*                  LET op: in de sovf80 file staat: TE0 in cm**                       */
                           /*                          in de LDS file staat HE0 in mm [HE0 = wortel(TE0) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE0) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE0, "%d", num_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                           strcpy(HE0, LDS_ONTBREKEND);

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(HE0, LDS_ONTBREKEND);
                  } /* if (line_teller == 29) */


                  /*
                  //////////////// HE1 (HFE) hoog frequentie (afgele1d van TE1)
                  */
                  if (line_teller == 30)                            /* TE1 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv G 8'\0' (dus inclusief '\0') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(TE1, line + pos);                   /* de "G " niet in lijstje */
                           TE1[hulp_lengte -1 -pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lds [HE1 = wortel(TE1) * 4]                                                         */
                           /*                  LET op: in de sovf82 file staat: TE1 in cm**                       */
                           /*                          in de LDS file staat HE1 in mm [HE1 = wortel(TE1) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE1) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE1, "%d", num_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                           strcpy(HE1, LDS_ONTBREKEND);

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(HE1, LDS_ONTBREKEND);
                  } /* if (line_teller == 30) */


                  /*
                  //////////// HE2 (MFE) midden frequentie (afgeleid van TE2)
                  */
                  if (line_teller == 31)                            /* TE2 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(TE2, line + pos);                 /* de "G " niet in lijstje */
                           TE2[hulp_lengte -1 -pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lds [HE2 = wortel(TE2) * 4]                                                         */
                           /*                  LET op: in de sovf82 file staat: TE2 in cm**                       */
                           /*                          in de LDS file staat HE2 in mm [HE2 = wortel(TE2) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE2) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE2, "%d", num_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                           strcpy(HE2, LDS_ONTBREKEND);

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(HE2, LDS_ONTBREKEND);
                  } /* if (line_teller == 31) */


                  /*
                  //////////// HE3 (LFE)  laag frequentie (afgeleid van TE3)
                  */
                  if (line_teller == 32)                            /* TE3 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(TE3, line + pos);                 /* de "G " niet in lijstje */
                           TE3[hulp_lengte -1 -pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lds [HE3 = wortel(TE3) * 4]                                                         */
                           /*                  LET op: in de sovf82 file staat: TE3 in cm**                       */
                           /*                          in de LDS file staat HE3 in mm [HE3 = wortel(TE3) * 4]     */
                           /*                           1 cm** = 100 mm**                                         */
                           double_hulp_waarde = atof(TE3) * 100;   /* nu in mm kwadraad */
                           num_hulp_waarde = (int)(sqrt(double_hulp_waarde) * 4 + 0.5);
                           sprintf(HE3, "%d", num_hulp_waarde);
                        }  /* if (strncmp(line, "G ", 2) == 0) */
                        else
                           strcpy(HE3, LDS_ONTBREKEND);

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(HE3, LDS_ONTBREKEND);
                  } /* if (line_teller == 32) */



                  /*
                  //////////// HM0
                  */
                  if (line_teller == 35)                            /* HM0 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor CDS (cm) */
                           strcpy(HM0_cds, line + pos);                 /* de "G " niet in lijstje */
                           HM0_cds[hulp_lengte -1 - pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* voor LDS:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 - pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(HM0_lds, char_hulp_waarde);
                        }
                        else
                        {
                           strcpy(HM0_cds, CDS_ONTBREKEND);
                           strcpy(HM0_lds, LDS_ONTBREKEND);
                        }
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(HM0_cds, CDS_ONTBREKEND);
                        strcpy(HM0_lds, LDS_ONTBREKEND);
                     }
                  } /* if (line_teller == 35) */



                  /*
                  //////////// Tm02 (Tm02)
                  */
                  if (line_teller == 36)                               /* Tm02 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)            /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                 /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Tm02, line + pos);               /* de "G " niet in lijstje */
                           Tm02[hulp_lengte -1 -pos] = '\0';       /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Tm02, LDS_ONTBREKEND);

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Tm02, LDS_ONTBREKEND);
                  } /* if (line_teller == 36) */



                  /*
                  //////////// Tm_10 (TM0-1, Ts)
                  */
                  if (line_teller == 37)                            /* Tm_10 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* cds */
                           strcpy(Tm_10_cds, line + pos);               /* de "G " niet in lijstje */
                           Tm_10_cds[hulp_lengte -1 -pos] = '\0';       /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           /* lds */
                           strcpy(Tm_10_lds, line + pos);               /* de "G " niet in lijstje */
                           Tm_10_lds[hulp_lengte -1 -pos] = '\0';       /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                        {
                           strcpy(Tm_10_cds, CDS_ONTBREKEND);
                           strcpy(Tm_10_lds, LDS_ONTBREKEND);
                        }

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                     {
                        strcpy(Tm_10_cds, CDS_ONTBREKEND);
                        strcpy(Tm_10_lds, LDS_ONTBREKEND);
                     }
                  } /* if (line_teller == 37) */


                  /*
                  //////////// Fp (piekfrequentie)
                  */
                  if (line_teller == 38)                            /* fp */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(Fp, line + pos);                  /* de "G " niet in lijstje */
                           Fp[hulp_lengte -1 -pos] = '\0';          /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(Fp, LDS_ONTBREKEND);
                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Fp, LDS_ONTBREKEND);
                  } /* if (line_teller == 38) */



                  /*
                  //////////// Hs7
                  */
                  if (line_teller == 39)                            /* Hs7 */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)         /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel moet beginnen met "G " */
                        {
                           pos = 2;

                           /* voor lds:  moet mm worden (staat in input file in cm) */
                           char_hulp_waarde[0] = '\0';
                           strcpy(char_hulp_waarde, line + pos);
                           char_hulp_waarde[hulp_lengte -1 -pos] = '\0';
                           num_hulp_waarde = atoi(char_hulp_waarde) * 10;
                           char_hulp_waarde[0] = '\0';
                           sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                           strcpy(Hs7_lds, char_hulp_waarde);
                        } /* if (strncmp(line, "G ", 2) == 0) */
                        else
                           strcpy(Hs7_lds, LDS_ONTBREKEND);

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(Hs7_lds, LDS_ONTBREKEND);
                  } /* if (line_teller == 39) */


                  /*
                  //////////// AV10_H (vrijheidsgraden spectrum componenten)
                  */
                  if (line_teller == 40)                                /* fp */
                  {
                     if ((hulp_lengte = strlen(line)) >= 4)             /* bv "G 8'\n'" (dus inclusief '\n') */
                     {
                        if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                  /* regel moet beginnen met "G " */
                        {
                           pos = 2;
                           strcpy(AV10_H, line + pos);                  /* de "G " niet in lijstje */
                           AV10_H[hulp_lengte -1 -pos] = '\0';          /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */
                        }
                        else
                           strcpy(AV10_H, LDS_ONTBREKEND);

                     } /* if ((hulp_lengte = strlen(line)) >= 4) */
                     else
                        strcpy(AV10_H, LDS_ONTBREKEND);
                  } /* if (line_teller == 40) */




                  /*
                  //////////// Czz5 en Czz10
                  //           omdat de regels met frequenties wat kan verschillen testen op GH_C label
                  */
                  /* eerst line met aanduiding "GH_C" localiseren  */
                  if ((hulp_lengte = strlen(line)) == 5)                        /* geeft lengte inclusief '\ n' */
                  {
                     if (strncmp(line, "GH_C", 4) == 0)
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
                                    if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel OK */
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
                                 Czz5_ok = 0;                              /* false */
                                 break;
                              }  /* else */
                           } /* if (fgets(line, 255, in) != NULL) */
                           else
                           {
                              Czz5_ok = 0;                                /* false */
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
                                    if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G 99999", 7) != 0) )              /* regel OK */
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

                              /* ////// TEST ///////////
                              if (czz10_array_teller == 0)
                              {
                                fprintf(stderr, "%s", czz10_array[0]);
                                getchar();
                              }
                              ///// TEST ///////////*/


                                             pos_old = pos + 1;                  /* +1 om net verder dan de " " komen */
                                             czz10_array_teller++;
                                          } /* if (strncmp(line + pos, " ", 1) == 0) */
                                       } /* for (i = 0; i < hulp_lengte; i++) */
                                    } /* if (strncmp(line, "G", 1) == 0) */
                                    else
                                    {
                                       Czz10_ok = 0;                              /* false */
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
                                 Czz10_ok = 0;                              /* false */
                                 break;
                              }  /* else */
                           } /* if (fgets(line, 255, in) != NULL) */
                           else
                           {
                              Czz10_ok = 0;                              /* false */
                              break;
                           }  /* else */
                        } /* while (czz10_array_teller < AANTAL_CZZ10) */

                     } /* if (strncmp(line, "GH_C", 4) == 0) */
                  } /* if ((hulp_lengte = strlen(line)) == 5) */




                  /*
                  //////////// Th010
                  //           omdat de regels met frequenties wat kan verschillen testen op GR label
                  */
                  /* eerst line met aanduiding "GR" localiseren  */
                  if ((hulp_lengte = strlen(line)) == 3)                      /* geeft lengte inclusief '\ n' */
                  {
                     if (strncmp(line, "GR", 2) == 0)
                     {

                        /*
                        ///////////////// Th0 (gemiddelde richting totale spectrum) (Annex C punt 103)
                        */
                        fgets(line, 255, in);
                        if ((hulp_lengte = strlen(line)) >= 4)                /* bv "G 8'\n'" (dus inclusief '\n') */
                        {
                           if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                   /* regel moet beginnen met "G " */
                           {
                              pos = 2;

                              /* cds */
                              strcpy(Th0_cds, line + pos);                    /* de "G " niet in lijstje */
                              Th0_cds[hulp_lengte - 1 - pos] = '\0';          /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                              /* lds */
                              strcpy(Th0_lds, line + pos);                    /* de "G " niet in lijstje */
                              Th0_lds[hulp_lengte - 1 - pos] = '\0';          /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           } /* if (strncmp(line, "G ", 2) == 0) */
                           else
                           {
                              strcpy(Th0_cds, CDS_ONTBREKEND);
                              strcpy(Th0_lds, LDS_ONTBREKEND);
                           }

                        } /* if ((hulp_lengte = strlen(line)) >= 4) */
                        else
                        {
                           strcpy(Th0_cds, CDS_ONTBREKEND);
                           strcpy(Th0_lds, LDS_ONTBREKEND);
                        }


                        /*
                        //////////////////////////// Sobh (gem. spreiding totale spectrum)(Annex C punt 104)
                        */
                        fgets(line, 255, in);
                        if ((hulp_lengte = strlen(line)) >= 4)                /* bv "G 8'\n'" (dus inclusief '\n') */
                        {
                           if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                   /* regel moet beginnen met "G " */
                           {
                              pos = 2;
                              strcpy(S0bh, line + pos);                       /* de "G " niet in lijstje */
                              S0bh[hulp_lengte - 1 - pos] = '\0';             /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           } /* if (strncmp(line, "G ", 2) == 0) */
                           else
                              strcpy(S0bh, LDS_ONTBREKEND);

                        } /* if ((hulp_lengte = strlen(line)) >= 4) */
                        else
                           strcpy(S0bh, LDS_ONTBREKEND);


                        /*
                        ///////////////////////////// Th3 (gem. richting LFE) (Annex C punt 105)
                        */
                        fgets(line, 255, in);
                        if ((hulp_lengte = strlen(line)) >= 4)                /* bv "G 8'\n'" (dus inclusief '\n') */
                        {
                           if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                   /* regel moet beginnen met "G " */
                           {
                              pos = 2;
                              strcpy(Th3, line + pos);                        /* de "G " niet in lijstje */
                              Th3[hulp_lengte - 1 - pos] = '\0';              /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           } /* if (strncmp(line, "G ", 2) == 0) */
                           else
                              strcpy(Th3, LDS_ONTBREKEND);

                        } /* if ((hulp_lengte = strlen(line)) >= 4) */
                        else
                           strcpy(Th3, LDS_ONTBREKEND);


                        /*
                        ////////////////////////////////// AV10_R (aantal vrijheidsgraden sp.el.) (Annex C punt 106)
                        */
                        fgets(line, 255, in);
                        if ((hulp_lengte = strlen(line)) >= 4)                /* bv "G 8'\n'" (dus inclusief '\n') */
                        {
                           if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                   /* regel moet beginnen met "G " */
                           {
                              pos = 2;
                              strcpy(AV10_R, line + pos);                     /* de "G " niet in lijstje */
                              AV10_R[hulp_lengte - 1 - pos] = '\0';           /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           } /* if (strncmp(line, "G ", 2) == 0) */
                           else
                              strcpy(AV10_R, LDS_ONTBREKEND);

                        } /* if ((hulp_lengte = strlen(line)) >= 4) */
                        else
                           strcpy(AV10_R, LDS_ONTBREKEND);


                        /*
                        ///////////////////// DL_index (golfgetal DL_index)
                        */
                        fgets(line, 255, in);
                        if ((hulp_lengte = strlen(line)) >= 4)                /* bv "G 8'\n'" (dus inclusief '\n') */
                        {
                           if ( (strncmp(line, "G ", 2) == 0) && (strncmp(line, "G 99999", 7) != 0) )                   /* regel moet beginnen met "G " */
                           {
                              pos = 2;
                              strcpy(DL_index, line + pos);                   /* de "G " niet in lijstje */
                              DL_index[hulp_lengte - 1 - pos] = '\0';         /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                           } /* if (strncmp(line, "G ", 2) == 0) */
                           else
                              strcpy(DL_index, LDS_ONTBREKEND);

                        } /* if ((hulp_lengte = strlen(line)) >= 4) */
                        else
                           strcpy(DL_index, LDS_ONTBREKEND);


                        /*
                        /////////////////////// Th010 (Annex C punt 108 - 158) (gem. richting)
                        */

                        /* initialisatie  */
                        th010_array_teller = 0;
                        th010_line_teller  = 0;

                        while (th010_array_teller < AANTAL_TH010)               /* zijn altijd 51 waarden in th010 blok */
                        {
                           if (fgets(line, 255, in) != NULL)                     /* regel inlezen */
                           {
                              th010_line_teller++;
                              if ((hulp_lengte = strlen(line)) >= 2)
                              {
                                 if (th010_line_teller == 1)                      /* eerst regel van th010 staat een G als het goed is */
                                 {
                                    if ( (strncmp(line, "G", 1) == 0) /*&& (strncmp(line, "G 99999", 7) != 0)*/ )             /* regel OK */
                                    {
                                       /* in de praktijk blijkt dat er soms bij de eerste "G 99999"staat maar hierna */
                                       /* toch weer reeele waarde daarom niet testen op "G 99999" bij Th010 en S0bh10 */

                                       /* ingelezen line scannen */
                                       pos_old = 2;
                                       for (pos = 3; pos < hulp_lengte; pos++)   /* "G " overslaan */
                                       {
                                          if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )  /* voorbij eerste " " */
                                          {
                                             /* staat in graden */
                                             char_hulp_waarde[0] = '\0';
                                             strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                             char_hulp_waarde[pos - pos_old] = '\0';
                                             num_hulp_waarde = atoi(char_hulp_waarde) * 1;

                                             if (num_hulp_waarde != 99999)      /* als er 99999 stond een ongeldige waarde */
                                             {
                                                char_hulp_waarde[0] = '\0';
                                                sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                                strcpy(th010_array[th010_array_teller], char_hulp_waarde);
                                             }
                                             else
                                             {
                                                /* Evert zet dan "9999" (lds format is ook voor 4 char) */
                                                strcpy(th010_array[th010_array_teller], "9999");
                                             }

                                             pos_old = pos + 1;                   /* +1 om net verder dan de " " komen */
                                             th010_array_teller++;
                                          } /* if (strncmp(line + pos, " ", 1) == 0) */
                                       } /* for (i = 0; i < hulp_lengte; i++) */
                                    } /* if (strncmp(line, "G", 1) == 0) */
                                    else
                                       break;                                       /* dus bij een "N" er uit springen (wordt niets in aaray geschreven) */
                                 } /* if (th010_line_teller == 1) */
                                 else /* dus niet eerste regel van th010 blok */
                                 {
                                    /* ingelezen line scannen (i.t.t. de eerste line beginnen volgende niet met "G") */
                                    pos_old = 0;
                                    for (pos = 1; pos < hulp_lengte; pos++)
                                    {
                                       if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )
                                       {
                                          /* staat in graden */
                                          char_hulp_waarde[0] = '\0';
                                          strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                          char_hulp_waarde[pos - pos_old] = '\0';
                                          num_hulp_waarde = atoi(char_hulp_waarde) * 1;

                                          if (num_hulp_waarde != 99999)      /* als er 99999 stond een ongeldige waarde */
                                          {
                                             char_hulp_waarde[0] = '\0';
                                             sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                             strcpy(th010_array[th010_array_teller], char_hulp_waarde);
                                          }
                                          else
                                          {
                                             /* Evert zet dan "9999" (lds format is ook voor 4 char) */
                                             strcpy(th010_array[th010_array_teller], "9999");
                                          }

                                          pos_old = pos +1;                   /* +1 om net verder dan de " " komen */
                                          th010_array_teller++;
                                       } /* if (strncmp(line + pos, " ", 1) == 0) */
                                    } /* for (i = 0; i < hulp_lengte; i++) */
                                 } /* else (niet eerst eregel van th010 blok) */

                              } /* if ((hulp_lengte = strlen(line)) >= 2) */
                              else
                              {
                                 break;
                              }  /* else */
                           } /* if (fgets(line, 255, in) != NULL) */
                           else
                           {
                              break;
                           }  /* else */
                        } /* while (th010_array_teller < AANTAL_TH010) */



                        /*
                        /////////////////////// S0bh10 (Annex C punt 159 - 209) (spreiding)
                        */

                        /* initialisatie  */
                        S0bh10_array_teller = 0;
                        S0bh10_line_teller = 0;

                        while (S0bh10_array_teller < AANTAL_S0BH10)              /* zijn altijd 51 waarden in S0bh10 blok */
                        {
                           if (fgets(line, 255, in) != NULL)                     /* regel inlezen */
                           {
                              S0bh10_line_teller++;
                              if ((hulp_lengte = strlen(line)) >= 2)
                              {
                                 if (S0bh10_line_teller == 1)                     /* eerst regel van Sobh10 staat een G als het goed is */
                                 {
                                    if ( (strncmp(line, "G", 1) == 0) /*&& (strncmp(line, "G 99999", 7) != 0)*/ )              /* regel OK */
                                    {
                                       /* in de praktijk blijkt dat er soms bij de eerste "G 99999"staat maar hierna */
                                       /* toch weer reeele waarde daarom niet testen op "G 99999" bij Th010 en S0bh10 */


                                       /* ingelezen line scannen */
                                       pos_old = 2;
                                       for (pos = 3; pos < hulp_lengte; pos++)   /* "G " overslaan */
                                       {
                                          if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )  /* voorbij eerste " " */
                                          {
                                             /* staat in graden */
                                             char_hulp_waarde[0] = '\0';
                                             strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                             char_hulp_waarde[pos - pos_old] = '\0';
                                             num_hulp_waarde = atoi(char_hulp_waarde);

                                             if (num_hulp_waarde != 99999)      /* als er 99999 stond een ongeldige waarde */
                                             {
                                                char_hulp_waarde[0] = '\0';
                                                sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                                strcpy(S0bh10_array[S0bh10_array_teller], char_hulp_waarde);
                                             }
                                             else
                                             {
                                                /* Evert zet dan "9999" (lds format is ook voor 4 char) */
                                                strcpy(S0bh10_array[S0bh10_array_teller], "9999");
                                             }

                                             pos_old = pos + 1;                   /* +1 om net verder dan de " " komen */
                                             S0bh10_array_teller++;
                                          } /* if (strncmp(line + pos, " ", 1) == 0) */
                                       } /* for (i = 0; i < hulp_lengte; i++) */
                                    } /* if (strncmp(line, "G", 1) == 0) */
                                    else
                                       break;                                       /* dus bij een "N" er uit springen (wordt niets in aaray geschreven) */
                                 } /* if (S0bh10_line_teller == 1) */

                                 else /* dus niet eerste regel van S0bh10 blok */
                                 {
                                    /* ingelezen line scannen (i.t.t. de eerste line beginnen volgende niet met "G") */
                                    pos_old = 0;
                                    for (pos = 1; pos < hulp_lengte; pos++)
                                    {
                                       if ( (strncmp(line + pos, " ", 1) == 0) || (strncmp(line + pos, "\n", 1) == 0) )
                                       {
                                          /* staat in graden */
                                          char_hulp_waarde[0] = '\0';
                                          strncpy(char_hulp_waarde, line + pos_old, pos - pos_old);
                                          char_hulp_waarde[pos - pos_old] = '\0';
                                          num_hulp_waarde = atoi(char_hulp_waarde);

                                          if (num_hulp_waarde != 99999)      /* als er 99999 stond een ongeldige waarde */
                                          {
                                             char_hulp_waarde[0] = '\0';
                                             sprintf(char_hulp_waarde, "%d", num_hulp_waarde);
                                             strcpy(S0bh10_array[S0bh10_array_teller], char_hulp_waarde);
                                          }
                                          else
                                          {
                                             /* Evert zet dan "9999" (lds format is ook voor 4 char) */
                                             strcpy(S0bh10_array[S0bh10_array_teller], "9999");
                                          }

                                          pos_old = pos + 1;                   /* +1 om net verder dan de " " komen */
                                          S0bh10_array_teller++;
                                       } /* if (strncmp(line + pos, " ", 1) == 0) */
                                    } /* for (i = 0; i < hulp_lengte; i++) */
                                 } /* else (niet eerst eregel van S0bh10 blok) */

                              } /* if ((hulp_lengte = strlen(line)) >= 2) */
                              else
                              {
                                 break;
                              }  /* else */
                           } /* if (fgets(line, 255, in) != NULL) */
                           else
                           {
                              break;
                           }  /* else */
                        } /* while (S0bh10_array_teller < AANTAL_S0BH10) */

                     } /* if (strncmp(line, "GR", 2) == 0) */
                  } /* if ((hulp_lengte = strlen(line)) == 3) */



                  /*
                  //////////// dd en ff
                  //           omdat de regels met frequenties czz5/czz10/th010/S0bh10 kunnen verschillen in aantal
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
                              if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G99999", 6) != 0) && (ff_factor != 99999) )   /* regel moet beginnen met "G" */
                              {
                                 pos = 1;
                                 strcpy(hulp_ff, line + pos);                    /* de "G" niet in lijstje */
                                 hulp_ff[hulp_lengte -1 -pos] = '\0';            /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                                 hulp_num_ff = (int)(atof(hulp_ff) / ff_factor + 0.5);
                                 sprintf(ff_cds, "%d", hulp_num_ff);
                                 sprintf(ff_lds, "%d", hulp_num_ff);

                              } /* if ( (strncmp(line, "G", 1) == 0) && (ff_factor != 99999) ) */
                              else
                              {
                                 strcpy(ff_cds, CDS_ONTBREKEND);
                                 strcpy(ff_lds, FF_LDS_ONTBREKEND);
                              }
                           } /* if ((hulp_lengte = strlen(line)) >= 3) */
                           else
                           {
                              strcpy(ff_cds, CDS_ONTBREKEND);
                              strcpy(ff_lds, FF_LDS_ONTBREKEND);
                           }
                        } /* if (fgets(line, 255, in) != NULL) */

                        /* dd */
                        if (fgets(line, 255, in) != NULL)
                        {
                           if ((hulp_lengte = strlen(line)) == 6)         /* bv "G143='\n'" (dus inclusief '\n') */
                           {
                              /* hulp_ddd in graden, moet dd in tientallen graden worden */
                              if ( (strncmp(line, "G", 1) == 0) && (strncmp(line, "G99999", 6) != 0) )             /* regel moet beginnen met "G" */
                              {
                                 pos = 1;
                                 strcpy(hulp_ddd, line + pos);                    /* de "G" niet in lijstje */
                                 hulp_ddd[hulp_lengte -1 -pos] = '\0';            /* moet een '\0' komen i.p.v. de nu aanwezige '\n' */

                                 hulp_num_dd = (int)(atof(hulp_ddd) / 10 + 0.5);
                                 sprintf(dd_cds, "%2d", hulp_num_dd);
                                 sprintf(dd_lds, "%2d", hulp_num_dd);

                              } /* if (strncmp(line, "G", 1) == 0) */
                              else
                              {
                                 strcpy(dd_cds, CDS_ONTBREKEND);
                                 strcpy(dd_lds, DD_LDS_ONTBREKEND);
                              }
                           } /* if ((hulp_lengte = strlen(line)) == 6) */
                           else
                           {
                              strcpy(dd_cds, CDS_ONTBREKEND);
                              strcpy(dd_lds, DD_LDS_ONTBREKEND);
                           }
                        } /* if (fgets(line, 255, in) != NULL) */

                        break;                                     /* uit while lus springen */

                     } /* if (strncmp(line, wind_label, 4) == 0) */
                  } /* if ((hulp_lengte = strlen(line)) == 5) */

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
                  strcpy(dd_cds,       "\0");                                 /* 10 deg */
                  strcpy(ff_cds,       "\0");                                 /* knots */
                  strcpy(dd_lds,       "\0");                                 /* 10 deg */
                  strcpy(ff_lds,       "\0");                                 /* knots */

                  strcpy(dd_cds, dd_alternatief);
                  strcpy(dd_lds, dd_alternatief);

                  strcpy(ff_cds, ff_alternatief);
                  strcpy(ff_lds, ff_alternatief);

               } /* if (strncmp(alternatieve_dd, "xx", 2) != 0) */



               /*
               /////////// S(fp) [cm2/Hz] (hoogste variantie dichtheid uit het spectrum -hoogste waarde uit de czz10[]-) en Dir fp[graden]
               */
               strcpy(S_fp,   "0");
               strcpy(Dir_fp, "0");
               Dir_fp_indice = 0;
               for (p = 0; p < AANTAL_CZZ10; p++)
               {
                  /* czz10_array staan de waarden in .cm2s dit weer terugzetten naar cm2s voor S(fp) */
                  if ( (strcmp(czz10_array[p], LDS_ONTBREKEND) != 0) && ((atoi(czz10_array[p]) /10) > atoi(S_fp)) )
                  {
                     num_hulp_waarde = atoi(czz10_array[p]) / 10;
                     sprintf(S_fp, "%d", num_hulp_waarde);
                     /*strcpy(S_fp, czz10_array[p]);*/
                     Dir_fp_indice = p;
                  }
               } /* for (p = 0; p < AANTAL_CZZ10; p++) */

               /* de richting van fp komt overeen met de czz10 gevonden max waarde (op die gevonden array plaats) */
               strcpy(Dir_fp, th010_array[Dir_fp_indice]);

               /* als er geen max is bepaalt dan alles op ontbrekend zetten */
               if (atoi(S_fp) == 0)
               {
                  strcpy(S_fp, CDS_ONTBREKEND);
                  strcpy(Dir_fp, CDS_ONTBREKEND);
               } /* if (S_fp == 0) */



               /*
               //////////////// aantal fouten (Nd_z + Nv_z + Nu_z) [AFGELEIDE WAARDE]
               // (nb kan niet testen op LDS_ONTBREKEND (=0) i.v.m. 0 fouten)
               */
               if ( (strcmp(Nd_z, "99999") != 0) && (strcmp(Nv_z, "99999") != 0) && (strcmp(Nu_z, "99999") != 0) )
               {
                  num_aantal_fouten = atoi(Nd_z) + atoi(Nv_z) + atoi(Nu_z);
                  sprintf(aantal_fouten, "%d", num_aantal_fouten);
               }
               else
                  strcpy(aantal_fouten, LDS_ONTBREKEND);


               /*
               //////////////// aantal meetwaarden  [AFGELEIDE WAARDE]
               // (nb kan niet testen op LDS_ONTBREKEND (0) i.v.m. 0 meetwaarden)
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
                  strcpy(aantal_meetwaarden, LDS_ONTBREKEND);



               /*
               //////////// Czz5 en Czz10 interpolaties [AFGELEIDE WAARDEN]
               */

               /* (Annex C punt 8) */
               if ( (strcmp(czz10_array[0], "-1") != 0) && (strcmp(czz10_array[1], "-1") != 0) &&
                    ((strcmp(czz10_array[0], LDS_ONTBREKEND) != 0) || (strcmp(czz10_array[1], LDS_ONTBREKEND) != 0)) )
               {
                  num_ghc_interpol_7_9 = (atoi(czz10_array[0]) + atoi(czz10_array[1])) / 2;
                  sprintf(ghc_interpol_7_9, "%d", num_ghc_interpol_7_9);
               } /* if ( (strcmp(czz10_array[0], "-1" != 0) && (strcmp(czz10_array[1], "-1" != 0) ) */

               /* (Annex C punt 10) */
               if ( (strcmp(czz10_array[1], "-1") != 0) && (strcmp(czz10_array[2], "-1") != 0) &&
                    ((strcmp(czz10_array[1], LDS_ONTBREKEND) != 0) || (strcmp(czz10_array[2], LDS_ONTBREKEND) != 0)) )
               {
                  num_ghc_interpol_9_11 = (atoi(czz10_array[1]) + atoi(czz10_array[2])) / 2;
                  sprintf(ghc_interpol_9_11, "%d", num_ghc_interpol_9_11);
               } /* if ( (strcmp(czz10_array[1], "-1" != 0) && (strcmp(czz10_array[2], "-1" != 0) ) */

               /* (Annex C punt 12) */
               if ( (strcmp(czz10_array[2], "-1") != 0) && (strcmp(czz5_array[0], "-1") != 0) &&
                    ((strcmp(czz10_array[2], LDS_ONTBREKEND) != 0) || (strcmp(czz5_array[0], LDS_ONTBREKEND) != 0)) )
               {
                  num_ghc_interpol_11_13 = (atoi(czz10_array[2]) + atoi(czz5_array[0])) / 2;
                  sprintf(ghc_interpol_11_13, "%d", num_ghc_interpol_11_13);
               } /* if ( (strcmp(czz10_array[2], "-1" != 0) && (strcmp(czz5_array[0], "-1" != 0) ) */


               /*
               //////////////// 2 dim spectrum voor polair plotje [AFGELEIDE WAARDE]
               // (var. namem zijn zoals Evert ze gebruikte)
               */

               /* S(f,richting) is niet rechtstreeks beschikbaar uit metingen, maar moet worden bepaald uit de */
               /* beschikbare gegevens: S(f), richting(f) en spreiding(f). Hiervoor wordt de */
               /* Cauchy-verdelingsfunctie gebruikt: */
               /* */
               /* - voor formule: ZIE DOCUMENTATIE -*/
               /* per frequentie f (0.03, 0.04, - ,0.40 Hz) wordt de variantiedichtheid S(f) verdeeld */
               /* over 36 richtingen (5, 15, -, 355 graden) volgens spreidingsparameter spreiding(f) */

               for (a = 0; a < AANTAL_CZZ10; a++)
               {
                  /* spectral values */
                  SP[a] = atof(czz10_array[a]) * 1.E-5;             /* SP nu in M**2 */
                  if (SP[a] < 0.0)
                     SP[a] = 0.0;

                  /* wave directions (degrees) */
                  TH[a] = atof(th010_array[a]);
                  SPR[a] = atof(S0bh10_array[a]);
               } /* for (a = 0; a < AANTAL_CZZ10; a++) */


               /* NB de waarden bij 0.00, 0.01 en 0.02 Hz hebben we voor polaire plotjes niet nodig */

               for (a = 3; a < AANTAL_CZZ10; a++)          /* beginnen bij 0.03 Hz (index no. 3) */
               {
                  /* plot only when dir. spread > 0 */
                  if (SPR[a] > 0.0)
                  {
                     X = 1.0 - 0.5 * (SPR[a] / DEGRAD) * (SPR[a] / DEGRAD);  /* X = 1.0 - 0.5 * (SPR[a] / DEGRAD)**2;*/
                     SMDSTR = 0.0;
                     for (b = 1; b <= 36; b++)
                     {
                        COSTH0 = cos((10 * b - 5 - TH[a]) / DEGRAD);
                        DSTR[b - 1] = (1 - X * X) / (TWOPI * (1.0 - (2.0 * X * COSTH0) + X * X));
                        SMDSTR = SMDSTR + DSTR[b - 1];
                     } /* for (b = 1; b <+ 36; b++) */

                     AMPL = NDVTWP * SP[a] / SMDSTR;

                     for (b = 0; b < 36; b++)
                        SPTH[a][b] = AMPL * DSTR[b];

                  } /* if (SPR[a] > 0) */
               } /* for (a = 3; a < AANTAL_CZZ10; a++) */

               for (a = 3; a < AANTAL_CZZ10; a++)
                  FR[a] = a * 0.01;



               /* sluiten input file */
               fclose(in);


               /*
               ////////////////// SUSPECT of OK ?
               */
               strcpy(flag, OK_FLAG);                                    /* default is OK */
               Bepaal_Suspect_Station(sensorcode, flag);                 /* kan hier SUSPECT worden */


            } /* if ((in = fopen(dir_en_input_filenaam, "r")) != NULL) */



            /*********************************************************************/
            /*                                                                   */
            /*           schrijven naar DSP output file (was al geopend)         */
            /*                                                                   */
            /*********************************************************************/

            /* moet wel een aantal meetwaarden hebben */
            if ((strncmp(flag, OK_FLAG, 7) != 0) || (atoi(aantal_meetwaarden) <= 768) ||         /* minimaal helft (max 6 * 256) mogelijke meetwaarden moet er zijn */
                (atoi(HM0_lds) >= 20000) || (atoi(HE1) >= 20000) || (atoi(HE2) >= 20000) || (atoi(HE3) >= 20000) ) /* 20000 mm */
               dsp_schrijven = 0;
            else /* > 0 aantal meetwaarden) */
            {
               /* alleen bij ergens een reele-waarde-combinatie gaan schrijven */
               dsp_schrijven = 0;
               for (b = 3; b < AANTAL_CZZ10; b++)
               {
                  if (SP[b] != 0.0 && TH[b] != 0.0 && SPR[b] != 0.0)
                  {
                     dsp_schrijven = 1;
                     break;
                  } /* if (SP[b] != 0.0 && TH[b] != 0.0 && SPR[b] != 0.0) */
               } /* for (b = 3; b < AANTAL_CZZ10; b++) */
            } /* else (>0 aantal_meetwaarden) */

            if (dsp_schrijven == 1)
            {
               /* sensorcode zonder "NZ" ervoor */
               fprintf(out_dsp, "%4.4s", sensorcode);

               fprintf(out_dsp, " ");                                            /* spatie */

               /* datum (zonder eeuw aanduiding) */
               strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
               pos = 2;
               strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
               jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
               fprintf(out_dsp, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

               fprintf(out_dsp, "\n");                                           /* nieuwe regel */

               for (b = 3; b < AANTAL_CZZ10; b++)
               {
                  fprintf(out_dsp, "%10.4f", FR[b]);                             /* frequentie aanduiding (0.03, 0.04 etc.) */
                  for (a = 0; a < 36; a++)
                     fprintf(out_dsp, "%10.4f", SPTH[b][a]);

                  fprintf(out_dsp, "\n");                                        /* nieuwe regel */
               } /* for (b = 3; b < AANTAL_CZZ10; b++) */

            } /* if (dsp_schrijven == true) */




            /*********************************************************************/
            /*                                                                   */
            /*           schrijven naar DSP2 output file (was al geopend)        */
            /*                                                                   */
            /*********************************************************************/

            /* moet wel een aantal meetwaarden hebben */
            if ((strncmp(flag, OK_FLAG, 7) != 0) || (atoi(aantal_meetwaarden) <= 768) ||         /* minimaal helft (max 6 * 256) mogelijke meetwaarden moet er zijn */
                (atoi(HM0_lds) >= 20000) || (atoi(HE1) >= 20000) || (atoi(HE2) >= 20000) || (atoi(HE3) >= 20000) ) /* 20000 mm */
               dsp_schrijven = 0;
            else /* > 0 aantal meetwaarden) */
            {
               /* alleen bij ergens een reele-waarde-combinatie gaan schrijven */
               dsp_schrijven = 0;
               for (b = 3; b < AANTAL_CZZ10; b++)
               {
                  if (SP[b] != 0.0 && TH[b] != 0.0 && SPR[b] != 0.0)
                  {
                     dsp_schrijven = 1;
                     break;
                  } /* if (SP[b] != 0.0 && TH[b] != 0.0 && SPR[b] != 0.0) */
               } /* for (b = 3; b < AANTAL_CZZ10; b++) */
            } /* else (>0 aantal_meetwaarden) */

            if (dsp_schrijven == 1)
            {
               /* datum (zonder eeuw aanduiding) */
               strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
               pos = 2;
               strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
               jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                 /* nu JJJJMMDDUU maar zonder eeuw */

               for (b = 3; b < AANTAL_CZZ10; b++)
               {
                  /* sensor naam + datum tijd */
                  fprintf(out_dsp2, "%4.4s", sensorcode);
                  fprintf(out_dsp2, " ");                                         /* spatie */
                  fprintf(out_dsp2, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* frequenty */
                  fprintf(out_dsp2, "%10.4f", FR[b]);                             /* frequentie aanduiding (0.03, 0.04 etc.) */

                  /* energy bij die freq. */
                  for (a = 0; a < 36; a++)
                     fprintf(out_dsp2, "%10.4f", SPTH[b][a]);

                  fprintf(out_dsp2, "\n");                                        /* nieuwe regel */
               } /* for (b = 3; b < AANTAL_CZZ10; b++) */

            } /* if (dsp_schrijven == true) */





            /*********************************************************************/
            /*                                                                   */
            /*           schrijven naar CDS output file (was al geopend)         */
            /*                                                                   */
            /*********************************************************************/

            /* alleen als er een HM0 instaat (indien geen HM0 dan nooit iets bijzonders verder per definitie) */
            /*        7654321: (ONTBREKEND) wordt door dit programma er ingezet als er geen waarde voor HM0 staat */
            /*          99999: staat soms in de input files als er (tijdelijk ?) geen data voor HM0 is */
            if ( (atoi(aantal_meetwaarden) > 768) &&
                 (strcmp(HM0_cds, CDS_ONTBREKEND) != 0) && (strcmp(HM0_cds, "99999") != 0) &&
                 (atoi(HM0_cds) < 2000) && (atoi(HE1) < 20000) && (atoi(HE2) < 20000) && (atoi(HE3) < 20000))  /* HMO_cds 2000 cm en HE1 etc 20000 mm */
            {
               /* checks (via Evert) HM0 > 0   (dan alle waarden uit de waarnemingen ongeldig) */
               /* NB 12-07-2005 afgesproken niet meer op Tm_10 te testen voor al of niet wegschrijven */
               if ( (atoi(HM0_cds) > 0) /*&& (atoi(Tm_10_cds) < 999)*/ )
               {
                  /* spatie (overblijfsel uit Fortran tijdperk) */
                  fprintf(out_cds, "%1.1s", " ");

                  /* datum (zonder eeuw aanduiding) */
                  strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
                  pos = 2;
                  strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
                  jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
                  fprintf(out_cds, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* sensorcode met "NZ" ervoor */
                  fprintf(out_cds, "%3.3s", "NZ");
                  fprintf(out_cds, "%4.4s", sensorcode);

                  /* Latitude, Longitude and Quadrant */
                  Bepaal_Lat_Lon_Q(sensorcode, latitude, longitude, quadrant);
                  fprintf(out_cds, "%2.2s", quadrant);
                  fprintf(out_cds, "%8.8s", longitude);
                  fprintf(out_cds, "%8.8s", latitude);

                  /* dd */
                  fprintf(out_cds, "%8.8s", dd_cds);

                  /* ff */
                  fprintf(out_cds, "%8.8s", ff_cds);

                  /* Tm-10 */
                  fprintf(out_cds, "%8.8s", Tm_10_cds);

                  /* Hm0 */
                  fprintf(out_cds, "%8.8s", HM0_cds);

                  /* Th0 */
                  fprintf(out_cds, "%8.8s", Th0_cds);

                  /* fp */
                  fprintf(out_cds, "%8.8s", Fp);

                  /* S_fp */
                  fprintf(out_cds, "%8.8s", S_fp);

                  /* Dir_fp */
                  fprintf(out_cds, "%8.8s", Dir_fp);

                  /* flag */
                  /*strcpy(flag, OK_FLAG);                    */                /* default */
                  /*Bepaal_Suspect_Station(sensorcode, flag); */                /* kan hier SUSPECT worden */
                  fprintf(out_cds, "%8.8s", flag);

                  /* nieuwe regel */
                  fprintf(out_cds, "\n");

               } /* if (atoi(HM0_cds) > 0) */
            } /* if ( (aantal_meetwaarden > 693) etc*/





            /*********************************************************************/
            /*                                                                   */
            /*       schrijven naar LDS en LDS2 output file (was al geopend)     */
            /*            (NB pos: zoals bij comment  gerekend vanaf 1)          */
            /*********************************************************************/

            if ( (strncmp(flag, OK_FLAG, 7) == 0) && (atoi(aantal_meetwaarden) >= 768) &&
                 (strcmp(HM0_lds, LDS_ONTBREKEND) != 0) && (strcmp(HM0_lds, "99999") != 0) &&
                 (atoi(HM0_lds) < 20000) && (atoi(HE1) < 20000) && (atoi(HE2) < 20000) && (atoi(HE3) < 20000) )   /* 20000 mm */
            {

               /* checks (via Evert) HM0    > 0   (dan alle waarden uit de waarnemingen geldig) */
               /*                    Tm_10 < 999 (dan alle waarden uit de waarnemingen geldig) */
               /* NB 12-07-2005 afgesproken niet meer op Tm_10 te testen voor al of niet wegschrijven */
               /* NB 7-12-2006 bleek toch waarschijnlijk door 0 in lds file nrdwam crash, daarom nu Czz5_ok en Czz10_ok*/

               /*if ( (atoi(HM0_lds) > 0) && (atoi(Tm_10_lds) < 999)) */
               /*if ( (atoi(HM0_lds) > 0) && (czz5_array[0] != LDS_ONTBREKEND) && (czz10_array[0] != LDS_ONTBREKEND) )*/ /* czz5 en czz10 extra chexks 07-11-2005 ingvoerd */
               if ( (atoi(HM0_lds) > 0) && (Czz5_ok == 1) && (Czz10_ok == 1) )  /* czz5_ok en czz10_ok checks 07-12-2006 ingevoerd */
               {
                  /* sensorcodes (bv K133) die echt in de lds file komen opslaan, die er niet instaan kunnen aangevuld worden met de sensorcode data uit de lfr file (deze hebben geen richtingsdata) */
                  strcpy(lds_sensorcode_array[lds_sensorcode_no], sensorcode);
                  lds_sensorcode_array[lds_sensorcode_no][4] = '\0';
                  lds_sensorcode_no++;

                  /* sensorcode (zonder "NZ" ervoor) [pos: 1] */
                  fprintf(out_lds,  "%4.4s", sensorcode);
                  fprintf(out_lds2, "%4.4s", sensorcode);

                  /* spatie [pos: 5] */
                  fprintf(out_lds,  "%1.1s", " ");
                  fprintf(out_lds2, "%1.1s", " ");

                  /* datum (zonder eeuw aanduiding) [pos: 6] */
                  strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
                  pos = 2;
                  strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
                  jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
                  fprintf(out_lds,  "%8.8s", jaar_dag_uur_JJJJMMDDUU);
                  fprintf(out_lds2, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* dd [pos: 14] */
                  fprintf(out_lds,  "%3.3s", dd_lds);
                  fprintf(out_lds2, "%3.3s", dd_lds);

                  /* ff [pos: 17] */
                  fprintf(out_lds,  "%2.2s", ff_lds);
                  fprintf(out_lds2, "%2.2s", ff_lds);

                  /* kwaliteitscode (overblijfsel van vroeger (1970) is altijd 1) [pos: 19] */
                  fprintf(out_lds,  "%2.2s", "1");
                  fprintf(out_lds2, "%2.2s", "1");

                  /* 0 (annex C punt f) [pos: 21] */
                  fprintf(out_lds,  "%2.2s", "0");
                  fprintf(out_lds2, "%2.2s", "0");

                  /* 0 (annex C punt g) [pos: 23] */
                  fprintf(out_lds,  "%1.1s", "0");
                  fprintf(out_lds2, "%1.1s", "0");

                  /* 0 (annex C punt h) [pos: 24] */
                  fprintf(out_lds,  "%1.1s", "0");
                  fprintf(out_lds2, "%1.1s", "0");

                  /* 0 (annex C punt i) [pos: 25] */
                  fprintf(out_lds,  "%1.1s", "0");
                  fprintf(out_lds2, "%1.1s", "0");

                  /* spatie (annex C punt i) [pos: 26] */
                  fprintf(out_lds,  "%1.1s", " ");
                  fprintf(out_lds2, "%1.1s", " ");

                  /* -1 (annex C punt 1) [pos: 27] */
                  fprintf(out_lds,  "%4.4s", "-1");
                  fprintf(out_lds2, "%4.4s", "-1");

                  /* -1 (annex C punt 2) [pos: 31] */
                  fprintf(out_lds,  "%4.4s", "-1");
                  fprintf(out_lds2, "%4.4s", "-1");

                  /* aantal fouten (afgeleide waarde kan nooit 99999 zijn) (annex C punt 3) [pos: 35] */
                  fprintf(out_lds,  "%4.4s", aantal_fouten);
                  fprintf(out_lds2, "%4.4s", aantal_fouten);

                  /* -1 (annex C punt 4) [pos: 39] */
                  fprintf(out_lds,  "%4.4s", "-1");
                  fprintf(out_lds2, "%4.4s", "-1");

                  /* aantal meetwaarden (afgeleide waarde kan nooit 99999 zijn ) (annex C punt 5) [pos: 43] */
                  fprintf(out_lds,  "%4.4s", aantal_meetwaarden);
                  fprintf(out_lds2, "%4.4s", aantal_meetwaarden);

                  /* -1 (annex C punt 6) [pos: 47] */
                  fprintf(out_lds,  "%4.4s", "-1");
                  fprintf(out_lds2, "%4.4s", "-1");

                  /* spectrum 5 mHz (0.0 Hz) (annex C punt 7) [pos: 51] */
                  fprintf(out_lds,  "%9.9s", czz10_array[0]);
                  fprintf(out_lds2, "%9.9s", czz10_array[0]);

                  /* spectrum interpolatie van GHC el. 7 en 9 (annex C punt 8) [pos: 60] */
                  fprintf(out_lds,  "%9.9s", ghc_interpol_7_9);
                  fprintf(out_lds2, "%9.9s", ghc_interpol_7_9);

                  /* spectrum 5 mHz (0.01 Hz) (annex C punt 9) [pos: 69] */
                  fprintf(out_lds,  "%9.9s", czz10_array[1]);
                  fprintf(out_lds2, "%9.9s", czz10_array[1]);

                  /* spectrum interpolatie van GHC el. 9 en 11 (annex Ax punt 10) [pos: 78] */
                  fprintf(out_lds,  "%9.9s", ghc_interpol_9_11);
                  fprintf(out_lds2, "%9.9s", ghc_interpol_9_11);

                  /* spectrum 5 mHz (0.02 Hz) (annex C punt 11) [pos: 87] */
                  fprintf(out_lds,  "%9.9s", czz10_array[2]);
                  fprintf(out_lds2, "%9.9s", czz10_array[2]);

                  /* spectrum interpolatie van GHC el. 11 en 13 (annex C punt 12) [pos: 96] */
                  fprintf(out_lds,  "%9.9s", ghc_interpol_11_13);
                  fprintf(out_lds2, "%9.9s", ghc_interpol_11_13);

                  /* spectrum 5 Mhz (0.03 Hz - 0.15 Hz) (Annex C punt 13 - 37) [pos: 105] */
                  for (m = 0; m < AANTAL_CZZ5; m++)
                  {
                     fprintf(out_lds,  "%9.9s", czz5_array[m]);
                     fprintf(out_lds2, "%9.9s", czz5_array[m]);
                  }

                  /* vrijh.graden sp. komp. (Annex C punt 38) [pos: 330] */
                  fprintf(out_lds,  "%3.3s", AV10_H);
                  fprintf(out_lds2, "%3.3s", AV10_H);

                  /* spectrum 10 Mhz (0.15 Hz - 0.5 Hz) (Annex C punt 39 - 74) [pos: 333] */
                  for (m = 15; m < AANTAL_CZZ10; m++)
                  {
                     fprintf(out_lds,  "%7.7s", czz10_array[m]);
                     fprintf(out_lds2, "%7.7s", czz10_array[m]);
                  }

                  /* HM0 (Annex C punt 75) [pos: 585] */
                  fprintf(out_lds,  "%5.5s", HM0_lds);
                  fprintf(out_lds2, "%5.5s", HM0_lds);

                  /* HE0 (Annex C punt 76) [pos: 590] */
                  fprintf(out_lds,  "%5.5s", HE0);
                  fprintf(out_lds2, "%5.5s", HE0);

                  /* HE1 (Annex C punt 77) [pos: 595] */
                  fprintf(out_lds,  "%5.5s", HE1);
                  fprintf(out_lds2, "%5.5s", HE1);

                  /* HE2 (Annex C punt 78) [pos: 600] */
                  fprintf(out_lds,  "%5.5s", HE2);
                  fprintf(out_lds2, "%5.5s", HE2);

                  /* HE3 (Annex C punt 79) [pos: 605] */
                  fprintf(out_lds,  "%5.5s", HE3);
                  fprintf(out_lds2, "%5.5s", HE3);

                  /* vrijheids graden HM0 (standaard op 0, staat niet in sovf80) (Annex C punt 80) [pos: 610] */
                  fprintf(out_lds,  "%4.4s", "0");
                  fprintf(out_lds2, "%4.4s", "0");

                  /* -1 (annex C punt 81) [pos: 614] */
                  fprintf(out_lds,  "%4.4s", "-1");
                  fprintf(out_lds2, "%4.4s", "-1");

                  /* Tm-10 (Tm0-1) (annex C punt 82) [pos: 618] */
                  fprintf(out_lds,  "%4.4s", Tm_10_lds);
                  fprintf(out_lds2, "%4.4s", Tm_10_lds);

                  /* fp (piekfrequentie) (annex C punt 83) [pos: 622] */
                  fprintf(out_lds,  "%4.4s", Fp);
                  fprintf(out_lds2, "%4.4s", Fp);

                  /* Hs7 (mm) (annex C punt 84) [pos: 626] */
                  fprintf(out_lds,  "%5.5s", Hs7_lds);
                  fprintf(out_lds2, "%5.5s", Hs7_lds);

                  /* Tm02 (annex C punt 85) [pos: 631] */
                  fprintf(out_lds,  "%3.3s", Tm02);
                  fprintf(out_lds2, "%3.3s", Tm02);

                  /* -1 (annex C punt 86) [pos: 634] */
                  fprintf(out_lds,  "%3.3s", "-1");
                  fprintf(out_lds2, "%3.3s", "-1");

                  /* aantal golven (AG) (annex C punt 87) [pos: 637] */
                  fprintf(out_lds,  "%3.3s", aantal_golven);
                  fprintf(out_lds2, "%3.3s", aantal_golven);

                  /* spectrum 10 Mhz (0.0 Hz - 0.14 Hz) (Annex C punt 88 - 102) [pos: 640] */
                  for (m = 0; m < 15; m++)
                  {
                     fprintf(out_lds,  "%9.9s", czz10_array[m]);
                     fprintf(out_lds2, "%9.9s", czz10_array[m]);
                  }

                  /* nieuwe regel */
                  fprintf(out_lds, "\n");
                  /* BIJ LDS2 GEEN NIEUWE REGEL !!! */

                  /* Th0 (gem. richting totale spectrum) (annex C punt 103) [pos: 1] */
                  fprintf(out_lds,  "%4.4s", Th0_lds);
                  fprintf(out_lds2, "%4.4s", Th0_lds);

                  /* S0bh (gem. spreiding totale spectrum) (annex C punt 104) [pos: 5] */
                  fprintf(out_lds,  "%4.4s", S0bh);
                  fprintf(out_lds2, "%4.4s", S0bh);

                  /* Th3 (gem. richting LFE) (annex C punt 105) [pos: 9] */
                  fprintf(out_lds,  "%4.4s", Th3);
                  fprintf(out_lds2, "%4.4s", Th3);

                  /* AV10_R (aantal vrijheidsgraden sp. el. (annex C punt 106) [pos: 13] */
                  fprintf(out_lds,  "%3.3s", AV10_R);
                  fprintf(out_lds2, "%3.3s", AV10_R);

                  /* DL_index (golfgetal) (annex C punt 107) [pos: 16] */
                  fprintf(out_lds,  "%5.5s", DL_index);
                  fprintf(out_lds2, "%5.5s", DL_index);

                  /* Th010 (gem. richting 10 mHz [0.0 Hz - 0.50 Hz]) (Annex C punt 108 - 158) [pos: 21] */
                  for (m = 0; m < AANTAL_TH010; m++)
                  {
                     fprintf(out_lds,  "%4.4s", th010_array[m]);
                     fprintf(out_lds2, "%4.4s", th010_array[m]);
                  }

                  /* S0bh10 (gem. spreiding 10 mHz [0.0 Hz - 0.50 Hz]) (Annex C punt 159 - 209) [pos: 225] */
                  for (m = 0; m < AANTAL_S0BH10; m++)
                  {
                     fprintf(out_lds,  "%4.4s", S0bh10_array[m]);
                     fprintf(out_lds2, "%4.4s", S0bh10_array[m]);
                  }

                  /* nieuwe regel */
                  fprintf(out_lds,  "\n");
                  fprintf(out_lds2, "\n");

               } /* if ( (atoi(HM0_lds) > 0) && (czz5_array[0] != LDS_ONTBREKEND) && (czz10_array[0] != LDS_ONTBREKEND) ) */
            } /* if ( (atoi(aantal_meetwaarden) > 693) etc. */

         }  /* if (strcmp(input_files[i], "\0") != 0) */
      }  /* for (i = 0; i < AANTAL_INPUT_FILES; i++) */


      /*********************************************************************/
      /*                                                                   */
      /*                           LDS aanvullen                           */
      /*        aanvullen met lfr data (deze zijn zonder richtingen)       */
      /*             NB LDS2 wordt NIET aangevuld                          */
      /*********************************************************************/

      /* dir en naam .lfr output file bepalen */
      Bepaal_Dir_en_LFR_filenaam(dir_en_lfr_filenaam);

      /* initialisatie */
      strcpy(lfr_line, "\0");

      /* openen lfr file (nb alleen de niet suspect stations staan in de lfr file) */
      if ((lfr_in = fopen(dir_en_lfr_filenaam, "r")) != NULL)
      {
         while (fgets(lfr_line, 1023, lfr_in) != NULL)
         {
            if (strlen(lfr_line) >= 845)                                       /* voor de zekerheid */
            {
               /*
               //////////////// sensorcode
               */
               pos = 0;
               strncpy(lfr_sensorcode, lfr_line + pos, 4);                      /* bv K131 */
               lfr_sensorcode[4] = '\0';


               /* per locatie maar de data van 1 lfr sensor beschikbaar stellen */
               aanvullen = 1;
               for (z = 0; z < AANTAL_SENSORCODES; z++)
               {
                  /* Alleen de eerste 2 char vergelijken (bv "K1" locatie van de sensoren K132, K131) nb eerste 2 char i.vm. NC1 en NC2  */
                  if (strncmp(lfr_sensorcode, lfr_sensorcode_array[z], 2) == 0)
                  {
                     aanvullen = 0;                                 /* deze is dan ook al getst tegen lds sensor code */
                     break;                                         /* van die locatie is dus al een sensor aanwezig */
                  }
               } /* for (z = 0; z < AANTAL_SENSORCODES; z++) */

               if (aanvullen)
               {
                  /* sensorcodes (bv MUN1) die echt in de lfr file komen opslaan omdat maar van 1 locatie de data uit lfr genomen moet worden om lds aan te vullen */
                  strcpy(lfr_sensorcode_array[lfr_sensorcode_no], lfr_sensorcode);
                  lfr_sensorcode_array[lfr_sensorcode_no][4] = '\0';
                  lfr_sensorcode_no++;
               } /* if (aanvullen) */


               /*aanvullen = 1;*/

               /* beschikbaar is dus lfr sensordata van een locatie, nu checken of deze data (wat lds betreft) echt nodig is */
               if (aanvullen)
               {
                  for (z = 0; z < AANTAL_SENSORCODES; z++)
                  {

                     /* ////// TEST ///////////
                     fprintf(stderr, "%s", lds_sensorcode_array[z]);
                     getchar();
                     ///// TEST ///////////*/

                     /* Alleen de eerste 2 char vergelijken (bv "K1" locatie van de sensoren K132, K131)nb eerste 2 char i.vm. NC1 en NC2 */
                     if (strncmp(lfr_sensorcode, lds_sensorcode_array[z], 2) == 0) /* lfr_sensorcode en lds_sensorcode altijd 4 char */
                     {
                        aanvullen = 0;
                        break;                                         /* van die locatie is dus alle data (uit lds) al aanwezig */
                     }
                  } /* for (z = 0; z < AANTAL_SENSORCODES; z++) */
               } /* if (aanvullen) */

               if (aanvullen)
               {
                  /*
                  ////////////////// initialisatie
                  */

                  /* initialisatie lds */
                  strcpy(sensorcode, "\0");

                  /* NB initialisatie met "0" voor lds komt voort uit de wijze hoe Evert dit deed (ook 0 in lds indien niet aanwezig)
                  /*    bij dd en ff moet indien ontbrekend dit 99 (dd) en 99 (ff) worden ! */

                  for (k = 0; k < AANTAL_CZZ5; k++)
                     strcpy(czz5_array[k], LDS_ONTBREKEND);

                  for (k = 0; k < AANTAL_CZZ10; k++)
                     strcpy(czz10_array[k], LDS_ONTBREKEND);

                  for (k = 0; k < AANTAL_TH010; k++)
                     strcpy(th010_array[k], LDS_TH010_ONTBREKEND);

                  for (k = 0; k < AANTAL_S0BH10; k++)
                     strcpy(S0bh10_array[k], LDS_S0BH10_ONTBREKEND);


                  strcpy(aantal_fouten,      LDS_ONTBREKEND);
                  strcpy(aantal_meetwaarden, LDS_ONTBREKEND);
                  strcpy(AV10_H,             LDS_ONTBREKEND);
                  strcpy(TE0,                LDS_ONTBREKEND);
                  strcpy(Fp,                 LDS_ONTBREKEND);
                  strcpy(Tm02,               LDS_ONTBREKEND);
                  strcpy(aantal_golven,      LDS_ONTBREKEND);
                  strcpy(H1d3,               LDS_ONTBREKEND);
                  strcpy(H1d10,              LDS_ONTBREKEND);
                  strcpy(GGh,                LDS_ONTBREKEND);
                  strcpy(Hmax,               LDS_ONTBREKEND);
                  strcpy(T1d3,               LDS_ONTBREKEND);
                  strcpy(TH1d3,              LDS_ONTBREKEND);
                  strcpy(GGT,                LDS_ONTBREKEND);
                  strcpy(Tmax,               LDS_ONTBREKEND);
                  strcpy(THmax,              LDS_ONTBREKEND);
                  strcpy(Ndir_H,             LDS_ONTBREKEND);
                  strcpy(Ngd_zP,             LDS_ONTBREKEND);
                  strcpy(Nd_z,               LDS_ONTBREKEND);
                  strcpy(Nu_z,               LDS_ONTBREKEND);
                  strcpy(Nv_z,               LDS_ONTBREKEND);
                  strcpy(Ni_z,               LDS_ONTBREKEND);
                  strcpy(HM0_lds,            LDS_ONTBREKEND);
                  strcpy(dd_lds,             LDS_ONTBREKEND);                       /* 10 deg */
                  strcpy(ff_lds,             LDS_ONTBREKEND);                       /* knots */
                  strcpy(TE0,                "\0");
                  strcpy(TE1,                "\0");                                 /* cm**2 */
                  strcpy(TE2,                "\0");                                 /* cm**2 */
                  strcpy(TE3,                "\0");                                 /* cm**2 */
                  strcpy(HE0,                LDS_ONTBREKEND);
                  strcpy(HE1,                LDS_ONTBREKEND);
                  strcpy(HE2,                LDS_ONTBREKEND);
                  strcpy(HE3,                LDS_ONTBREKEND);
                  strcpy(Tm_10_lds,          LDS_ONTBREKEND);                       /* 0.1*s */   /* Ts, TM0-1 */
                  strcpy(Hs7_lds,            LDS_ONTBREKEND);                       /* mm */
                  strcpy(H1d50,              LDS_ONTBREKEND);
                  strcpy(ghc_interpol_7_9,   LDS_ONTBREKEND);
                  strcpy(ghc_interpol_9_11,  LDS_ONTBREKEND);
                  strcpy(ghc_interpol_11_13, LDS_ONTBREKEND);
                  strcpy(Th0_lds,            LDS_ONTBREKEND);                       /* graden */
                  strcpy(S0bh,               LDS_ONTBREKEND);
                  strcpy(Th3,                LDS_ONTBREKEND);
                  strcpy(AV10_R,             LDS_ONTBREKEND);
                  strcpy(DL_index,           LDS_ONTBREKEND);


                  /*
                  //////////////// gewenste lfr data in lds variabelen zetten
                  */

                  /* sensor code */
                  strncpy(sensorcode, lfr_sensorcode, 4);

                  /* dd (uit lfr) */
                  pos = 14;
                  strncpy(dd_lds, lfr_line + pos, 2);

                  /* ff (uit lfr) */
                  pos = 16;
                  strncpy(ff_lds, lfr_line + pos, 2);

                  /* aantal fouten (uit lfr)*/
                  pos = 28;
                  strncpy(aantal_fouten, lfr_line + pos, 4);

                  /* aantal meetwaarden (uit lfr alternatief) */
                  pos = 36;
                  strncpy(aantal_meetwaarden, lfr_line + pos, 4);

                  /* spectrum 5 mHz (0.0 Hz) */
                  pos = 44;
                  strncpy(czz10_array[0], lfr_line + pos, 9);

                  /* spectrum interpolatie van GHC el. 7 en 9 */
                  pos = 53;
                  strncpy(ghc_interpol_7_9, lfr_line + pos, 9);

                  /* spectrum 5 mHz (0.01 Hz) */
                  pos = 62;
                  strncpy(czz10_array[1], lfr_line + pos, 9);

                  /* spectrum interpolatie van GHC el. 9 en 11 */
                  pos = 71;
                  strncpy(ghc_interpol_9_11, lfr_line + pos, 9);

                  /* spectrum 5 mHz (0.02 Hz) */
                  pos = 80;
                  strncpy(czz10_array[2], lfr_line + pos, 9);

                  /* spectrum interpolatie van GHC el. 11 en 13 */
                  pos = 89;
                  strncpy(ghc_interpol_11_13, lfr_line + pos, 9);

                  /* spectrum 5 mHz (0.03 Hz - 0.15 Hz) */
                  for (m = 0; m < AANTAL_CZZ5; m++)
                  {
                     pos += 9;                                              /* pos 98 (+ 24 * 9 = 216) - 314 */
                     strncpy(czz5_array[m], lfr_line + pos, 9);
                  }

                  /* vrijh. graden sp. komp. */
                  pos = 323;                                                /*  */
                  strncpy(AV10_H, lfr_line + pos, 3);

                  /* spectrum 10 mHz (0.15 Hz) */
                  pos = 326;                                                /*  */
                  strncpy(czz10_array[15], lfr_line + pos, 7);

                  /* spectrum 10 mHz (0.25 Hz - 0.5 Hz) */
                  for (m = 16; m < AANTAL_CZZ10; m++)
                  {
                     pos += 7;                                              /* pos 333 (+ 34 * 7 = 238) - 571 */
                     strncpy(czz10_array[m], lfr_line + pos, 7);
                  }

                  /* HM0 */
                  pos = 578;
                  strncpy(HM0_lds, lfr_line + pos, 5);

                  /* HE0 */
                  pos = 583;
                  strncpy(HE0, lfr_line + pos, 5);

                  /* HE1 */
                  pos = 588;
                  strncpy(HE1, lfr_line + pos, 5);

                  /* HE2 */
                  pos = 593;
                  strncpy(HE2, lfr_line + pos, 5);

                  /* HE3 */
                  pos = 598;
                  strncpy(HE3, lfr_line + pos, 5);

                  /* Tm-10 */
                  pos = 611;
                  strncpy(Tm_10_lds, lfr_line + pos, 4);

                  /* fp */
                  pos = 615;
                  strncpy(Fp, lfr_line + pos, 4);

                  /* Hs7 */
                  pos = 619;
                  strncpy(Hs7_lds, lfr_line + pos, 5);

                  /* Tm02 */
                  pos = 624;
                  strncpy(Tm02, lfr_line + pos, 3);

                  /* aantal golven */
                  pos = 630;
                  strncpy(aantal_golven, lfr_line + pos, 3);

                  /* spectrum 10 mHz (0.0 Hz) */
                  pos = 633;
                  strncpy(czz10_array[0], lfr_line + pos, 9);

                  /* spectrum 10 mHz (0.01 Hz - 0.14 Hz) */
                  for (m = 1; m < 15; m++)
                  {
                     pos += 9;
                     strncpy(czz10_array[m], lfr_line + pos, 9);
                  }



                  /*
                  //////////////// naar lds file schrijven (stond nog open)
                  */

                  /* sensorcode (zonder "NZ" ervoor) */
                  fprintf(out_lds, "%4.4s", sensorcode);

                  /* spatie */
                  fprintf(out_lds, "%1.1s", " ");

                  /* datum (zonder eeuw aanduiding) */
                  strcpy(jaar_dag_uur_JJJJMMDDUU, "\0");
                  pos = 2;
                  strncpy(jaar_dag_uur_JJJJMMDDUU, JJJJMMDDUU + pos, 8);
                  jaar_dag_uur_JJJJMMDDUU[8] = '\0';                                /* nu JJJJMMDDUU maar zonder eeuw */
                  fprintf(out_lds, "%8.8s", jaar_dag_uur_JJJJMMDDUU);

                  /* dd */
                  fprintf(out_lds, "%3.3s", dd_lds);

                  /* ff */
                  fprintf(out_lds, "%2.2s", ff_lds);

                  /* kwaliteitscode (overblijfsel van vroeger (1970) is altijd 1) */
                  fprintf(out_lds, "%2.2s", "1");

                  /* 0 (annex C punt f) */
                  fprintf(out_lds, "%2.2s", "0");

                  /* 0 (annex C punt g) */
                  fprintf(out_lds, "%1.1s", "0");

                  /* 0 (annex C punt h) */
                  fprintf(out_lds, "%1.1s", "0");

                  /* 0 (annex C punt i) */
                  fprintf(out_lds, "%1.1s", "0");

                  /* spatie (annex C punt i) */
                  fprintf(out_lds, "%1.1s", " ");

                  /* -1 (annex C punt 1) */
                  fprintf(out_lds, "%4.4s", "-1");

                  /* -1 (annex C punt 2) */
                  fprintf(out_lds, "%4.4s", "-1");

                  /* aantal fouten (afgeleide waarde kan nooit 99999 zijn) (annex C punt 3) */
                  fprintf(out_lds, "%4.4s", aantal_fouten);

                  /* -1 (annex C punt 4) */
                  fprintf(out_lds, "%4.4s", "-1");

                  /* aantal meetwaarden (afgeleide waarde kan nooit 99999 zijn ) (annex C punt 5) */
                  fprintf(out_lds, "%4.4s", aantal_meetwaarden);

                  /* -1 (annex C punt 6) */
                  fprintf(out_lds, "%4.4s", "-1");

                  /* spectrum 5 mHz (0.0 Hz) (annex C punt 7) */
                  fprintf(out_lds, "%9.9s", czz10_array[0]);

                  /* spectrum interpolatie van GHC el. 7 en 9 (annex C punt 8) */
                  fprintf(out_lds, "%9.9s", ghc_interpol_7_9);

                  /* spectrum 5 mHz (0.01 Hz) (annex C punt 9) */
                  fprintf(out_lds, "%9.9s", czz10_array[1]);

                  /* spectrum interpolatie van GHC el. 9 en 11 (annex Ax punt 10) */
                  fprintf(out_lds, "%9.9s", ghc_interpol_9_11);

                  /* spectrum 5 mHz (0.02 Hz) (annex C punt 11) */
                  fprintf(out_lds, "%9.9s", czz10_array[2]);

                  /* spectrum interpolatie van GHC el. 11 en 13 (annex C punt 12) */
                  fprintf(out_lds, "%9.9s", ghc_interpol_11_13);

                  /* spectrum 5 Mhz (0.03 Hz - 0.15 Hz) (Annex C punt 13 - 37) */
                  for (m = 0; m < AANTAL_CZZ5; m++)
                     fprintf(out_lds, "%9.9s", czz5_array[m]);

                  /* vrijh.graden sp. komp. (Annex C punt 38) */
                  fprintf(out_lds, "%3.3s", AV10_H);

                  /* spectrum 10 Mhz (0.15 Hz - 0.5 Hz) (Annex C punt 39 - 74) */
                  for (m = 15; m < AANTAL_CZZ10; m++)
                     fprintf(out_lds, "%7.7s", czz10_array[m]);

                  /* HM0 (Annex C punt 75) */
                  fprintf(out_lds, "%5.5s", HM0_lds);

                  /* HE0 (Annex C punt 76) */
                  fprintf(out_lds, "%5.5s", HE0);

                  /* HE1 (Annex C punt 77) */
                  fprintf(out_lds, "%5.5s", HE1);

                  /* HE2 (Annex C punt 78) */
                  fprintf(out_lds, "%5.5s", HE2);

                  /* HE3 (Annex C punt 79) */
                  fprintf(out_lds, "%5.5s", HE3);

                  /* vrijheids graden HM0 (standaard op 0, staat niet in sovf80) (Annex C punt 80) */
                  fprintf(out_lds, "%4.4s", "0");

                  /* -1 (annex C punt 81) */
                  fprintf(out_lds, "%4.4s", "-1");

                  /* Tm-10 (Tm0-1) (annex C punt 82) */
                  fprintf(out_lds, "%4.4s", Tm_10_lds);

                  /* fp (piekfrequentie) (annex C punt 83) */
                  fprintf(out_lds, "%4.4s", Fp);

                  /* Hs7 (mm) (annex C punt 84) */
                  fprintf(out_lds, "%5.5s", Hs7_lds);

                  /* Tm02 (annex C punt 85) */
                  fprintf(out_lds, "%3.3s", Tm02);

                  /* -1 (annex C punt 86) */
                  fprintf(out_lds, "%3.3s", "-1");

                  /* aantal golven (AG) (annex C punt 87) */
                  fprintf(out_lds, "%3.3s", aantal_golven);

                  /* spectrum 10 Mhz (0.0 Hz - 0.14 Hz) (Annex C punt 88 - 102) */
                  for (m = 0; m < 15; m++)
                     fprintf(out_lds, "%9.9s", czz10_array[m]);

                  /* nieuwe regel */
                  fprintf(out_lds, "\n");

                  /* Th0 (gem. richting totale spectrum) (annex C punt 103) */
                  fprintf(out_lds, "%4.4s", Th0_lds);

                  /* S0bh (gem. spreiding totale spectrum) (annex C punt 104) */
                  fprintf(out_lds, "%4.4s", S0bh);

                  /* Th3 (gem. richting LFE) (annex C punt 105) */
                  fprintf(out_lds, "%4.4s", Th3);

                  /* AV10_R (aantal vrijheidsgraden sp. el. (annex C punt 106) */
                  fprintf(out_lds, "%3.3s", AV10_R);

                  /* DL_index (golfgetal) (annex C punt 107) */
                  fprintf(out_lds, "%5.5s", DL_index);

                  /* Th010 (gem. richting 10 mHz [0.0 Hz - 0.50 Hz]) (Annex C punt 108 - 158) */
                  for (m = 0; m < AANTAL_TH010; m++)
                     fprintf(out_lds, "%4.4s", th010_array[m]);

                  /* S0bh10 (gem. spreiding 10 mHz [0.0 Hz - 0.50 Hz]) (Annex C punt 159 - 209) */
                  for (m = 0; m < AANTAL_S0BH10; m++)
                     fprintf(out_lds, "%4.4s", S0bh10_array[m]);

                  /* nieuwe regel */
                  fprintf(out_lds, "\n");

               } /* if (aanvullen) */

            } /* if (strlen(line) >= 845) */
         } /* while (fgets(line, 1023, in) != NULL) */

         /* sluiten lfr input file */
         fclose(lfr_in);

      } /* if (in_lfr = fopen(dir_en_lfr_filenaam, "r") != NULL) */
      else /* file openen dus mislukt */
      {
         getcwd(volledig_path, 512);

         /* bericht samen stellen */
         strcpy(info, "\0");

         if (strcmp(OS, "WINDOWS") == 0)
         {
            strcat(info, volledig_path);      /* de environment dir var geeft al een volledig path onder unix */
            strcat(info, "\\");
         }

         strcat(info, dir_en_lfr_filenaam);
         strcat(info, " niet te lezen\n");

         /* naar log schrijven */
         Write_Log(info);

      } /* else (file openen dus mislukt) */


      /* sluiten van opstaande output files */
      fclose(out_cds);
      fclose(out_lds);
      fclose(out_dsp);
      fclose(out_lds2);
      fclose(out_dsp2);

   } /* else (outputfiles zijn alle drie te schrijven) */


   return 0;
}



/*****************************************************************************/
/*                                                                           */
/*                 Bepaal_CDS_en_LDS_en_DSP_Filenamen                        */
/*                                                                           */
/*****************************************************************************/
int Bepaal_CDS_en_LDS_en_DSP_Filenamen(char* cds_filenaam, char* lds_filenaam, char* dsp_filenaam, char* lds2_filenaam, char* dsp2_filenaam)
{
   /* CDS  filenaam voorbeeld: 2005051801.CDS */
   /* LDS  filenaam voorbeeld: WAVE_LDS_200505180100_00000_LC */
   /* DSP  filenaam voorbeeld: WAVE_DSP_200505180100_00000_LC */
   /* LDS2 filenaam voorbeeld: WAVE_LDS2_200505180100_00000_LC */
   /* DSP2 filenaam voorbeeld: WAVE_DSP2_200505180100_00000_LC */

   /* initialisatie */
   strcpy(cds_filenaam, "\0");
   strcpy(lds_filenaam, "\0");
   strcpy(dsp_filenaam, "\0");
   strcpy(lds2_filenaam, "\0");
   strcpy(dsp2_filenaam, "\0");


   /* NB datum in output files moet met eeuw aanduiding */

   /* CDS */
   strcpy(cds_filenaam, JJJJMMDDUU);
   strcat(cds_filenaam, ".CDS");

   /* LDS */
   strcpy(lds_filenaam, "WAVE_LDS_");
   strcat(lds_filenaam, JJJJMMDDUU);
   strcat(lds_filenaam, "00_00000_LC");

   /* DSP */
   strcpy(dsp_filenaam, "WAVE_DSP_");
   strcat(dsp_filenaam, JJJJMMDDUU);
   strcat(dsp_filenaam, "00_00000_LC");

   /* LDS2 */
   strcpy(lds2_filenaam, "WAVE_LDS2_");
   strcat(lds2_filenaam, JJJJMMDDUU);
   strcat(lds2_filenaam, "00_00000_LC");

   /* DSP2 */
   strcpy(dsp2_filenaam, "WAVE_DSP2_");
   strcat(dsp2_filenaam, JJJJMMDDUU);
   strcat(dsp2_filenaam, "00_00000_LC");


   return 0;
}




/*****************************************************************************/
/*                                                                           */
/*                        Bepaal_Dir_en_LFR_filenaam                         */
/*                                                                           */
/*****************************************************************************/
int Bepaal_Dir_en_LFR_filenaam(char* dir_en_lfr_filenaam)
{
   /* inputfilenaam b.v.: output_lfr/2005071112.LFR */


   /* initialisatie */
   strcpy(dir_en_lfr_filenaam, "\0");


   if (strcmp(OS, "WINDOWS") == 0)                     /* WINDOWS */
      strcpy(dir_en_lfr_filenaam, "output_lfr\\");
   else                                                /* UNIX */
      /*strcpy(dir_en_lfr_filenaam, "output_lfr/");*/
      strcpy(dir_en_lfr_filenaam, getenv("ENV_SPECTRA_LFR"));     /* ivm APL */
   /* n.b. onder windows met unix manier gaat ook wel goed, maar niet andersom ! */


   strcat(dir_en_lfr_filenaam, JJJJMMDDUU);
   strcat(dir_en_lfr_filenaam, ".LFR");


   return 0;
}


