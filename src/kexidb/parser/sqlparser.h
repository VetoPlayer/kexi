#include <field.h>
#include "parser.h"

void parseData(KexiDB::Parser *p, const char *data);
/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PERCENT = 258,
     SQL_TYPE = 259,
     SQL_ABS = 260,
     ACOS = 261,
     AMPERSAND = 262,
     SQL_ABSOLUTE = 263,
     ADA = 264,
     ADD = 265,
     ADD_DAYS = 266,
     ADD_HOURS = 267,
     ADD_MINUTES = 268,
     ADD_MONTHS = 269,
     ADD_SECONDS = 270,
     ADD_YEARS = 271,
     ALL = 272,
     ALLOCATE = 273,
     ALTER = 274,
     AND = 275,
     ANY = 276,
     ARE = 277,
     ARITHMETIC_MINUS = 278,
     ARITHMETIC_PLUS = 279,
     AS = 280,
     ASIN = 281,
     ASC = 282,
     ASCII = 283,
     ASSERTION = 284,
     ASTERISK = 285,
     AT = 286,
     ATAN = 287,
     ATAN2 = 288,
     AUTHORIZATION = 289,
     AUTO_INCREMENT = 290,
     AVG = 291,
     BEFORE = 292,
     SQL_BEGIN = 293,
     BETWEEN = 294,
     BIGINT = 295,
     BINARY = 296,
     BIT = 297,
     BIT_LENGTH = 298,
     BREAK = 299,
     BY = 300,
     CASCADE = 301,
     CASCADED = 302,
     CASE = 303,
     CAST = 304,
     CATALOG = 305,
     CEILING = 306,
     CENTER = 307,
     SQL_CHAR = 308,
     CHAR_LENGTH = 309,
     CHARACTER_STRING_LITERAL = 310,
     CHECK = 311,
     CLOSE = 312,
     COALESCE = 313,
     COBOL = 314,
     COLLATE = 315,
     COLLATION = 316,
     COLON = 317,
     COLUMN = 318,
     COMMA = 319,
     COMMIT = 320,
     COMPUTE = 321,
     CONCAT = 322,
     CONNECT = 323,
     CONNECTION = 324,
     CONSTRAINT = 325,
     CONSTRAINTS = 326,
     CONTINUE = 327,
     CONVERT = 328,
     CORRESPONDING = 329,
     COS = 330,
     COT = 331,
     COUNT = 332,
     CREATE = 333,
     CURDATE = 334,
     CURRENT = 335,
     CURRENT_DATE = 336,
     CURRENT_TIME = 337,
     CURRENT_TIMESTAMP = 338,
     CURTIME = 339,
     CURSOR = 340,
     DATABASE = 341,
     SQL_DATE = 342,
     DATE_FORMAT = 343,
     DATE_REMAINDER = 344,
     DATE_VALUE = 345,
     DAY = 346,
     DAYOFMONTH = 347,
     DAYOFWEEK = 348,
     DAYOFYEAR = 349,
     DAYS_BETWEEN = 350,
     DEALLOCATE = 351,
     DEC = 352,
     DECLARE = 353,
     DEFAULT = 354,
     DEFERRABLE = 355,
     DEFERRED = 356,
     SQL_DELETE = 357,
     DESC = 358,
     DESCRIBE = 359,
     DESCRIPTOR = 360,
     DIAGNOSTICS = 361,
     DICTIONARY = 362,
     DIRECTORY = 363,
     DISCONNECT = 364,
     DISPLACEMENT = 365,
     DISTINCT = 366,
     DOLLAR_SIGN = 367,
     DOMAIN_TOKEN = 368,
     DOT = 369,
     SQL_DOUBLE = 370,
     DOUBLE_QUOTED_STRING = 371,
     DROP = 372,
     ELSE = 373,
     END = 374,
     END_EXEC = 375,
     EQUAL = 376,
     ESCAPE = 377,
     EXCEPT = 378,
     SQL_EXCEPTION = 379,
     EXEC = 380,
     EXECUTE = 381,
     EXISTS = 382,
     EXP = 383,
     EXPONENT = 384,
     EXTERNAL = 385,
     EXTRACT = 386,
     SQL_FALSE = 387,
     FETCH = 388,
     FIRST = 389,
     SQL_FLOAT = 390,
     FLOOR = 391,
     FN = 392,
     FOR = 393,
     FOREIGN = 394,
     FORTRAN = 395,
     FOUND = 396,
     FOUR_DIGITS = 397,
     FROM = 398,
     FULL = 399,
     GET = 400,
     GLOBAL = 401,
     GO = 402,
     GOTO = 403,
     GRANT = 404,
     GREATER_THAN = 405,
     GREATER_OR_EQUAL = 406,
     GROUP = 407,
     HAVING = 408,
     HOUR = 409,
     HOURS_BETWEEN = 410,
     IDENTITY = 411,
     IFNULL = 412,
     SQL_IGNORE = 413,
     IMMEDIATE = 414,
     SQL_IN = 415,
     INCLUDE = 416,
     INDEX = 417,
     INDICATOR = 418,
     INITIALLY = 419,
     INNER = 420,
     INPUT = 421,
     INSENSITIVE = 422,
     INSERT = 423,
     INTEGER = 424,
     INTERSECT = 425,
     INTERVAL = 426,
     INTO = 427,
     IS = 428,
     ISOLATION = 429,
     JOIN = 430,
     JUSTIFY = 431,
     KEY = 432,
     LANGUAGE = 433,
     LAST = 434,
     LCASE = 435,
     LEFT = 436,
     LEFTPAREN = 437,
     LENGTH = 438,
     LESS_OR_EQUAL = 439,
     LESS_THAN = 440,
     LEVEL = 441,
     LIKE = 442,
     LINE_WIDTH = 443,
     LOCAL = 444,
     LOCATE = 445,
     LOG = 446,
     SQL_LONG = 447,
     LOWER = 448,
     LTRIM = 449,
     LTRIP = 450,
     MATCH = 451,
     SQL_MAX = 452,
     MICROSOFT = 453,
     SQL_MIN = 454,
     MINUS = 455,
     MINUTE = 456,
     MINUTES_BETWEEN = 457,
     MOD = 458,
     MODIFY = 459,
     MODULE = 460,
     MONTH = 461,
     MONTHS_BETWEEN = 462,
     MUMPS = 463,
     NAMES = 464,
     NATIONAL = 465,
     NCHAR = 466,
     NEXT = 467,
     NODUP = 468,
     NONE = 469,
     NOT = 470,
     NOT_EQUAL = 471,
     NOW = 472,
     SQL_NULL = 473,
     NULLIF = 474,
     NUMERIC = 475,
     OCTET_LENGTH = 476,
     ODBC = 477,
     OF = 478,
     SQL_OFF = 479,
     SQL_ON = 480,
     ONLY = 481,
     OPEN = 482,
     OPTION = 483,
     OR = 484,
     ORDER = 485,
     OUTER = 486,
     OUTPUT = 487,
     OVERLAPS = 488,
     PAGE = 489,
     PARTIAL = 490,
     SQL_PASCAL = 491,
     PERSISTENT = 492,
     CQL_PI = 493,
     PLI = 494,
     POSITION = 495,
     PRECISION = 496,
     PREPARE = 497,
     PRESERVE = 498,
     PRIMARY = 499,
     PRIOR = 500,
     PRIVILEGES = 501,
     PROCEDURE = 502,
     PRODUCT = 503,
     PUBLIC = 504,
     QUARTER = 505,
     QUESTION_MARK = 506,
     QUIT = 507,
     RAND = 508,
     READ_ONLY = 509,
     REAL = 510,
     REFERENCES = 511,
     REPEAT = 512,
     REPLACE = 513,
     RESTRICT = 514,
     REVOKE = 515,
     RIGHT = 516,
     RIGHTPAREN = 517,
     ROLLBACK = 518,
     ROWS = 519,
     RPAD = 520,
     RTRIM = 521,
     SCHEMA = 522,
     SCREEN_WIDTH = 523,
     SCROLL = 524,
     SECOND = 525,
     SECONDS_BETWEEN = 526,
     SELECT = 527,
     SEMICOLON = 528,
     SEQUENCE = 529,
     SETOPT = 530,
     SET = 531,
     SHOWOPT = 532,
     SIGN = 533,
     SIGNED_INTEGER = 534,
     SIN = 535,
     SINGLE_QUOTE = 536,
     SQL_SIZE = 537,
     SLASH = 538,
     SMALLINT = 539,
     SOME = 540,
     SPACE = 541,
     SQL = 542,
     SQL_TRUE = 543,
     SQLCA = 544,
     SQLCODE = 545,
     SQLERROR = 546,
     SQLSTATE = 547,
     SQLWARNING = 548,
     SQRT = 549,
     STDEV = 550,
     SUBSTRING = 551,
     SUM = 552,
     SYSDATE = 553,
     SYSDATE_FORMAT = 554,
     SYSTEM = 555,
     TABLE = 556,
     TAN = 557,
     TEMPORARY = 558,
     THEN = 559,
     THREE_DIGITS = 560,
     TIME = 561,
     TIMESTAMP = 562,
     TIMEZONE_HOUR = 563,
     TIMEZONE_MINUTE = 564,
     TINYINT = 565,
     TO = 566,
     TO_CHAR = 567,
     TO_DATE = 568,
     TRANSACTION = 569,
     TRANSLATE = 570,
     TRANSLATION = 571,
     TRUNCATE = 572,
     GENERAL_TITLE = 573,
     TWO_DIGITS = 574,
     UCASE = 575,
     UNION = 576,
     UNIQUE = 577,
     SQL_UNKNOWN = 578,
     UNSIGNED_INTEGER = 579,
     UPDATE = 580,
     UPPER = 581,
     USAGE = 582,
     USER = 583,
     USER_DEFINED_NAME = 584,
     USING = 585,
     VALUE = 586,
     VALUES = 587,
     VARBINARY = 588,
     VARCHAR = 589,
     VARYING = 590,
     VENDOR = 591,
     VIEW = 592,
     WEEK = 593,
     WHEN = 594,
     WHENEVER = 595,
     WHERE = 596,
     WHERE_CURRENT_OF = 597,
     WITH = 598,
     WORD_WRAPPED = 599,
     WORK = 600,
     WRAPPED = 601,
     YEAR = 602,
     YEARS_BETWEEN = 603
   };
#endif
#define PERCENT 258
#define SQL_TYPE 259
#define SQL_ABS 260
#define ACOS 261
#define AMPERSAND 262
#define SQL_ABSOLUTE 263
#define ADA 264
#define ADD 265
#define ADD_DAYS 266
#define ADD_HOURS 267
#define ADD_MINUTES 268
#define ADD_MONTHS 269
#define ADD_SECONDS 270
#define ADD_YEARS 271
#define ALL 272
#define ALLOCATE 273
#define ALTER 274
#define AND 275
#define ANY 276
#define ARE 277
#define ARITHMETIC_MINUS 278
#define ARITHMETIC_PLUS 279
#define AS 280
#define ASIN 281
#define ASC 282
#define ASCII 283
#define ASSERTION 284
#define ASTERISK 285
#define AT 286
#define ATAN 287
#define ATAN2 288
#define AUTHORIZATION 289
#define AUTO_INCREMENT 290
#define AVG 291
#define BEFORE 292
#define SQL_BEGIN 293
#define BETWEEN 294
#define BIGINT 295
#define BINARY 296
#define BIT 297
#define BIT_LENGTH 298
#define BREAK 299
#define BY 300
#define CASCADE 301
#define CASCADED 302
#define CASE 303
#define CAST 304
#define CATALOG 305
#define CEILING 306
#define CENTER 307
#define SQL_CHAR 308
#define CHAR_LENGTH 309
#define CHARACTER_STRING_LITERAL 310
#define CHECK 311
#define CLOSE 312
#define COALESCE 313
#define COBOL 314
#define COLLATE 315
#define COLLATION 316
#define COLON 317
#define COLUMN 318
#define COMMA 319
#define COMMIT 320
#define COMPUTE 321
#define CONCAT 322
#define CONNECT 323
#define CONNECTION 324
#define CONSTRAINT 325
#define CONSTRAINTS 326
#define CONTINUE 327
#define CONVERT 328
#define CORRESPONDING 329
#define COS 330
#define COT 331
#define COUNT 332
#define CREATE 333
#define CURDATE 334
#define CURRENT 335
#define CURRENT_DATE 336
#define CURRENT_TIME 337
#define CURRENT_TIMESTAMP 338
#define CURTIME 339
#define CURSOR 340
#define DATABASE 341
#define SQL_DATE 342
#define DATE_FORMAT 343
#define DATE_REMAINDER 344
#define DATE_VALUE 345
#define DAY 346
#define DAYOFMONTH 347
#define DAYOFWEEK 348
#define DAYOFYEAR 349
#define DAYS_BETWEEN 350
#define DEALLOCATE 351
#define DEC 352
#define DECLARE 353
#define DEFAULT 354
#define DEFERRABLE 355
#define DEFERRED 356
#define SQL_DELETE 357
#define DESC 358
#define DESCRIBE 359
#define DESCRIPTOR 360
#define DIAGNOSTICS 361
#define DICTIONARY 362
#define DIRECTORY 363
#define DISCONNECT 364
#define DISPLACEMENT 365
#define DISTINCT 366
#define DOLLAR_SIGN 367
#define DOMAIN_TOKEN 368
#define DOT 369
#define SQL_DOUBLE 370
#define DOUBLE_QUOTED_STRING 371
#define DROP 372
#define ELSE 373
#define END 374
#define END_EXEC 375
#define EQUAL 376
#define ESCAPE 377
#define EXCEPT 378
#define SQL_EXCEPTION 379
#define EXEC 380
#define EXECUTE 381
#define EXISTS 382
#define EXP 383
#define EXPONENT 384
#define EXTERNAL 385
#define EXTRACT 386
#define SQL_FALSE 387
#define FETCH 388
#define FIRST 389
#define SQL_FLOAT 390
#define FLOOR 391
#define FN 392
#define FOR 393
#define FOREIGN 394
#define FORTRAN 395
#define FOUND 396
#define FOUR_DIGITS 397
#define FROM 398
#define FULL 399
#define GET 400
#define GLOBAL 401
#define GO 402
#define GOTO 403
#define GRANT 404
#define GREATER_THAN 405
#define GREATER_OR_EQUAL 406
#define GROUP 407
#define HAVING 408
#define HOUR 409
#define HOURS_BETWEEN 410
#define IDENTITY 411
#define IFNULL 412
#define SQL_IGNORE 413
#define IMMEDIATE 414
#define SQL_IN 415
#define INCLUDE 416
#define INDEX 417
#define INDICATOR 418
#define INITIALLY 419
#define INNER 420
#define INPUT 421
#define INSENSITIVE 422
#define INSERT 423
#define INTEGER 424
#define INTERSECT 425
#define INTERVAL 426
#define INTO 427
#define IS 428
#define ISOLATION 429
#define JOIN 430
#define JUSTIFY 431
#define KEY 432
#define LANGUAGE 433
#define LAST 434
#define LCASE 435
#define LEFT 436
#define LEFTPAREN 437
#define LENGTH 438
#define LESS_OR_EQUAL 439
#define LESS_THAN 440
#define LEVEL 441
#define LIKE 442
#define LINE_WIDTH 443
#define LOCAL 444
#define LOCATE 445
#define LOG 446
#define SQL_LONG 447
#define LOWER 448
#define LTRIM 449
#define LTRIP 450
#define MATCH 451
#define SQL_MAX 452
#define MICROSOFT 453
#define SQL_MIN 454
#define MINUS 455
#define MINUTE 456
#define MINUTES_BETWEEN 457
#define MOD 458
#define MODIFY 459
#define MODULE 460
#define MONTH 461
#define MONTHS_BETWEEN 462
#define MUMPS 463
#define NAMES 464
#define NATIONAL 465
#define NCHAR 466
#define NEXT 467
#define NODUP 468
#define NONE 469
#define NOT 470
#define NOT_EQUAL 471
#define NOW 472
#define SQL_NULL 473
#define NULLIF 474
#define NUMERIC 475
#define OCTET_LENGTH 476
#define ODBC 477
#define OF 478
#define SQL_OFF 479
#define SQL_ON 480
#define ONLY 481
#define OPEN 482
#define OPTION 483
#define OR 484
#define ORDER 485
#define OUTER 486
#define OUTPUT 487
#define OVERLAPS 488
#define PAGE 489
#define PARTIAL 490
#define SQL_PASCAL 491
#define PERSISTENT 492
#define CQL_PI 493
#define PLI 494
#define POSITION 495
#define PRECISION 496
#define PREPARE 497
#define PRESERVE 498
#define PRIMARY 499
#define PRIOR 500
#define PRIVILEGES 501
#define PROCEDURE 502
#define PRODUCT 503
#define PUBLIC 504
#define QUARTER 505
#define QUESTION_MARK 506
#define QUIT 507
#define RAND 508
#define READ_ONLY 509
#define REAL 510
#define REFERENCES 511
#define REPEAT 512
#define REPLACE 513
#define RESTRICT 514
#define REVOKE 515
#define RIGHT 516
#define RIGHTPAREN 517
#define ROLLBACK 518
#define ROWS 519
#define RPAD 520
#define RTRIM 521
#define SCHEMA 522
#define SCREEN_WIDTH 523
#define SCROLL 524
#define SECOND 525
#define SECONDS_BETWEEN 526
#define SELECT 527
#define SEMICOLON 528
#define SEQUENCE 529
#define SETOPT 530
#define SET 531
#define SHOWOPT 532
#define SIGN 533
#define SIGNED_INTEGER 534
#define SIN 535
#define SINGLE_QUOTE 536
#define SQL_SIZE 537
#define SLASH 538
#define SMALLINT 539
#define SOME 540
#define SPACE 541
#define SQL 542
#define SQL_TRUE 543
#define SQLCA 544
#define SQLCODE 545
#define SQLERROR 546
#define SQLSTATE 547
#define SQLWARNING 548
#define SQRT 549
#define STDEV 550
#define SUBSTRING 551
#define SUM 552
#define SYSDATE 553
#define SYSDATE_FORMAT 554
#define SYSTEM 555
#define TABLE 556
#define TAN 557
#define TEMPORARY 558
#define THEN 559
#define THREE_DIGITS 560
#define TIME 561
#define TIMESTAMP 562
#define TIMEZONE_HOUR 563
#define TIMEZONE_MINUTE 564
#define TINYINT 565
#define TO 566
#define TO_CHAR 567
#define TO_DATE 568
#define TRANSACTION 569
#define TRANSLATE 570
#define TRANSLATION 571
#define TRUNCATE 572
#define GENERAL_TITLE 573
#define TWO_DIGITS 574
#define UCASE 575
#define UNION 576
#define UNIQUE 577
#define SQL_UNKNOWN 578
#define UNSIGNED_INTEGER 579
#define UPDATE 580
#define UPPER 581
#define USAGE 582
#define USER 583
#define USER_DEFINED_NAME 584
#define USING 585
#define VALUE 586
#define VALUES 587
#define VARBINARY 588
#define VARCHAR 589
#define VARYING 590
#define VENDOR 591
#define VIEW 592
#define WEEK 593
#define WHEN 594
#define WHENEVER 595
#define WHERE 596
#define WHERE_CURRENT_OF 597
#define WITH 598
#define WORD_WRAPPED 599
#define WORK 600
#define WRAPPED 601
#define YEAR 602
#define YEARS_BETWEEN 603




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 497 "sqlparser.y"
typedef union YYSTYPE {
	char stringValue[255];
	int integerValue;
	KexiDB::Field::Type coltype;
	KexiDB::Field *field;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 740 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



