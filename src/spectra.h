/* sentry */
#define spectra_h

/* hier moet staan WINDOWS of UNIX */
#define UNIX


/* betreffende input files arrays */
#define AANTAL_INPUT_FILES                                    50         /* (max) aantal input files dat klaar kan staan voor openen */
#define LENGTE_INPUT_FILENAAM                                 38         /* lengte input file naam in characters (bv MSS_R098123256_644_SOVF80_EHDB_081200) */

/* geografische posities (graden en tienden graden) */
#define NC1_LAT                                               "    6120"
#define NC1_LON                                               "     110"
#define NC2_LAT                                               "    6120"
#define NC2_LON                                               "     110"
#define ANA1_LAT                                              "    5726"
#define ANA1_LON                                              "      78"
#define AUK1_LAT                                              "    5640"
#define AUK1_LON                                              "     200"
#define AUK2_LAT                                              "    5640"
#define AUK2_LON                                              "     200"
#define K131_LAT                                              "    5320"
#define K131_LON                                              "     320"
#define K132_LAT                                              "    5320"
#define K132_LON                                              "     320"
#define K133_LAT                                              "    5320"
#define K133_LON                                              "     320"
#define ELD1_LAT                                              "    5327"
#define ELD1_LON                                              "     465"
#define SMN1_LAT                                              "    5359"
#define SMN1_LON                                              "     617"
#define IJ51_LAT                                              "    5249"
#define IJ51_LON                                              "     427"
#define MUN1_LAT                                              "    5257"
#define MUN1_LON                                              "     406"
#define MUN2_LAT                                              "    5257"
#define MUN2_LON                                              "     406"
#define MPN1_LAT                                              "    5220"
#define MPN1_LON                                              "     420"
#define MPN2_LAT                                              "    5220"
#define MPN2_LON                                              "     420"
#define EPL2_LAT                                              "    5190"
#define EPL2_LON                                              "     320"
#define EPL3_LAT                                              "    5190"
#define EPL3_LON                                              "     320"
#define DWE1_LAT                                              "    5195"
#define DWE1_LON                                              "     300"
#define LEG1_LAT                                              "    5190"
#define LEG1_LON                                              "     360"
#define LEG2_LAT                                              "    5190"
#define LEG2_LON                                              "     360"
#define E131_LAT                                              "    5200"
#define E131_LON                                              "     373"
#define Q11_LAT                                               "    5293"
#define Q11_LON                                               "     415"
#define Q12_LAT                                               "    5293"
#define Q12_LON                                               "     415"
#define A121_LAT                                              "    5542"
#define A121_LON                                              "     382"
#define A122_LAT                                              "    5542"
#define A122_LON                                              "     382"
#define BG2b_LAT                                              "    5177"
#define BG2b_LON                                              "     362"

/* quadranten */
#define NC1_QUADRANT                                          "1"
#define NC2_QUADRANT                                          "1"
#define ANA1_QUADRANT                                         "1"
#define AUK1_QUADRANT                                         "1"
#define AUK2_QUADRANT                                         "1"
#define K131_QUADRANT                                         "1"
#define K132_QUADRANT                                         "1"
#define K133_QUADRANT                                         "1"
#define ELD1_QUADRANT                                         "1"
#define SMN1_QUADRANT                                         "1"
#define IJ51_QUADRANT                                         "1"
#define MUN1_QUADRANT                                         "1"
#define MUN2_QUADRANT                                         "1"
#define MPN1_QUADRANT                                         "1"
#define MPN2_QUADRANT                                         "1"
#define EPL2_QUADRANT                                         "1"
#define EPL3_QUADRANT                                         "1"
#define DWE1_QUADRANT                                         "1"
#define LEG1_QUADRANT                                         "1"
#define LEG2_QUADRANT                                         "1"
#define E131_QUADRANT                                         "1"
#define Q11_QUADRANT                                          "1"
#define Q12_QUADRANT                                          "1"
#define A121_QUADRANT                                         "1"
#define A122_QUADRANT                                         "1"
#define BG2b_QUADRANT                                         "1"

/* reductie factoren wind snelheid (uit sovf) voor hoogte */
#define NC1_FF_FACTOR                                         1.27
#define NC2_FF_FACTOR                                         1.27
#define ANA1_FF_FACTOR                                        1.00
#define AUK1_FF_FACTOR                                        1.27
#define AUK2_FF_FACTOR                                        1.27
#define K131_FF_FACTOR                                        1.23
#define K132_FF_FACTOR                                        1.23
#define K133_FF_FACTOR                                        1.23
#define ELD1_FF_FACTOR                                        1.00
#define SMN1_FF_FACTOR                                        1.00
#define IJ51_FF_FACTOR                                        1.07
#define MUN1_FF_FACTOR                                        1.07
#define MUN2_FF_FACTOR                                        1.07
#define MPN1_FF_FACTOR                                        1.12
#define MPN2_FF_FACTOR                                        1.12
#define EPL2_FF_FACTOR                                        1.12
#define EPL3_FF_FACTOR                                        1.12
#define DWE1_FF_FACTOR                                        1.12
#define LEG1_FF_FACTOR                                        1.15
#define LEG2_FF_FACTOR                                        1.15
#define E131_FF_FACTOR                                        1.15
#define Q11_FF_FACTOR                                         1.00          /* 15-03-2010: gokje, nog verder uitzoekn */
#define Q12_FF_FACTOR                                         1.00          /* 15-03-2010: gokje, nog verder uitzoekn */
#define A121_FF_FACTOR                                        1.00          /* 15-03-2010: gokje, nog verder uitzoekn */
#define A122_FF_FACTOR                                        1.00          /* 15-03-2010: gokje, nog verder uitzoekn */
#define BG2b_FF_FACTOR                                        1.00          /* 15-03-2010: gokje, nog verder uitzoekn */

/* waterdiepten (meters) */
#define NC1_DIEPTE                                            161.0
#define NC2_DIEPTE                                            161.0
#define ANA1_DIEPTE                                           86.0
#define AUK1_DIEPTE                                           77.0
#define AUK2_DIEPTE                                           77.0
#define K131_DIEPTE                                           28.0
#define K132_DIEPTE                                           28.0
#define K133_DIEPTE                                           28.0
#define ELD1_DIEPTE                                           28.0
#define SMN1_DIEPTE                                           18.0
#define IJ51_DIEPTE                                           20.0
#define MUN1_DIEPTE                                           24.0
#define MUN2_DIEPTE                                           24.0
#define MPN1_DIEPTE                                           16.0
#define MPN2_DIEPTE                                           16.0
#define EPL2_DIEPTE                                           30.0
#define EPL3_DIEPTE                                           30.0
#define DWE1_DIEPTE                                           30.0
#define LEG1_DIEPTE                                           20.0
#define LEG2_DIEPTE                                           20.0
#define E131_DIEPTE                                           22.0
#define Q11_DIEPTE                                            26.0          /* 15-03-2010: opgezocht met Frits Koek in zeekaart */
#define Q12_DIEPTE                                            26.0          /* 15-03-2010: opgezocht met Frits Koek in zeekaart */
#define A121_DIEPTE                                           28.0          /* 15-03-2010: opgezocht met Frits Koek in zeekaart */
#define A122_DIEPTE                                           28.0          /* 15-03-2010: opgezocht met Frits Koek in zeekaart */
#define BG2b_DIEPTE                                           10.0          /* 15-03-2010: opgezocht met Frits Koek in zeekaart */

/* ontbrekende waarden n.a.v. sovf80 */
#define CIC_ONTBREKEND                                        " 7654321"    /* erfenis Evert, aanduiding in CIC output file voor ontbrekende waarde */
#define LFR_ONTBREKEND                                        "0"           /* erfenis Evert, aanduiding in LFR output file voor algemeen (behalve dd, ff) ontbrekende waarde */
#define DD_LFR_ONTBREKEND                                     "99"          /* erfenis Evert, aanduiding in LFR output file voor ontbrekende dd waarde*/
#define FF_LFR_ONTBREKEND                                     "99"          /* erfenis Evert, aanduiding in LFR output file voor ontbrekende ff waarde*/

/* ontbrekende waarden n.a.v. sovf81 */
#define CID_ONTBREKEND                                        " 7654321"    /* erfenis Evert, aanduiding in CID output file voor ontbrekende waarde */
#define LFD_ONTBREKEND                                        "0"           /* erfenis Evert, aanduiding in LFD output file voor algemeen (behalve dd, ff) ontbrekende waarde */
#define DD_LFD_ONTBREKEND                                     "99"          /* erfenis Evert, aanduiding in LFD output file voor ontbrekende dd waarde*/
#define FF_LFD_ONTBREKEND                                     "99"          /* erfenis Evert, aanduiding in LFD output file voor ontbrekende ff waarde*/

/* ontbrekende waarden n.a.v. sovf82 */
#define CDS_ONTBREKEND                                        " 7654321"    /* erfenis Evert, aanduiding in CDS output file voor ontbrekende waarde */
#define LDS_ONTBREKEND                                        "0"           /* erfenis Evert, aanduiding in LDS output file voor algemeen (behalve dd, ff) ontbrekende waarde */
#define LDS_TH010_ONTBREKEND                                  "9999"        /* erfenis Evert, aanduiding in LDS output file voor Th010 ontbrekende waarde */
#define LDS_S0BH10_ONTBREKEND                                 "9999"        /* erfenis Evert, aanduiding in LDS output file voor S0bh10 ontbrekende waarde */
#define DD_LDS_ONTBREKEND                                     "99"          /* erfenis Evert, aanduiding in LDS output file voor ontbrekende dd waarde*/
#define FF_LDS_ONTBREKEND                                     "99"          /* erfenis Evert, aanduiding in LDS output file voor ontbrekende ff waarde*/
#define AANTAL_SENSORCODES                                    25            /* max aantal sensorcode namen waarvan de data echt in de lds file is terechtgekomen (ook in preprocessing hiervoor lfr gebruikt) */

/* flags */
#define SUSPECT_FLAG                                          "SUSPECT"
#define OK_FLAG                                               "OK     "

/* i.v.m. sovf80 en sovf82 */
#define AANTAL_CZZ5                                           25
#define AANTAL_CZZ10                                          51

/* i.v.m. sovf82 */
#define AANTAL_TH010                                          51
#define AANTAL_S0BH10                                         51
#define TWOPI                                                 6.283185      /* erfenis Evert voor DSP files */
#define DEGRAD                                                57.2958       /* erfenis Evert voor DSP files */
#define NDVTWP                                                5.72958       /* erfenis Evert voor DSP files */

/* i.v.m. sovf81 */
#define AANTAL_SOVF81                                         129           /* i.t.t sovf80 en sovf82 heeft sovf81 altijd 129 records */
#define PI                                                    3.14159265







